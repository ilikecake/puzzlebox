/*
 * @brief Host mode driver for the library USB Still Image Class driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * Copyright(C) Dean Camera, 2011, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */


#define  __INCLUDE_FROM_USB_DRIVER
#include "../../Core/USBMode.h"

#if defined(USB_CAN_BE_HOST)

#define  __INCLUDE_FROM_SI_DRIVER
#define  __INCLUDE_FROM_STILLIMAGE_HOST_C
#include "StillImageClassHost.h"

uint8_t SI_Host_ConfigurePipes(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                               uint16_t ConfigDescriptorSize,
                               void* ConfigDescriptorData)
{
	USB_Descriptor_Endpoint_t*  DataINEndpoint      = NULL;
	USB_Descriptor_Endpoint_t*  DataOUTEndpoint     = NULL;
	USB_Descriptor_Endpoint_t*  EventsEndpoint      = NULL;
	USB_Descriptor_Interface_t* StillImageInterface = NULL;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	memset(&SIInterfaceInfo->State, 0x00, sizeof(SIInterfaceInfo->State));

	if (DESCRIPTOR_TYPE(ConfigDescriptorData) != DTYPE_Configuration)
	  return SI_ENUMERROR_InvalidConfigDescriptor;

	while (!(DataINEndpoint) || !(DataOUTEndpoint) || !(EventsEndpoint))
	{
		if (!(StillImageInterface) ||
		    USB_GetNextDescriptorComp(&ConfigDescriptorSize, &ConfigDescriptorData,
		                              DCOMP_SI_Host_NextSIInterfaceEndpoint) != DESCRIPTOR_SEARCH_COMP_Found)
		{
			if (USB_GetNextDescriptorComp(&ConfigDescriptorSize, &ConfigDescriptorData,
			                              DCOMP_SI_Host_NextSIInterface) != DESCRIPTOR_SEARCH_COMP_Found)
			{
				return SI_ENUMERROR_NoCompatibleInterfaceFound;
			}

			StillImageInterface = DESCRIPTOR_PCAST(ConfigDescriptorData, USB_Descriptor_Interface_t);

			DataINEndpoint  = NULL;
			DataOUTEndpoint = NULL;
			EventsEndpoint  = NULL;

			continue;
		}

		USB_Descriptor_Endpoint_t* EndpointData = DESCRIPTOR_PCAST(ConfigDescriptorData, USB_Descriptor_Endpoint_t);

		if ((EndpointData->EndpointAddress & ENDPOINT_DIR_MASK) == ENDPOINT_DIR_IN)
		{
			if ((EndpointData->Attributes & EP_TYPE_MASK) == EP_TYPE_INTERRUPT)
			  EventsEndpoint = EndpointData;
			else
			  DataINEndpoint = EndpointData;
		}
		else
		{
			DataOUTEndpoint = EndpointData;
		}
	}

	for (uint8_t PipeNum = 1; PipeNum < PIPE_TOTAL_PIPES; PipeNum++)
	{
		uint16_t Size;
		uint8_t  Type;
		uint8_t  Token;
		uint8_t  EndpointAddress;
		uint8_t  InterruptPeriod;
		bool     DoubleBanked;

		if (PipeNum == SIInterfaceInfo->Config.DataINPipeNumber)
		{
			Size            = DataINEndpoint->EndpointSize;
			EndpointAddress = DataINEndpoint->EndpointAddress;
			Token           = PIPE_TOKEN_IN;
			Type            = EP_TYPE_BULK;
			DoubleBanked    = SIInterfaceInfo->Config.DataINPipeDoubleBank;
			InterruptPeriod = 0;

			SIInterfaceInfo->State.DataINPipeSize = DataINEndpoint->EndpointSize;
		}
		else if (PipeNum == SIInterfaceInfo->Config.DataOUTPipeNumber)
		{
			Size            = DataOUTEndpoint->EndpointSize;
			EndpointAddress = DataOUTEndpoint->EndpointAddress;
			Token           = PIPE_TOKEN_OUT;
			Type            = EP_TYPE_BULK;
			DoubleBanked    = SIInterfaceInfo->Config.DataOUTPipeDoubleBank;
			InterruptPeriod = 0;

			SIInterfaceInfo->State.DataOUTPipeSize = DataOUTEndpoint->EndpointSize;
		}
		else if (PipeNum == SIInterfaceInfo->Config.EventsPipeNumber)
		{
			Size            = EventsEndpoint->EndpointSize;
			EndpointAddress = EventsEndpoint->EndpointAddress;
			Token           = PIPE_TOKEN_IN;
			Type            = EP_TYPE_INTERRUPT;
			DoubleBanked    = SIInterfaceInfo->Config.EventsPipeDoubleBank;
			InterruptPeriod = EventsEndpoint->PollingIntervalMS;

			SIInterfaceInfo->State.EventsPipeSize = EventsEndpoint->EndpointSize;
		}
		else
		{
			continue;
		}
		
		if (!(Pipe_ConfigurePipe(portnum,PipeNum, Type, Token, EndpointAddress, Size,
		                         DoubleBanked ? PIPE_BANK_DOUBLE : PIPE_BANK_SINGLE)))
		{
			return SI_ENUMERROR_PipeConfigurationFailed;
		}
		
		if (InterruptPeriod)
		  Pipe_SetInterruptPeriod(InterruptPeriod);
	}

	SIInterfaceInfo->State.InterfaceNumber = StillImageInterface->InterfaceNumber;
	SIInterfaceInfo->State.IsActive = true;

	return SI_ENUMERROR_NoError;
}

uint8_t DCOMP_SI_Host_NextSIInterface(void* const CurrentDescriptor)
{
	USB_Descriptor_Header_t* Header = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Header_t);

	if (Header->Type == DTYPE_Interface)
	{
		USB_Descriptor_Interface_t* Interface = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Interface_t);

		if ((Interface->Class    == SI_CSCP_StillImageClass)    &&
		    (Interface->SubClass == SI_CSCP_StillImageSubclass) &&
		    (Interface->Protocol == SI_CSCP_BulkOnlyProtocol))
		{
			return DESCRIPTOR_SEARCH_Found;
		}
	}

	return DESCRIPTOR_SEARCH_NotFound;
}

uint8_t DCOMP_SI_Host_NextSIInterfaceEndpoint(void* const CurrentDescriptor)
{
	USB_Descriptor_Header_t* Header = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Header_t);

	if (Header->Type == DTYPE_Endpoint)
	{
		USB_Descriptor_Endpoint_t* Endpoint = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Endpoint_t);

		uint8_t EndpointType = (Endpoint->Attributes & EP_TYPE_MASK);

		if (((EndpointType == EP_TYPE_BULK) || (EndpointType == EP_TYPE_INTERRUPT)) &&
		    (!(Pipe_IsEndpointBound(Endpoint->EndpointAddress))))
		{
			return DESCRIPTOR_SEARCH_Found;
		}
	}
	else if (Header->Type == DTYPE_Interface)
	{
		return DESCRIPTOR_SEARCH_Fail;
	}

	return DESCRIPTOR_SEARCH_NotFound;
}

uint8_t SI_Host_SendBlockHeader(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                                PIMA_Container_t* const PIMAHeader)
{
	uint8_t ErrorCode;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	if (SIInterfaceInfo->State.IsSessionOpen)
	  PIMAHeader->TransactionID = cpu_to_le32(SIInterfaceInfo->State.TransactionID++);

	Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.DataOUTPipeNumber);
	Pipe_Unfreeze();

	if ((ErrorCode = Pipe_Write_Stream_LE(portnum,PIMAHeader, PIMA_COMMAND_SIZE(0), NULL)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	uint8_t ParamBytes = (PIMAHeader->DataLength - PIMA_COMMAND_SIZE(0));

	if (ParamBytes)
	{
		if ((ErrorCode = Pipe_Write_Stream_LE(portnum,&PIMAHeader->Params, ParamBytes, NULL)) != PIPE_RWSTREAM_NoError)
		  return ErrorCode;
	}

	Pipe_ClearOUT(portnum);
	Pipe_Freeze();

	return PIPE_RWSTREAM_NoError;
}

uint8_t SI_Host_ReceiveBlockHeader(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                                   PIMA_Container_t* const PIMAHeader)
{
	uint16_t TimeoutMSRem        = SI_COMMAND_DATA_TIMEOUT_MS;
	uint16_t PreviousFrameNumber = USB_Host_GetFrameNumber();
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.DataINPipeNumber);
	Pipe_Unfreeze();

	while (!(Pipe_IsINReceived(portnum)))
	{
		uint16_t CurrentFrameNumber = USB_Host_GetFrameNumber();

		if (CurrentFrameNumber != PreviousFrameNumber)
		{
			PreviousFrameNumber = CurrentFrameNumber;

			if (!(TimeoutMSRem--))
			  return PIPE_RWSTREAM_Timeout;
		}

		Pipe_Freeze();
		Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.DataOUTPipeNumber);
		Pipe_Unfreeze();

		if (Pipe_IsStalled(portnum))
		{
			USB_Host_ClearEndpointStall(portnum,Pipe_GetBoundEndpointAddress(portnum));
			return PIPE_RWSTREAM_PipeStalled;
		}

		Pipe_Freeze();
		Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.DataINPipeNumber);
		Pipe_Unfreeze();

		if (Pipe_IsStalled(portnum))
		{
			USB_Host_ClearEndpointStall(portnum,Pipe_GetBoundEndpointAddress(portnum));
			return PIPE_RWSTREAM_PipeStalled;
		}

		if (USB_HostState[portnum] == HOST_STATE_Unattached)
		  return PIPE_RWSTREAM_DeviceDisconnected;
	}

	Pipe_Read_Stream_LE(portnum,PIMAHeader, PIMA_COMMAND_SIZE(0), NULL);

	if (PIMAHeader->Type == CPU_TO_LE16(PIMA_CONTAINER_ResponseBlock))
	{
		uint8_t ParamBytes = (PIMAHeader->DataLength - PIMA_COMMAND_SIZE(0));

		if (ParamBytes)
		  Pipe_Read_Stream_LE(portnum,&PIMAHeader->Params, ParamBytes, NULL);

		Pipe_ClearIN(portnum);
	}

	Pipe_Freeze();

	return PIPE_RWSTREAM_NoError;
}

uint8_t SI_Host_SendData(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                         void* Buffer,
                         const uint16_t Bytes)
{
	uint8_t ErrorCode;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.DataOUTPipeNumber);
	Pipe_Unfreeze();

	ErrorCode = Pipe_Write_Stream_LE(portnum,Buffer, Bytes, NULL);

	Pipe_ClearOUT(portnum);
	Pipe_Freeze();

	return ErrorCode;
}

uint8_t SI_Host_ReadData(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                         void* Buffer,
                         const uint16_t Bytes)
{
	uint8_t ErrorCode;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.DataINPipeNumber);
	Pipe_Unfreeze();

	ErrorCode = Pipe_Read_Stream_LE(portnum,Buffer, Bytes, NULL);

	Pipe_Freeze();

	return ErrorCode;
}

bool SI_Host_IsEventReceived(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo)
{
	bool IsEventReceived = false;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return false;

	Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.EventsPipeNumber);
	Pipe_Unfreeze();

	if (Pipe_BytesInPipe(portnum))
	  IsEventReceived = true;

	Pipe_Freeze();

	return IsEventReceived;
}

uint8_t SI_Host_ReceiveEventHeader(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                                   PIMA_Container_t* const PIMAHeader)
{
	uint8_t ErrorCode;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	Pipe_SelectPipe(portnum,SIInterfaceInfo->Config.EventsPipeNumber);
	Pipe_Unfreeze();

	ErrorCode = Pipe_Read_Stream_LE(portnum,PIMAHeader, sizeof(PIMA_Container_t), NULL);

	Pipe_ClearIN(portnum);
	Pipe_Freeze();

	return ErrorCode;
}

uint8_t SI_Host_OpenSession(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo)
{
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	uint8_t ErrorCode;

	SIInterfaceInfo->State.TransactionID = 0;
	SIInterfaceInfo->State.IsSessionOpen = false;

	PIMA_Container_t PIMABlock = (PIMA_Container_t)
		{
			.DataLength    = CPU_TO_LE32(PIMA_COMMAND_SIZE(1)),
			.Type          = CPU_TO_LE16(PIMA_CONTAINER_CommandBlock),
			.Code          = CPU_TO_LE16(0x1002),
			.Params        = {CPU_TO_LE32(1)},
		};

	if ((ErrorCode = SI_Host_SendBlockHeader(SIInterfaceInfo, &PIMABlock)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	if ((ErrorCode = SI_Host_ReceiveBlockHeader(SIInterfaceInfo, &PIMABlock)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	if ((PIMABlock.Type != CPU_TO_LE16(PIMA_CONTAINER_ResponseBlock)) || (PIMABlock.Code != CPU_TO_LE16(0x2001)))
	  return SI_ERROR_LOGICAL_CMD_FAILED;

	SIInterfaceInfo->State.IsSessionOpen = true;

	return PIPE_RWSTREAM_NoError;
}

uint8_t SI_Host_CloseSession(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo)
{
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	uint8_t ErrorCode;

	PIMA_Container_t PIMABlock = (PIMA_Container_t)
		{
			.DataLength    = CPU_TO_LE32(PIMA_COMMAND_SIZE(1)),
			.Type          = CPU_TO_LE16(PIMA_CONTAINER_CommandBlock),
			.Code          = CPU_TO_LE16(0x1003),
			.Params        = {CPU_TO_LE32(1)},
		};

	if ((ErrorCode = SI_Host_SendBlockHeader(SIInterfaceInfo, &PIMABlock)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	if ((ErrorCode = SI_Host_ReceiveBlockHeader(SIInterfaceInfo, &PIMABlock)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	SIInterfaceInfo->State.IsSessionOpen = false;

	if ((PIMABlock.Type != CPU_TO_LE16(PIMA_CONTAINER_ResponseBlock)) || (PIMABlock.Code != CPU_TO_LE16(0x2001)))
	  return SI_ERROR_LOGICAL_CMD_FAILED;

	return PIPE_RWSTREAM_NoError;
}

uint8_t SI_Host_SendCommand(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo,
                            const uint16_t Operation,
                            const uint8_t TotalParams,
                            uint32_t* const Params)
{
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	uint8_t ErrorCode;

	PIMA_Container_t PIMABlock = (PIMA_Container_t)
		{
			.DataLength    = cpu_to_le32(PIMA_COMMAND_SIZE(TotalParams)),
			.Type          = CPU_TO_LE16(PIMA_CONTAINER_CommandBlock),
			.Code          = cpu_to_le16(Operation),
		};

	memcpy(&PIMABlock.Params, Params, sizeof(uint32_t) * TotalParams);

	if ((ErrorCode = SI_Host_SendBlockHeader(SIInterfaceInfo, &PIMABlock)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	return PIPE_RWSTREAM_NoError;
}

uint8_t SI_Host_ReceiveResponse(USB_ClassInfo_SI_Host_t* const SIInterfaceInfo)
{
	uint8_t ErrorCode;
	PIMA_Container_t PIMABlock;
	uint8_t portnum = SIInterfaceInfo->Config.PortNumber;

	if ((USB_HostState[portnum] != HOST_STATE_Configured) || !(SIInterfaceInfo->State.IsActive))
	  return PIPE_RWSTREAM_DeviceDisconnected;

	if ((ErrorCode = SI_Host_ReceiveBlockHeader(SIInterfaceInfo, &PIMABlock)) != PIPE_RWSTREAM_NoError)
	  return ErrorCode;

	if ((PIMABlock.Type != CPU_TO_LE16(PIMA_CONTAINER_ResponseBlock)) || (PIMABlock.Code != CPU_TO_LE16(0x2001)))
	  return SI_ERROR_LOGICAL_CMD_FAILED;

	return PIPE_RWSTREAM_NoError;
}

#endif




#ifndef _SDCARD_H_
#define _SDCARD_H_

//Standard type definitions
#include "lpc_types.h"

#define SDCARD_IDENTIFICATION_MODE_CLOCK_RATE_HZ   400000l

//Registers
#define SDCARD_REGISTER_OCR		0x00
#define SDCARD_REGISTER_CID		0x01
#define SDCARD_REGISTER_CSD		0x02
#define SDCARD_REGISTER_RCA		0x03
#define SDCARD_REGISTER_DSR		0x04
#define SDCARD_REGISTER_SCR		0x05

//values for response R1
#define SDCARD_OK				0x00
#define SDCARD_IDLE				0x01
#define SDCARD_ERASE_RST		0x02
#define SDCARD_ILLEGAL_CMD		0x04
#define SDCARD_CRC_ERROR		0x08
#define SDCARD_ERASE_ERROR		0x10
#define SDCARD_ADDRESS_ERROR	0x20
#define SDCARD_PARAM_ERROR		0x40

//Definitions for control tokens
#define SDCARD_DATA_RESPONSE_TOKEN_MASK				0x1F
#define SDCARD_DATA_RESPONSE_TOKEN_OK				0x05
#define SDCARD_DATA_RESPONSE_TOKEN_CRC_ERROR		0x0B
#define SDCARD_DATA_RESPONSE_TOKEN_WRITE_ERROR		0x0D

#define SDCARD_START_BLOCK_TOKEN					0xFE
#define SDCARD_START_MULTI_BLOCK_WRITE_TOKEN		0xFC
#define SDCARD_STOP_TRANS_TOKEN						0xFD
#define SDCARD_BUSY_TOKEN							0x00

#define SDCARD_DATA_ERROR_TOKEN_MASK		0x0F
#define SDCARD_DATA_ERROR_TOKEN_RANGE		0x08
#define SDCARD_DATA_ERROR_TOKEN_ECC			0x04
#define SDCARD_DATA_ERROR_TOKEN_CC			0x02
#define SDCARD_DATA_ERROR_TOKEN_ERROR		0x01

#define SDCARD_CMD8_TEST_CODE				0xAA



typedef enum
{
  SDCard_Ok = 0, 
  SDCard_NotPresent, 
  SDCard_NoResponse, 
  SDCard_CardError, 
  SDCard_DataError,
  SDCard_OtherError, 
  SDCard_ParameterError, 
  SDCard_MisCompare
} 
SDCard_State_t;

typedef enum
{
  CMD0 = 0, // Resets the MultiMediaCard
  CMD1,     // Activates the card initialization process
  CMD8,		// Asks card for allowable voltage (SD v2.0 only, must come after CMD0 for SD v2.0+)
  CMD9,     // Asks the selected card to send its card-specific data (CSD)
  CMD10,    // Asks the selected card to send its card identification (CID)
  CMD12,    // Stop transmission on multiple block read
  CMD13,    // Asks the selected card to send its status register
  CMD16,    // Selects a block length (in bytes) for all following block commands (read and write)
  CMD17,    // Reads a block of the size selected by the SET_BLOCKLEN command
  CMD18,    // Continuously transfers data blocks from card to host until interrupted by a Stop command or the requested number of data blocks transmitted
  CMD24,    // Writes a block of the size selected by the SET_BLOCKLEN command
  CMD25,    // Continuously writes blocks of data until a "Stop Tran" token or the requested number of blocks received
  CMD27,    // Programming of the programmable bits of the CSD
  CMD28,    // If the card has write protection features, this command sets the write protection bit of the addressed group. The properties of write protection are coded in the card specific data (WP_GRP_SIZE).
  CMD29,    // If the card has write protection features, this command clears the write protection bit of the addressed group
  CMD30,    // If the card has write protection features, this command asks the card to send the status of the write protection bits
  CMD32,    // Sets the address of the first sector of the erase group
  CMD33,    // Sets the address of the last sector in a continuous range within the selected erase group, or the address of a single sector to be selected for erase
  CMD34,    // Removes one previously selected sector from the erase selection
  CMD35,    // Sets the address of the first erase group within a range to be selected for erase
  CMD36,    // Sets the address of the last erase group within a continuous range to be selected for erase
  CMD37,    // Removes one previously selected erase group from the erase selection.
  CMD38,    // Erases all previously selected sectors
  CMD42,    // Used to set/reset the password or lock/unlock the card. The size of the Data Block is defined by the SET_BLOCK_LEN command
  CMD55,    // Notifies the card that the next command is an application specific command rather than a standard command.
  CMD56,    // Used either to transfer a Data Block to the card or to get a Data Block from the card for general purpose/application specific commands. The size of the Data Block is defined with SET_BLOCK_LEN command
  CMD58,    // Reads the OCR register of a card
  CMD59,    // Turns the CRC option on or off. A ‘1’ in the CRC option bit will turn the option on, a ‘0’ will turn it off
  ACMD41,   // Activates the card’s initialization process (Only for SD)
  CMD_END   // End of commands index
} 
SDCard_CommandIndex_t;

typedef enum
{
  SDCard_R1 = 0,
  SDCard_R1_Data,
  SDCard_R1b,
  SDCard_R2,
  SDCard_R3,
  SDCard_R7
} 
SDCard_ResponseType_t;

typedef struct
{
  uint8_t            		txData;
  SDCard_ResponseType_t 	resp;
} 
SDCard_Command_t;

typedef enum
{
  DISKTYPE_SDv1 = 0,
  DISKTYPE_SDv2,
  DISKTYPE_SDv2HC,
  DISKTYPE_SDv2XC,
  DISKTYPE_UNKNOWN,
} 
diskType_e;

typedef struct
{
	uint8_t				CSD_Structure;			//CSD structure type
	uint32_t			sizeKB;					//Size of the card in kB
	uint16_t			blockSize;				//Block size, should always be 512
	diskType_e			diskType;				//SD card type
	uint8_t				writeProtect;			//Card write protected status
	uint32_t			readTimeoutBytes;		//The read timout in bytes
	uint32_t			writeTimeoutBytes;		//the write timeout in bytes
	Bool				cardPresent;			//TRUE if card is detected
	Bool				cardInitalized;			//TRUE if card is initalized
} 
SDCard_DiskStatus_t;

//Global variables
SDCard_DiskStatus_t SDCard_Status;











uint8_t SDTest (uint8_t in);

/** Initalizes the SD card
*
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Init (void);

/** Send command to the SD card.
*		
* \param[in]	commandIndex The command to send.
* \param[in]	arg The argument data to send.
* \return 		The data from the card. The return data depends on the response code of the command.
*/
uint32_t SDCard_SendCmd (SDCard_CommandIndex_t commandIndex, uint32_t arg);

/** Sends CMD13 to get the status of the card
*
* \return 		The two-bit status register of the card.
*/
uint16_t SDCard_GetStatus(void);

/** Set the transfer speed of the card.
*		This function should be used to set the transfer frequency as it also modifies the timeouts properly
* \param[in]	TransferSpeed The transfer speed to set (in Hz).
* \return 		TRUE if transfer speed is valid.
*/
Bool SDCard_SetTransferSpeed(uint32_t TransferSpeed);

/** Read register from the card
*
* \param[in]	Register The command to send.
* \param[in]	*pData Pointer to the variable to store the data
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_ReadRegister(uint8_t Register, uint8_t *pData);

/** Read data from the card
*
* \param[in]	*pData Pointer to the variable to store the data.
* \param[in]	address The block number to read.
* \param[in]	length	The number of blocks to read.
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Read(uint8_t *pData, uint32_t address, uint32_t length);

/** Write data to the card
*
* \param[in]	*pData Pointer to the data to write.
* \param[in]	address The block number to read.
* \param[in]	length	The number of blocks to read.
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Write(const uint8_t *pData, uint32_t address, uint32_t length);

/** Verify data written to the card
*
* \param[in]	*pData Pointer to the data to to verify against.
* \param[in]	address The block number to start the comparison.
* \param[in]	length	The number of blocks to compare.
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Verify(uint8_t *pData, uint32_t address, uint32_t length);

#endif 

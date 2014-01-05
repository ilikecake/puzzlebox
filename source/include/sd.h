//  SD Card Hardware Interface Header for LPC17XX
//  9/4/2011 - PDS

#ifndef _SD_H_
#define _SD_H_

int SDHwInit( void );
void SDChipSelect (int select);
int SDSendByte (int ByteToSend);

#endif
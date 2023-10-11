/*
 * ExternalCmd_M.c
 *
 * Created: 26.08.2019 09:47:57
 *  Author: root
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "USART.h"
#include "BillValidator.h"
#include "MDB.h"

void EXTCMD_PROCESS() {//receive commands from VMC
	if (EXTCMDCOMPLETE)
	{
		uint8_t check1 = EXT_UART_BUFFER[0];
		if (check1 == 0x05)
		{
			uint8_t command = EXT_UART_BUFFER[1];
			switch (command)
			{
				case 0x01:
				memcpy(&BillValidatorSlaveSetupData, &EXT_UART_BUFFER[2], 27);
				break;
				case 0x02:
				BillValidatorInsertedByte = 0x80 | ((EXT_UART_BUFFER[3] & 0x07) << 4) | (EXT_UART_BUFFER[2] & 0x0f);
				if (EXT_UART_BUFFER[3] == 0x00)
				{
					 BillValidatorStackerCount = (BillValidatorStackerCount + 1) % 0xffff;
				}
				BillValidatorDevice.Status = 2;
				break;
				case 0x03:
				BillValidatorDevice.Status = 0;
				break;
				case 0x04:
				memcpy(&BillValidatorSlaveIDData, &EXT_UART_BUFFER[2], 29);
				//unsigned char buff[8];
				//for (int a = 0; a < 28; a++){
					//sprintf(&buff, "%02x ", BillValidatorSlaveIDData[a]);
					//EXT_UART_Transmit(buff);
				//}
				//sprintf(&buff, "%02x\r\n", BillValidatorSlaveIDData[28]);
				//EXT_UART_Transmit(buff);
				break;
				default:
				/* Your code here */
				break;
			}
			//EXT_UART_Transmit("SLAVE#BVCMD#OK\r\n");
		}
		EXT_UART_BUFFER_COUNT = 0;
		EXTCMDCOMPLETE = 0;
	}
}
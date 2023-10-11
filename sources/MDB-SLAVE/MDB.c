/*
 * MDB.c
 *
 * Created: 11.05.2019 10:24:49
 *  Author: root
 */ 
#ifndef F_CPU
#define F_CPU       16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include "Cashless.h"
#include "Settings.h"
#include "MDB.h"
#include "BillValidator.h"

void MDBCommandHandler()
{
	uint16_t data = MDB_Receive();
	if (!MDBReceiveErrorFlag && ((data & 0x100) == 0x100))
	{
		memcpy(&MDB_BUFFER[MDB_BUFFER_COUNT++], &data, 2);
		switch (MDB_BUFFER[0].data & 0xf8)
		{
			//case 0x10:
			//if (MDBDeviceState[0]) CashlessCommandHandler(0);
			//break;
			//case 0x60:
			//if (MDBDeviceState[1]) CashlessCommandHandler(1);
			//break;
			case 0x30:
			HandleBillValidatorCommand(data & 0xff);
			default:
			break;
		}
	}
	MDB_Reset_Buffer();
}

void MDB_Reset_Buffer()
{
	MDBReceiveErrorFlag = 0;
	MDBReceiveComplete = 0;
	MDB_BUFFER_COUNT = 0;
}


/*
 * Settings.c
 *
 * Created: 11.05.2019 20:00:47
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
#include "USART.h"
#include "LEDControl.h"

void SetupPins()
{
	DDRA |= 0xff;
	PORTA = 0;
	DDRB |= 0xff;
	PORTB = 0;
	DDRC |= 0xff;
	PORTC = 0;
	DDRD |= 0xf0;
	PORTD &= ~(1 << PORTD0)|~(1 << PORTD1)|~(1 << PORTD2)|~(1 << PORTD3);
}

void ReadMDBDeviceState()
{
	eeprom_read_block((void*)&MDBDeviceState, (const void*)STORAGE_OFFSET, 5);
}

void WriteMDBDeviceState()
{
	eeprom_write_block((void*)&MDBDeviceState, (const void*)STORAGE_OFFSET, 5);
}

void ReadCashlessSetupData()
{
	eeprom_read_block((void*)&ReaderSetupData, (const void*)STORAGE_OFFSET + 5, 22);
}

void WriteCashlessSetupData()
{
	eeprom_write_block((void*)&ReaderSetupData, (const void*)STORAGE_OFFSET + 5, 22);
}

void ReadCashlessExpIDData()
{
	eeprom_read_block((void*)&ReaderPeriphID_Data, (const void*)STORAGE_OFFSET + 27, 68);
}

void WriteCashlessExpIDDData()
{
	eeprom_write_block((void*)&ReaderPeriphID_Data, (const void*)STORAGE_OFFSET + 27, 68);
}

void ReadCashlessRevalueAmountLimits()
{
	eeprom_read_block((void*)&ReaderRevalueAmountLimit[0], (const void*)STORAGE_OFFSET + 95, 4);
}

void WriteCashlessRevalueAmountLimits()
{
	eeprom_write_block((void*)&ReaderRevalueAmountLimit[0], (const void*)STORAGE_OFFSET + 95, 4);
}

void ReadExtUARTBaudRateID()
{
	EXT_UART_BAUDRATE = 38400;
	//EXT_UART_BAUDRATE_ID = eeprom_read_byte((const void*)STORAGE_OFFSET + 99);
	//switch (EXT_UART_BAUDRATE_ID)
	//{
		//case 2:
		//EXT_UART_BAUDRATE = 19200;
		//break;
		//case 3:
		//EXT_UART_BAUDRATE = 38400;
		//break;
		//default:
		//EXT_UART_BAUDRATE = 9600;
		//EXT_UART_BAUDRATE_ID = 1;
		//break;
	//}
	MYUBRR1 = F_CPU/16/EXT_UART_BAUDRATE-1;
	//ControlExtUartSpeedLEDs();
}

void WriteExtUARTBaudRateID()
{
	eeprom_write_byte((const void*)STORAGE_OFFSET + 99, EXT_UART_BAUDRATE_ID);
}
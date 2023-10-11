/*
 * Settings_M.c
 *
 * Created: 18.05.2019 10:11:12
 *  Author: root
 */ 
#define F_CPU 16000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "USART_M.h"
#include "Cashless_M.h"
#include "BillValidator_M.h"
#include "CoinChanger_M.h"
#include "CoinHopper_M.h"
#include "Settings_M.h"
#include "MDB_M.h"

VMCData_t EEMEM nv_VMCData = {3,16,2,1,"RUS","000000000000","000000000002","01"};
ReaderOptions_t EEMEM nv_ReaderOptions[2] = {{0xffffffff, 0x00000000,{0x10,0x18}, {0,1,1,1,0,1}},{0xffffffff, 0x00000000,{0x10,0x18}, {0,1,1,1,0,1}}};
CoinChangerOptions_t EEMEM nv_CoinChangerOptions = {0xffff,0xffff,0x07};
BillValidatorOptions_t EEMEM nv_BillValidatorOptions = {0xffff,0xffff,0x0000,0xffff,0x0000,1};
CoinHopperOptions_t EEMEM nv_CoinHopperOptions[2] = {{0xffff},{0xffff}};

void ReadVMCData()
{
	eeprom_read_block((void*)&VMCData, (const void*)&nv_VMCData, 33);
	EXT_UART_Transmit("SYS*VMCSET*READ*");
	EXT_UART_OK();
}

void ReadCashlessPrices()
{
	eeprom_read_block((void*)&ReaderOptions, (const void*)&nv_ReaderOptions, 32);
	EXT_UART_Transmit("SYS*CDSET*READ*");
	EXT_UART_OK();
}

void ReadCoinChangerOptions()
{
	eeprom_read_block((void*)&CoinChangerOptions, (const void*)&nv_CoinChangerOptions, 5);
	EXT_UART_Transmit("SYS*CCSET*READ*");
	EXT_UART_OK();
}

void WriteCoinChangerOptions()
{
	eeprom_write_block((void*)&CoinChangerOptions, (const void*)&nv_CoinChangerOptions, 5);
	EXT_UART_Transmit("SYS*CCSET*SAVE*");
	EXT_UART_OK();
}

void ResetCoinChangerOptions()
{
	uint8_t tmp[5] = {0xff,0xff,0xff,0xff,0x07};
	eeprom_write_block((void*)&tmp, (const void*)&nv_CoinChangerOptions, 5);
	eeprom_read_block((void*)&CoinChangerOptions, (const void*)&nv_CoinChangerOptions, 5);
	EXT_UART_Transmit("SYS*CCSET*RESET*");
	EXT_UART_OK();
}

void ReadBVOptions()
{
	eeprom_read_block((void*)&BillValidatorOptions, (const void*)&nv_BillValidatorOptions, 11);
	EXT_UART_Transmit("SYS*BVSET*READ*");
	EXT_UART_OK();
}

void WriteBVOptions()
{
	eeprom_write_block((void*)&BillValidatorOptions, (const void*)&nv_BillValidatorOptions, 11);
	EXT_UART_Transmit("SYS*BVSET*SAVE*");
	EXT_UART_OK();
}

void ResetBVOptions()
{
	uint8_t tmp[7] = {0xff,0xff,0xff,0xff,0x00,0x00,0xff,0xff,1};
	eeprom_write_block((void*)&tmp, (const void*)&nv_BillValidatorOptions, 11);
	eeprom_read_block((void*)&BillValidatorOptions, (const void*)&nv_BillValidatorOptions, 11);
	EXT_UART_Transmit("SYS*BVSET*RESET*");
	EXT_UART_OK();
}

void ReadCoinHoppersOptions()
{
	eeprom_read_block((void*)&CoinHopperOptions, (const void*)&nv_CoinHopperOptions, 4);
	EXT_UART_Transmit("SYS*CHSET*READ*");
	EXT_UART_OK();
}

void WriteCoinHoppersOptions()
{
	eeprom_write_block((void*)&CoinHopperOptions, (const void*)&nv_CoinHopperOptions, 4);
	EXT_UART_Transmit("SYS*CHSET*SAVE*");
	EXT_UART_OK();
}

void ResetCoinHoppersOptions()
{
	uint8_t tmp[4] = {0xff,0xff,0xff,0xff};
	eeprom_write_block((void*)&tmp, (const void*)&nv_CoinHopperOptions, 4);
	eeprom_read_block((void*)&CoinHopperOptions, (const void*)&nv_CoinHopperOptions, 4);
	EXT_UART_Transmit("SYS*CHSET*RESET*");
	EXT_UART_OK();
}

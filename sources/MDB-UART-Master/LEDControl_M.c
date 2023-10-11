/*
 * LEDControl_M.c
 *
 * Created: 18.05.2019 10:17:32
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
#include "Settings_M.h"
#include "LEDControl_M.h"

uint8_t * str_devonline = "SYS*DEVONLINE*";
uint8_t * str_devlost = "SYS*DEVLOST*";

void CCLED_ON()
{
	if (((PORTD >> PORTD6) & 1) != 1)
	{
		PORTD |= (1 << PORTD6);
		EXT_UART_Transmit(str_devonline);
		EXT_UART_Transmit("CC");
		EXT_CRLF();
	}
}

void CCLED_OFF()
{
	if (((PORTD >> PORTD6) & 1) == 1)
	{
		PORTD &= ~(1 << PORTD6);
		EXT_UART_Transmit(str_devlost);
		EXT_UART_Transmit("CC");
		EXT_CRLF();
	}
}

void BVLED_ON()
{
	if (((PORTC >> PORTC7) & 1) != 1)
	{
		PORTC |= (1 << PORTC7);
		EXT_UART_Transmit(str_devonline);
		EXT_UART_Transmit("BV");
		EXT_CRLF();
	}
}

void BVLED_OFF()
{
	if (((PORTC >> PORTC7) & 1) == 1)
	{
		PORTC &= ~(1 << PORTC7);
		EXT_UART_Transmit(str_devlost);
		EXT_UART_Transmit("BV");
		EXT_CRLF();
	}
}

void CHLED_ON(uint8_t index)
{
	if (index)
	{
		if (((PORTD >> PORTD5) & 1) != 1)
		{
			PORTD |= (1 << PORTD5);
			EXT_UART_Transmit(str_devonline);
			EXT_UART_Transmit("CH2");
			EXT_CRLF();
		}
	} else
	{
		if (((PORTD >> PORTD4) & 1) != 1)
		{
			PORTD |= (1 << PORTD4);
			EXT_UART_Transmit(str_devonline);
			EXT_UART_Transmit("CH1");
			EXT_CRLF();
		}
	}
}

void CHLED_OFF(uint8_t index)
{
	if (index)
	{
		if (((PORTD >> PORTD5) & 1) == 1)
		{
			PORTD &= ~(1 << PORTD5);
			EXT_UART_Transmit(str_devlost);
			EXT_UART_Transmit("CH2");
			EXT_CRLF();
		}
	} else
	{
		if (((PORTD >> PORTD4) & 1) == 1)
		{
			PORTD &= ~(1 << PORTD4);
			EXT_UART_Transmit(str_devlost);
			EXT_UART_Transmit("CH1");
			EXT_CRLF();
		}
	}
}

void CDLED_ON(uint8_t index)
{
	if (index) PORTC |= (1 << PORTC7); else PORTC |= (1 << PORTC6);
}

void CDLED_OFF(uint8_t index)
{
	if (index) PORTC &= ~(1 << PORTC7); else PORTC &= ~(1 << PORTC6);
}

void USDLED_ON(uint8_t index)
{
	if (index == 0) PORTC |= (1 << PORTC4);
	else
	if (index == 1) PORTC |= (1 << PORTC2);
	else
	if (index == 2) PORTC |= (1 << PORTC3);
}

void USDLED_OFF(uint8_t index)
{
	if (index == 0) PORTC &= ~(1 << PORTC4);
	else
	if (index == 1) PORTC &= ~(1 << PORTC2);
	else
	if (index == 2) PORTC &= ~(1 << PORTC3);
}


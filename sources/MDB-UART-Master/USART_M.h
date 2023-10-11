/*
 * USART_M.h
 *
 * Created: 18.05.2019 09:47:23
 *  Author: root
 */ 
#include "MDB_M.h"

#ifndef USART_M_H_
#define USART_M_H_

uint8_t EXT_UART_BUFFER[32];
MDB_Byte MDB_BUFFER[37];
volatile uint8_t EXT_UART_BUFFER_COUNT;
volatile uint8_t MDB_BUFFER_COUNT;
volatile uint8_t EXTCMDCOMPLETE;


//MDB receiving flags
volatile uint8_t MDBReceiveComplete;  //MDB message receive completed flag
volatile uint8_t MDBReceiveErrorFlag;  //MDB message receive error flag

void MDB_Setup(void);
void EXT_UART_Setup(void);
void EXT_UART_Transmit(uint8_t data[]);
void EXT_CRLF(void);
void EXT_UART_FAIL(void);
void EXT_UART_OK(void);
void EXT_UART_NAK(void);
void MDB_ACK(void);
void MDB_Send(uint8_t data[], uint8_t len);
void MDB_getByte(MDB_Byte* mdbb);
void MDB_read(void);
uint8_t MDB_ChecksumValidate(void);
int MDB_Receive(void);
void delay_1ms(uint16_t ms);

#endif /* USART_M_H_ */
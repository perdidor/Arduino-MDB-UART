/*
 * MDB.h
 *
 * Created: 11.05.2019 10:20:42
 *  Author: root
 */ 


#ifndef MDB_H_
#define MDB_H_

// MDB 9-bit defined as two bytes
typedef struct {
	unsigned char data;
	unsigned char mode;
} MDB_Byte;

typedef struct {
	uint8_t Status;
	uint8_t OfflinePollsCount;
} mdbdevice;

volatile uint8_t MDB_BUFFER_COUNT;
volatile MDB_Byte MDB_BUFFER[37];
//MDB receiving flags
unsigned char MDBReceiveComplete;  //MDB message receive completed flag
unsigned char MDBReceiveErrorFlag;  //MDB message receive error flag

void MDBCommandHandler(void);
void HandleCashlessCommand(int CommandByte);

#endif /* MDB_H_ */
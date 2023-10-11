/*
 * MDB_M.h
 *
 * Created: 18.05.2019 09:56:59
 *  Author: root
 */ 


#ifndef MDB_M_H_
#define MDB_M_H_

// Address of each peripheral device type (MDB Version 4.2, page 25)
#define ADDRESS_CHANGER   (0x08)  // CoinChanger
#define ADDRESS_CD1       (0x10)  // Cashless Device 1
#define ADDRESS_VALIDATOR (0x30)  // BillValidator
#define ADDRESS_USD1      (0x40)  // Universal satellite device 1
#define ADDRESS_USD2      (0x48)  // Universal satellite device 2
#define ADDRESS_USD3      (0x50)  // Universal satellite device 3
#define ADDRESS_COIN1     (0x58)  // Coin Hopper 1
#define ADDRESS_CD2       (0x60)  // Cashless Device 2
#define ADDRESS_COIN2     (0x70)  // Coin Hopper 2

// MDB 9-bit defined as two bytes
typedef struct {
	uint8_t data;
	uint8_t mode;
} MDB_Byte;

typedef struct {
	uint8_t Status;
	uint8_t OfflinePollsCount;
} mdbdevice;

void DebugMDBMessage(void);
void ProcessMDBResponse(uint8_t addr);
void MDBDeviceReset(uint8_t DevAddress);
void ResetAll(void);
void PollDevice(uint8_t address);
void PollReader(uint8_t index);

void MDBDebug(void);



#endif /* MDB_M_H_ */
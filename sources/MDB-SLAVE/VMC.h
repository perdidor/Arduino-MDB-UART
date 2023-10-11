/*
 * VMC.h
 *
 * Created: 11.05.2019 18:59:05
 *  Author: root
 */ 


#ifndef VMC_H_
#define VMC_H_

typedef struct {
	uint8_t feature_level;
	uint8_t dispaly_cols;
	uint8_t dispaly_rows;
	uint8_t dispaly_info;
} VMCConfig_t;

typedef struct {
	unsigned char ManufacturerCode[3];
	unsigned char SerialNumber[12];
	unsigned char ModelRevision[12];
	uint16_t SoftwareVersion;
} VMCID_t;

typedef union {
	uint32_t Value;
	uint8_t Bytes[4];
} VMCPrice_t;

VMCConfig_t ReaderVMCSetupData[2];
VMCPrice_t ReaderVMCMaxPrice[2];
VMCPrice_t ReaderVMCMinPrice[2];
VMCID_t ReaderVMCIDData[2];

unsigned char (CommandsArray[])[6];

void VMCCommandHandler(void);
void ResetReaderVMCData(unsigned char index);
void CashlessExtCommandHandler(unsigned char index);

#endif /* VMC_H_ */
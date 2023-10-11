/*
 * settings.h
 *
 * Created: 11.05.2019 20:00:56
 *  Author: root
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

#define STORAGE_OFFSET	128

//Administrative state for every supported device (0x01=Enabled\0x00=Disabled)
uint8_t MDBDeviceState[5];

void SetupPins(void);
void ReadExtUARTBaudRateID(void);
void WriteExtUARTBaudRateID(void);
void ReadMDBDeviceState(void);
void WriteMDBDeviceState(void);
void ReadCashlessSetupData(void);
void WriteCashlessSetupData(void);
void ReadCashlessExpIDData(void);
void WriteCashlessExpIDDData(void);
void ReadCashlessRevalueAmountLimits(void);
void WriteCashlessRevalueAmountLimits(void);


#endif /* SETTINGS_H_ */
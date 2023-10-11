/*
 * IncFile1.h
 *
 * Created: 04.08.2020 19:30:21
 *  Author: root
 */ 


#ifndef BILLVALIDATOR_H_
#define BILLVALIDATOR_H_

#include "MDB.h"

uint8_t BillValidatorSlaveSetupData[27];

uint8_t BillValidatorSlaveIDData[29];

uint8_t BillValidatorInsertedByte;

mdbdevice BillValidatorDevice;

uint16_t BillValidatorStackerCount;

uint16_t BillValidatorVMCEnabledBillTypes;
uint16_t BillValidatorVMCEscrowBillTypes;

uint8_t IsEnabled;

void BillValidator_Poll(void);
void BillValidator_SetupData(void);
void BillValidator_Reset(void);
void BillValidator_Stacker(void);
void BillValidator_IDData(void);
void BillValidator_Security(void);
void BillValidator_BillType(void);
void BillValidator_Escrow(void);
void HandleBillValidatorCommand(int command);



#endif /* BILLVALIDATOR_H_ */
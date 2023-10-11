/*
 * BillValidator_M.h
 *
 * Created: 18.05.2019 10:05:57
 *  Author: root
 */ 
#include "MDB_M.h"

#ifndef BILLVALIDATOR_M_H_
#define BILLVALIDATOR_M_H_

typedef struct {
	uint8_t BillValidatorFeatureLevel;
	uint16_t CountryOrCurrencyCode;
	uint16_t BillScalingFactor;
	uint8_t DecimalPlaces;
	uint16_t StackerCapacity;
	uint8_t Escrow;
	uint8_t BillSecurityLevel[16];
	uint8_t BillRecycleEnabled[16];
	uint16_t BillTypeCredit[16];
} basetupdata;

typedef struct {
	uint16_t SoftwareVersion;
	uint8_t BillRecyclingSupported;
	uint8_t FTLSupported;
	uint8_t ManufacturerCode[3];
	uint8_t SerialNumber[12];
	uint8_t ModelRevision[12];
} baiddata;

baiddata BillValidatorIDData;
basetupdata BillValidatorSetupData;
mdbdevice BillValidatorDevice;

void BillValidatorPollResponse(void);
void GetBillValidatorIdentification(void);
void BillValidatorEnableFeatures(void);
void GetBillValidatorStackerStatus(void);
void GetBillValidatorSetupData(void);
void GetBillRecyclerSetupData(void);
void GetBVDispenserStatus(void);
void BVDispenseBills(uint8_t BillType, uint16_t Number);
void BVDispenseValue(uint16_t PayoutValue);
void BillValidatorPayoutStatus(void);
void BillValidatorPayoutValue(void);
void BillValidatorCancelPayout(void);
void BillValidatorEscrow(uint8_t action);
void BillValidatorEnableBillType(uint8_t BillType, uint8_t EnableAccept, uint8_t EnableEscrow, uint8_t EnableRecycle, uint8_t EnableManualDispense, uint8_t HighSecurityLevel);
void BillValidatorRecyclerEnable(void);
void BillValidatorRecyclerDisable(void);
void BillValidatorEnableAcceptBills(void);
void BillValidatorDisableAcceptBills(void);
void BillValidatorConfigFeatures(uint8_t RecyclerEnable);
void BillValidatorSetSecurityLevels(void);




#endif /* BILLVALIDATOR_M_H_ */
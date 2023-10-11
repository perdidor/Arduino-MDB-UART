/*
 * Cashless.h
 *
 * Created: 11.05.2019 09:35:32
 *  Author: root
 */ 


#ifndef CASHLESS_H_
#define CASHLESS_H_

//---------------------------------------------------------------------------
//  SESSION INITITATOR
//---------------------------------------------------------------------------
enum SESSION_INITIATOR {INITIATOR_VMC, INITIATOR_READER};
	
//---------------------------------------------------------------------------
//  SESSION STAGE
//---------------------------------------------------------------------------
enum SESSION_STAGE {SESSION_NONE};

//---------------------------------------------------------------------------
//  CASHLESS READER STATES
//---------------------------------------------------------------------------
enum MDB_STATES {MDB_INACTIVE,MDB_DISABLED,MDB_ENABLED,MDB_SESSION_IDLE,MDB_VENDING,MDB_REVALUE,MDB_NEGATIVE_VEND};

//---------------------------------------------------------------------------
//  CASHLESS READER COMMANDS
//---------------------------------------------------------------------------
enum MDB_CMD {MDB_IDLE,MDB_RESET = 0x10,MDB_SETUP,MDB_POLL,MDB_VEND,MDB_READER,MDB_EXPANSION};

//---------------------------------------------------------------------------
//  CASHLESS VEND RESULT
//---------------------------------------------------------------------------
enum VEND_RESULT {VEND_UNKNOWN,VEND_APPROVE,VEND_DENY};
	
//---------------------------------------------------------------------------
//  CASHLESS ERRORS
//---------------------------------------------------------------------------
enum READER_ERROR {ERROR_UNKNOWN,ERROR_PAYMENT_MEDIA,INVALID_PAYMENT_MEDIA,TAMPER_ERROR,MFG_DEFINED_ERROR1,COMM_ERROR,SERVICE_REQUIRED,MFG_DEFINED_ERROR2,READER_FAILURE,COMM_ERROR2,PAYMENT_MEDIA_JAMMED,REFUND_ERROR};

//---------------------------------------------------------------------------
//  CASHLESS READER POLL REPLYS
//---------------------------------------------------------------------------
enum POLL_REPLY {MDB_REPLY_SILENCE, MDB_REPLY_ACK, MDB_REPLY_JUST_RESET, MDB_REPLY_READER_CFG, MDB_REPLY_DISPLAY_REQ, MDB_REPLY_BEGIN_SESSION,
	MDB_REPLY_SESSION_CANCEL_REQ, MDB_REPLY_VEND_APPROVED, MDB_REPLY_VEND_DENIED, MDB_REPLY_END_SESSION,
	MDB_REPLY_CANCELLED, MDB_REPLY_PERIPHERIAL_ID, MDB_REPLY_ERROR, MDB_REPLY_CMD_OUT_SEQUENCE, MDB_REPLY_REVALUE_APPROVED,
	MDB_REPLY_REVALUE_DENIED, MDB_REPLY_REVALUE_LIMIT_AMOUNT, MDB_REPLY_TIMEDATE_REQUEST, MDB_REPLY_DATAENTRY_REQUEST, MDB_REPLY_DATAENTRY_CANCEL};

typedef struct {
	uint8_t  CFG_Constant;
	uint8_t  Feature_Level;
	uint16_t Country_Code;
	uint8_t  Scale_Factor;
	uint8_t  Decimal_Places;
	uint8_t  MaxResp_Time;
	uint8_t  Misc_Options_RefundAvailable;
	uint8_t  Misc_Options_MultivendSupport;
	uint8_t  Misc_Options_DisplayPresent;
	uint8_t  Misc_Options_VendCashSaleSupport;
} CashlessConfig_t;

typedef struct {
	uint16_t Funds;
	uint32_t MediaID;
	uint8_t PaymentType;
	uint16_t PaymentData;
} Begin_t;

typedef struct {
	uint8_t Result;
	uint16_t Amount;
} Result_t;

typedef struct {
	Begin_t StartData;
	Result_t ResultData;
	uint8_t Initiator;
} ReaderMDBSession_t;

typedef struct {
	uint8_t Repeated;
	uint8_t EntryLength;
} ReaderDataEntryRequest_t;

typedef struct {
	uint8_t DisplayTime;
	uint8_t DisplayData[32];
} ReaderDisplayRequest_t;

typedef struct {
	uint8_t ID_Constant;
	unsigned char ManufacturerCode[3];
	unsigned char SerialNumber[12];
	unsigned char ModelRevision[12];
	uint16_t SoftwareVersion;
	uint32_t OptionalFeatureBits;
} ReaderExpID_t;

typedef struct {
	uint8_t NegativeVendEnabled;
	uint8_t DataEntryEnabled;
	uint8_t AlwaysIdleEnabled;
} ReaderOptFeatures_t;

CashlessConfig_t ReaderSetupData[2];
ReaderMDBSession_t ReaderSessionData[2];
ReaderExpID_t ReaderPeriphID_Data[2];
ReaderDisplayRequest_t ReaderDisplayRequestData[2];
ReaderDataEntryRequest_t ReaderDataEntryRequest[2];
ReaderOptFeatures_t ReaderOptFeatures[2];

uint8_t Reader_State[2];
uint8_t Reader_Poll_Reply[2];
uint8_t Reader_Active_Command[2];
uint16_t ReaderRevalueAmountLimit[2];

void Reader_Reset(unsigned char index);
void Reader_Setup(unsigned char index);
void Reader_Expansion(unsigned char index);
void Reader_Poll(unsigned char index);
void ReaderSetIdle(unsigned char index);
void Reader_Reader(unsigned char index);
void Reader_Revalue(unsigned char index);
void Reader_Vend(unsigned char index);
void HandleCashlessCommand(int CommandByte);


#endif /* CASHLESS_H_ */
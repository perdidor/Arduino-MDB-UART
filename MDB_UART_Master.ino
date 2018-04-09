//------------------------------------------------------------------------------
// Arduino as MDB Master
// 25.03.2018 Copyright Vladimir Pakhomov ga3@yandex.ru
// inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer
//
// This is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// For full text of GPL see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------


// CONSTANTS
#define BAUD 9600

#define ADDRESS_MASK      (0xF8)  // section 2.3 - top five bits of address are actually address
#define COMMAND_MASK      (0x07)  // section 2.3 - bottom three bits are command

// Address of each peripheral device type (MDB Version 4.2, page 25)
#define ADDRESS_VMC       (0x00)  // Reserved!!!
#define ADDRESS_CHANGER   (0x08)  // Coin Changer
#define ADDRESS_CD1       (0x10)  // Cashless Device 1
#define ADDRESS_GATEWAY   (0x18)  // Communication Gateway
#define ADDRESS_DISPLAY   (0x20)  // Display
#define ADDRESS_EMS       (0x28)  // Energy Management System
#define ADDRESS_VALIDATOR (0x30)  // Bill Validator
#define ADDRESS_USD1      (0x40)  // Universal satellite device 1
#define ADDRESS_USD2      (0x48)  // Universal satellite device 2
#define ADDRESS_USD3      (0x50)  // Universal satellite device 3
#define ADDRESS_COIN1     (0x58)  // Coin Hopper 1
#define ADDRESS_CD2       (0x60)  // Cashless Device 2
#define ADDRESS_AVD       (0x68)  // Age Verification Device
#define ADDRESS_COIN2     (0x70)  // Coin Hopper 2

//it's for auto setting baudrate
#include <util/setbaud.h>

// MDB 9-bit defined as two bytes
struct MDB_Byte {
  byte data;
  byte mode;
};

//array of *POLL* commands for every device type
byte POLL_ADDRESS[10]{0x0B, 0x12, 0x1A, 0x33, 0x42, 0x4A, 0x52, 0x5B, 0x62, 0x73};

byte EXT_UART_BUFFER[37]; //incoming buffer for receive data from VMC
struct MDB_Byte MDB_BUFFER[37]; //incoming buffer for receive data from MDB peripheral
int EXT_UART_BUFFER_COUNT;
volatile int MDB_BUFFER_COUNT;

//MDB receiving flags
int rcvcomplete;  //MDB message receive completed flag
int mdboverflow;  //MDB message receive error flag



void MDB_Setup() {
  // Set baud rate with setbaud.h
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
  // Disable USART rate doubler (arduino bootloader leaves it enabled...)
  UCSR0A &= ~(1 << U2X0);
  // Set 9600-9-N-1 UART mode
  UCSR0C = (0<<UMSEL01)|(0<<UMSEL00)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);
  UCSR0B |= (1<<UCSZ02); // 9bit
  // Enable rx/tx
  UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}

void EXT_UART_Setup()
{
  Serial1.begin(9600);
  Serial1.println("MDB-UART PLC ready");
}

void EXT_UART_read() {//receive commands from VMC
  EXT_UART_BUFFER_COUNT = 0;
  while (Serial1.available()) {
    // buffer the data
    EXT_UART_BUFFER[EXT_UART_BUFFER_COUNT++]=Serial1.read();
  }
  if ((EXT_UART_BUFFER_COUNT > 0) && EXT_ChecksumValidate()) {//command received, checksum is valid, proceeding
    bool IsAddressValid = false;
    switch(EXT_UART_BUFFER[0] & ADDRESS_MASK) {//check first byte for peripheral address validity
    case ADDRESS_CHANGER  :  IsAddressValid = true;  break;
    case ADDRESS_GATEWAY  :  IsAddressValid = true;  break;
    case ADDRESS_DISPLAY  :  IsAddressValid = true;  break;
    case ADDRESS_EMS      :  IsAddressValid = true;  break;
    case ADDRESS_VALIDATOR:  IsAddressValid = true;  break;
    case ADDRESS_AVD      :  IsAddressValid = true;  break;
    case ADDRESS_CD1      :  IsAddressValid = true;  break;
    case ADDRESS_CD2      :  IsAddressValid = true;  break;
    case ADDRESS_USD1     :  IsAddressValid = true;  break;
    case ADDRESS_USD2     :  IsAddressValid = true;  break;
    case ADDRESS_USD3     :  IsAddressValid = true;  break;
    case ADDRESS_COIN1    :  IsAddressValid = true;  break;
    case ADDRESS_COIN2    :  IsAddressValid = true;  break;
    
    default:
      break;
    }
    if (IsAddressValid){ //command seems valid, let's try to send it to peripheral
      struct MDB_Byte AddressByte;
      int addrtx = 0;
      AddressByte.data = EXT_UART_BUFFER[0];
      AddressByte.mode = 0x01;
      memcpy(&addrtx, &AddressByte, 2);
      MDB_write(addrtx);
      for (int i = 1; i < EXT_UART_BUFFER_COUNT; i++){
        MDB_write(EXT_UART_BUFFER[i]);
      }
      processresponse(EXT_UART_BUFFER[0] & ADDRESS_MASK);
    }
  }
}

void MDB_checksumGenerate() {
  byte sum = 0;
  for (int i=0; i < (EXT_UART_BUFFER_COUNT); i++)
    sum += EXT_UART_BUFFER[i];
  EXT_UART_BUFFER[EXT_UART_BUFFER_COUNT++] = (sum & 0xFF);//only first 8 bits are checksum
}

void MDB_write(int data) {
  struct MDB_Byte b;
  memcpy(&b, &data, 2);
  write_9bit(b);
}

void write_9bit(struct MDB_Byte mdbb) {
  while ( !( UCSR0A & (1<<UDRE0)));
  if (mdbb.mode) {
     //turn on bit 9
    UCSR0B |= (1<<TXB80);
  } else {
     //turn off bit 9
    UCSR0B &= ~(1<<TXB80);
  }
  UDR0 = mdbb.data;
}

int MDB_Receive() {
  unsigned char resh, resl;
  char tmpstr[64];
  int rtr = 0;
  // Wait for data to be received
  while ((!(UCSR0A & (1<<RXC0))) and rtr < 50) {
    delay(1);
    rtr++;
  }
  if (rtr == 50){
    mdboverflow = 1;
    rcvcomplete = 1;
  }
  // Get 9th bit, then data from buffer
  resh = UCSR0B;
  resl = UDR0;
  // Filter the 9th bit, then return only data w\o mode bit
  resh = (resh >> 1) & 0x01;
  return ((resh << 8) | resl);
}

void MDB_getByte(struct MDB_Byte* mdbb) {
  int b;
  b = 0;
  b = MDB_Receive();
  memcpy (mdbb, &b, 2);  
}

byte EXT_ChecksumValidate() {
  byte sum = 0;
  for (int i=0; i < (EXT_UART_BUFFER_COUNT-1); i++)
    sum += EXT_UART_BUFFER[i];
  if (EXT_UART_BUFFER[EXT_UART_BUFFER_COUNT-1] == (sum & 0xFF))
    return 1;
  else
    return 0;
}

byte MDB_ChecksumValidate() {
  int sum = 0;
  for (int i=0; i < (MDB_BUFFER_COUNT-1); i++)
    sum += MDB_BUFFER[i].data;
  if (MDB_BUFFER[MDB_BUFFER_COUNT-1].data == (sum & 0xFF))
    return 1;
  else
    return 0;
}

void MDB_read() {
  MDB_getByte(&MDB_BUFFER[MDB_BUFFER_COUNT]);
  MDB_BUFFER_COUNT++;
  if (MDB_BUFFER_COUNT == 35){
    rcvcomplete = 1;
    mdboverflow = 1;
  }
  if (MDB_BUFFER[MDB_BUFFER_COUNT - 1].mode && MDB_ChecksumValidate()){
    rcvcomplete = 1;
  }
}

void MDBFlush(){
  MDB_BUFFER_COUNT = 0;
  Serial.flush();
} 

void processresponse(int addr){
  mdboverflow = 0;
  rcvcomplete = 0;
  while (!rcvcomplete){
    MDB_read();
  }
  if ((rcvcomplete) && (!mdboverflow))
  {
    if (MDB_BUFFER_COUNT > 1){
      MDB_write(0x00);// send ACK to MDB if peripheral answer is not just *ACK*, otherwise peripheral will try to send unconfirmed data with next polls
    } else{
      if (MDB_BUFFER_COUNT == 1){
        //just *ACK* received from peripheral device, no confirmation needed
      }
    }
    //finally, send data from peripheral to VMC via serial port as string representation of hex bytes
    char buff[5];
    sprintf(buff, "%02x ", addr);
    Serial1.print(buff);
    for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
    sprintf(buff, "%02x ", MDB_BUFFER[a].data);
    Serial1.print(buff);  
    }
    //last byte representation will be sent without following space but with EOL to easy handle
    sprintf(buff, "%02x", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
    Serial1.println(buff);
  }
}

void PollDevice(byte devaddrbyte){
  struct MDB_Byte addrbyte;
  rcvcomplete = 0;
  int addrtx = 0;
  addrbyte.data = devaddrbyte;
  addrbyte.mode = 0x01;
  memcpy(&addrtx, &addrbyte, 2);
  MDB_write(addrtx);
  MDB_write(addrbyte.data);
  processresponse(addrbyte.data & ADDRESS_MASK);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(0, OUTPUT);
  MDB_Setup();
  EXT_UART_Setup();
  MDB_BUFFER_COUNT = 0;
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  EXT_UART_read();
  MDBFlush();
  for (int q = 0; q < 10; q++){
    PollDevice(POLL_ADDRESS[q]); 
    MDBFlush();  
    delay(20); 
  }
}

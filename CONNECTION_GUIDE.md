# MDB to UART Converter - Connection Guide

This guide provides detailed instructions for connecting and using the MDB-UART converter to interface MDB devices with a PC or microcontroller via RS-232/UART.

## Table of Contents
- [Overview](#overview)
- [Required Components](#required-components)
- [Safety Warnings](#safety-warnings)
- [Hardware Connections](#hardware-connections)
  - [Power Supply](#power-supply)
  - [MDB Device Connection](#mdb-device-connection)
  - [PC/Controller Connection](#pccontroller-connection)
- [Pinout Reference](#pinout-reference)
- [Testing and Verification](#testing-and-verification)
- [Communication Settings](#communication-settings)
- [Troubleshooting](#troubleshooting)

## Overview

The MDB-UART converter acts as a bridge between:
- **MDB Side**: 9-bit serial communication with MDB peripheral devices (coin changers, bill validators, cashless devices, etc.)
- **UART Side**: Standard 8-bit RS-232/UART communication with PC or microcontroller

```
┌─────────────┐          ┌──────────────────┐          ┌─────────────┐
│ PC/Computer │◄────────►│  MDB-UART        │◄────────►│ MDB Device  │
│ (RS-232)    │  Serial  │  Converter       │   MDB    │ (Coin/Bill) │
│  9600 8N1   │          │ (ATmega1284/644) │  9-bit   │  Acceptor   │
└─────────────┘          └──────────────────┘          └─────────────┘
                                   ▲
                                   │
                              24-34V DC
                              Power Supply
```

## Required Components

### Essential Items
1. **MDB-UART Converter** (assembled and programmed with firmware)
2. **Power Supply**: 24-34V DC, minimum 2A capacity
   - Vending machine power supply, or
   - Dedicated 24V/34V DC power adapter
3. **RS-232 Cable**: DB9 Female connector or USB-to-Serial adapter
4. **MDB Device**: Coin changer, bill validator, or other MDB peripheral
5. **MDB Cable**: Standard MDB 34-pin connector or appropriate adapter

### Optional Items
- Heat sink for voltage regulator (L78S05CV) - **HIGHLY RECOMMENDED**
- Multimeter for voltage verification
- Terminal emulator software (PuTTY, Tera Term, Arduino Serial Monitor)

## Safety Warnings

⚠️ **CRITICAL SAFETY INFORMATION:**

1. **Heat Management**:
   - ALWAYS install a heat sink on the voltage regulator (VR1 - L78S05CV)
   - Operating without heat sink may cause component failure and fire hazard
   - Voltage regulator dissipates significant heat when converting 24-34V to 5V

2. **Voltage Precautions**:
   - Verify power supply voltage before connection (24-34V DC)
   - Do NOT exceed 34V DC input voltage
   - Ensure correct polarity when connecting power supply
   - Reverse polarity may permanently damage the device

3. **ESD Protection**:
   - Handle the PCB by edges only
   - Use ESD wrist strap when working with the device
   - Avoid touching component pins and circuit traces

## Hardware Connections

### Power Supply

The converter requires 24-34V DC input power, typically sourced from vending machine power supply.

#### Power Connection Steps:

1. **Locate Power Connector** (Conn2 - Xinya 300-022-12):
   - Find the 2-pin power input connector on the PCB
   - Check connector labeling for polarity markings (+/-)

2. **Connect Power Wires**:
   ```
   Power Supply Connections:
   ┌────────────────────────┐
   │  Connector Conn2       │
   │  Pin 1: +24-34V DC     │ ←── Red wire from power supply (+)
   │  Pin 2: GND (Ground)   │ ←── Black wire from power supply (-)
   └────────────────────────┘
   ```

3. **Verify Voltage**:
   - Use multimeter to verify 24-34V DC at power supply output
   - Check polarity before making connection
   - Verify 5V DC at IC1 pin 10 (VCC) after power-up

4. **LED Indicators After Power-Up**:
   - **LED1 (Green)**: MDB Power indicator - should be ON
   - **LED2 (Red)**: MDB TX - blinks during transmission to MDB bus
   - **LED3 (Amber)**: MDB RX - blinks when receiving from MDB devices

### MDB Device Connection

MDB uses a 34-pin connector or simplified 5-pin interface. The converter communicates with MDB peripherals using this standardized bus.

#### MDB Connector Pinout (Conn1):

```
Standard MDB Connection (simplified):
┌───────────────────────────────────┐
│  MDB Connector (6-pin header)     │
│  Pin 1: MDB Bus +               │ ←── MDB data line positive (from device pin 5)
│  Pin 2: MDB Bus -               │ ←── MDB data line negative (from device pin 6)
│  Pin 3: MDB +24V                │ ←── 24-34V power to MDB device
│  Pin 4: MDB GND                 │ ←── Ground
│  Pin 5: (Optional features)      │
│  Pin 6: (Optional features)      │
└───────────────────────────────────┘

Standard MDB 34-pin connector mapping:
Pin 5:  MDB Data (+) / TX+
Pin 6:  MDB Data (-) / TX-
Pin 33: +24V Power
Pin 34: Ground
```

#### Connection Steps:

1. **Power OFF** both converter and MDB device before connections

2. **Identify MDB Cable Wires**:
   - Consult your MDB device datasheet for pinout
   - Standard colors (may vary by manufacturer):
     - Data+: White or Orange
     - Data-: Black or Blue
     - Power: Red
     - Ground: Black

3. **Connect MDB Wires to Converter**:
   - Match MDB device pins to converter Conn1
   - Ensure secure connections (no loose wires)
   - Double-check polarity for power lines

4. **Secure Connections**:
   - Use appropriate crimp connectors or screw terminals
   - Verify continuity with multimeter if available

### PC/Controller Connection

Connect the converter to your PC or microcontroller using RS-232 serial interface.

#### RS-232 Connector (Conn4 - DB9 Female):

```
DB9 Female Connector Pinout (viewed from solder side):
   ┌─────────────────┐
   │  5  4  3  2  1  │
   │   9  8  7  6    │
   └─────────────────┘

Pin Assignment:
Pin 2: RXD (Receive Data)  ──► Data FROM PC to Converter
Pin 3: TXD (Transmit Data) ──► Data FROM Converter to PC
Pin 5: GND (Signal Ground) ──► Common ground
Pins 1,4,6,7,8,9: Not used (internal connections for handshaking)
```

#### Connection Options:

**Option A: Direct RS-232 Connection**
- Use standard DB9 serial cable (straight-through, not null-modem)
- Connect DB9 male connector to PC COM port
- Connect DB9 female connector to converter Conn4

**Option B: USB-to-Serial Adapter**
- Use USB-to-Serial adapter with FTDI, Prolific, or CH340 chip
- Connect adapter's DB9 connector to converter
- Plugin USB to PC
- Install appropriate drivers if needed
- Note virtual COM port number assigned by OS

#### ISP Programming Connector (Conn3):

The 5-pin ISP connector (Conn3) is used for programming the ATmega chip:

```
ISP Connector (DS1021-1x5):
Pin 1: MISO  (Master In Slave Out)
Pin 2: VCC   (5V)
Pin 3: SCK   (Serial Clock)
Pin 4: MOSI  (Master Out Slave In)
Pin 5: RST   (Reset)
GND: Available on separate pin or shared with power ground
```

**Note**: This connector is only needed for firmware updates. Do not connect during normal operation.

## Pinout Reference

### Complete PCB Connector Summary:

| Connector | Type | Purpose | Pins | Notes |
|-----------|------|---------|------|-------|
| Conn1 | DS1073-01-2x3 | MDB Bus | 6-pin | 2x3 header for MDB connection |
| Conn2 | Xinya 300-022-12 | Power Input | 2-pin | 24-34V DC input |
| Conn3 | DS1021-1x5 | ISP Programming | 5-pin | For firmware updates only |
| Conn4 | DB9 Female | RS-232/UART | 9-pin | PC/Controller connection |

### LED Indicators:

| LED | Color | Function | Normal Behavior |
|-----|-------|----------|-----------------|
| LED1 | Green | MDB Power | Solid ON when MDB powered |
| LED2 | Red | MDB TX (Transmit) | Blinks when sending to MDB |
| LED3 | Amber | MDB RX (Receive) | Blinks when receiving from MDB |

## Testing and Verification

### Initial Power-Up Test:

1. **Connect Power Supply** (24-34V DC):
   - Green LED (LED1) should turn ON immediately
   - This indicates power supply is working

2. **Connect RS-232 to PC**:
   - Open terminal emulator software
   - Configure serial port settings (see below)

3. **Check Boot Message**:
   - Upon power-up, converter transmits version information
   - Expected output:
     ```
     SYS*MDBSTART*1.1.1730
     ```
   - If no MDB devices connected, you'll see MDB TX LED blinking (polling)

### With MDB Device Connected:

1. **Connect MDB Device** (coin changer or bill validator)

2. **Observe LED Activity**:
   - MDB TX LED (Red): Blinks regularly (converter polling device)
   - MDB RX LED (Amber): Should blink in response (device answering)

3. **Terminal Output**:
   - Converter should display detected device information
   - Example for coin changer:
     ```
     CC*ENABLED*
     CC*SETUP*<configuration data>
     ```

4. **Test Device Function**:
   - Insert coin/bill into MDB device
   - Terminal should show credit messages
   - Device activity messages appear in real-time

## Communication Settings

### Serial Port Configuration:

Configure your terminal emulator with these settings:

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Baud Rate** | 9600 | Fixed in firmware |
| **Data Bits** | 8 | Standard |
| **Parity** | None | N |
| **Stop Bits** | 1 | Standard |
| **Flow Control** | None | No hardware/software flow control |
| **Local Echo** | OFF | Recommended for clarity |
| **Line Ending** | CR+LF | Carriage Return + Line Feed |

**Quick Reference**: 9600 8N1

### Recommended Terminal Software:

- **Windows**:
  - PuTTY (free, open-source)
  - Tera Term (free, open-source)
  - Arduino IDE Serial Monitor

- **Linux**:
  - minicom
  - screen
  - GTKTerm
  - Arduino IDE Serial Monitor

- **macOS**:
  - screen (built-in terminal command)
  - CoolTerm
  - Arduino IDE Serial Monitor

### Terminal Software Setup Example (PuTTY):

1. Open PuTTY
2. Select "Serial" connection type
3. Enter COM port (e.g., COM3)
4. Set speed to 9600
5. Go to Connection → Serial:
   - Data bits: 8
   - Stop bits: 1
   - Parity: None
   - Flow control: None
6. Click "Open"

## Troubleshooting

### Problem: No Power Indicators

**Symptoms**: No LEDs light up when power applied

**Possible Causes and Solutions**:
- Check power supply voltage (should be 24-34V DC)
- Verify power supply polarity (reverse polarity damages device)
- Check power connector seating
- Test power supply with multimeter
- Inspect for damaged components or cold solder joints

### Problem: No Boot Message on PC

**Symptoms**: LEDs work but no data in terminal

**Possible Causes and Solutions**:
- Verify RS-232 cable connection
- Check COM port number in terminal software
- Confirm serial settings: 9600 8N1
- Try different USB-to-Serial adapter (if using one)
- Check DB9 connector pins for continuity
- Verify MAX232A chip (IC2) is properly seated and working
- Test with different terminal software

### Problem: MDB TX Blinks but No RX Response

**Symptoms**: Red LED blinks, but amber LED stays off

**Possible Causes and Solutions**:
- MDB device not powered or defective
- MDB cable miswired or damaged
- Check MDB data line connections (Bus+ and Bus-)
- Verify MDB device receives 24-34V power
- Try known-working MDB device
- Check for short circuits on MDB lines
- Verify transistors VT1 and VT2 are functional

### Problem: Erratic Communication

**Symptoms**: Intermittent messages, garbled data

**Possible Causes and Solutions**:
- Loose connections on RS-232 or MDB interfaces
- Electromagnetic interference near cables
- Damaged crystal oscillator (Q1 - 16MHz)
- Incorrect fuse settings on ATmega chip
- Power supply voltage fluctuation (add filter capacitors)
- Try shorter cable lengths
- Add shielded cables for MDB connection

### Problem: Voltage Regulator Overheating

**Symptoms**: VR1 (L78S05CV) very hot to touch

**Possible Causes and Solutions**:
- **CRITICAL**: Install heat sink immediately (device may fail)
- Input voltage too high (check for >34V)
- Short circuit on 5V rail
- Excessive current draw from 5V components
- Defective voltage regulator (replace)

### Problem: Firmware Not Working After Upload

**Symptoms**: Device powered but behaves incorrectly

**Possible Causes and Solutions**:
- Incorrect fuse settings for ATmega chip
- Wrong firmware file for chip type (check ATmega1284 vs 644 vs 2560)
- EEPROM not programmed (flash both .hex and .eep files)
- Crystal frequency mismatch (must use 16MHz external crystal)
- Verify fuse settings:
  - External 16MHz crystal selected
  - Brown-out detection enabled
  - See `Compiled_firmware/atmega644p_16MHzExternal_fuses.png`

### Problem: MDB Device Not Detected

**Symptoms**: Boot message appears but no device information

**Possible Causes and Solutions**:
- MDB device not supported by firmware (check manual)
- Device address conflict (only one device per type)
- MDB cable too long (>3 meters may cause issues)
- MDB device in error state (power cycle device)
- Incompatible MDB protocol version
- Check firmware device support list

## Additional Resources

### Documentation:
- `Docs/MDB_Master_User_Manual_en.pdf` - Complete user manual (English)
- `Docs/MDB_Master_User_Manual_rus.pdf` - Complete user manual (Russian)
- `Docs/MDB_version_4-2.pdf` - MDB Protocol Specification v4.2

### Hardware Reference:
- `PCB/Full Schematics.png` - Complete circuit schematic
- `PCB/parts_list.txt` - Bill of materials
- `Compiled_firmware/Readme.txt` - Firmware programming instructions

### Firmware:
- `Compiled_firmware/` - Pre-compiled firmware for different ATmega variants
- Check firmware version in boot message
- Current version: 1.1.1730

## Command Reference

The converter accepts commands from PC via UART and reports device status. Detailed command syntax available in user manual.

### Common Status Messages:

| Message | Meaning |
|---------|---------|
| `SYS*MDBSTART*` | System boot message |
| `CC*ENABLED*` | Coin changer detected and enabled |
| `BV*ENABLED*` | Bill validator detected and enabled |
| `CC*CREDIT*<amount>` | Coin credit received |
| `BV*CREDIT*<amount>` | Bill credit received |
| `ERROR*` | Error condition (see manual) |

## Support and Further Information

For detailed protocol information, command syntax, and advanced configuration:
- Review complete user manual in `Docs/` folder
- Check MDB protocol specification for technical details
- Refer to source code in `sources/MDB-UART-Master/` for implementation

---

**Revision**: 1.0
**Last Updated**: November 2025
**Compatible Firmware**: v1.1.1730 and later

For firmware source code and modifications, see `sources/MDB-UART-Master/` directory.

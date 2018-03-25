
# Arduino-MDB-UART
Atmega1284 PLC acts as man-in-the-middle between MDB peripheral and master PC (VMC) serial port.
Command received from master PC (VMC) via serial port (RXD1 on AtMega1284), then sent to MDB serial port (TXD0) with 9-bit format.
Answers from peripheral devices receivet via RXD0 converted to string representation of hex bytes sent to Master PC (TXD1).

AGAIN:
Master PC writes command as bytes, including CHK byte (see MDB datasheet included), but receives answers as text with EOL. It's important.

Breadboard demo: https://youtu.be/YV8bc2hhqS0

# History
Once I needed to manage the MDB devices from a PC. Googling brought to Aliexpress, where a lot of adapters are sold at a price of $ 50 or more. Further research showed a complete lack of solutions that can be repeated at home, so I had to do it myself for the sake of economy.
The cost of the finished device will not exceed $ 20. I hope this project will affect the prices as they too expensive (imho).

# Get Started
It will be simple to repeat, all components are available and cheap.
You can use any other Atmega PLC with this code, having 2 hardware UARTs on board. I used AtMEGA1284P-PU as it has DIP (through-hole) design, which is more friendly for assembling and soldering.
Electric principal circuit included see MDB_Master.pdf (Copyright (C) 2001 by BonusData AG).

IMPORTANT: I RECOMMEND TO CHANGE R4 from 10K to 2.2K as it cannot drive MDB levels when there are more than 1 peripheral device on bus.

# Credits
Inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer

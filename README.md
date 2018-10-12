# Arduino-MDB-UART
Atmega1284 PLC acts as man-in-the-middle between MDB peripheral and master PC (VMC) serial port. It off-loads MDB bus polling and converting data between 9-bit MDB and 8-bit RS232 tasks from VMC, effectively delivers data by using hardware interrupts and provides conformity for required timing restrictions (see MDB datasheet for details).
Command received from master PC (VMC) via serial port (RXD1 on AtMega1284), then sent to MDB serial port (TXD0) with 9-bit format.
Answers from peripheral devices received via RXD0 converted to string representation of hex bytes sent to Master PC (TXD1).
First byte of answer sent to VMC is the address of peripheral device so VMC "knows" which device responds.

AGAIN:
Master PC writes command as bytes, including CHK byte (see MDB datasheet included), but receives answers as text with EOL. It's important.
"MDB-RS232-Test" folder contains test software sources (C#).

Breadboard live demo with ICT A7\V7 bill validator and Currenza C2 Blue coin changer (see description below video):

[![Video](http://img.youtube.com/vi/YV8bc2hhqS0/0.jpg)](http://www.youtube.com/watch?v=YV8bc2hhqS0)

# HOW CAN I USE IT?
It's for connecting PC or other computer with "usual" 8-bit COM port ("Master", "VMC") to MDB devices (slaves such as coin charger, bill validator etc...), to control these devices and receive messages from them. It's NOT for emulating coin charger or something like it (for these purposes you'll need another adapter).

# History
Once I needed to manage the MDB devices from a PC. Googling brought to Aliexpress, where a lot of adapters are sold at a price of $ 50 or more like this: http://www.waferstar.com/en/MDB-PC.html or similar.
~~Further research showed a complete lack of solutions that can be repeated at home, so I had to do it myself for the sake of economy.
~~The cost of the finished device will not exceed $ 20. ~~I hope this project will affect the prices as they too expensive (imho)~~I just want to punish these hucksters for irrepressible greed.

# Get Started
It will be simple to repeat, all components are available and cheap.
You can use any other Atmega PLC with this code, having 2 hardware UARTs on board. AtMEGA1284P-PU recommended for beginners as it has DIP (through-hole) design, which is more friendly for assembling and soldering.

IMPORTANT:
1. I RECOMMEND TO CHANGE R4 from 10K to 2.2K as it cannot drive MDB levels when there are more than 1 peripheral device on bus. See "Full Schematics.png" for actual wiring diagram (tested and worked).
2. Don't forget common GND.

# Credits
Inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer

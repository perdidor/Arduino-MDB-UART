# Arduino-MDB-UART
Atmega1284 PLC acts as man-in-the-middle between MDB peripheral and master PC (VMC) serial port. It off-loads MDB bus polling and converting data between 9-bit MDB and 8-bit RS232 tasks from VMC, effectively delivers data by using hardware interrupts and provides conformity for required timing restrictions (see MDB datasheet for details).
Command received from master PC (VMC) via serial port (RXD1 on AtMega1284), then sent to MDB serial port (TXD0) with 9-bit format.
Answers from peripheral devices received via RXD0 converted to string representation of hex bytes sent to Master PC (TXD1).
First byte of answer sent to VMC is the address of peripheral device so VMC "knows" which device responds. Last answer byte followed by EOL. **AGAIN: Master PC writes command as bytes, including CHK byte (see MDB datasheet included), but receives answers as text with EOL. It's important. "MDB-RS232-Test" folder contains test software sources (C#).**

You can use any other Atmega PLC with this code, having 2 hardware UARTs on board. AtMEGA1284P-PU recommended for beginners as it has DIP (through-hole) design, which is more friendly for assembling and soldering.


Breadboard live demo with ICT A7\V7 bill validator and Currenza C2 Blue coin changer (see description below video):

[![Video](http://img.youtube.com/vi/YV8bc2hhqS0/0.jpg)](http://www.youtube.com/watch?v=YV8bc2hhqS0)

# HOW CAN I USE IT?
It's for connecting PC or other computer with "usual" 8-bit COM port ("Master", "VMC") to MDB devices (slaves such as coin charger, bill validator etc...), to control these devices and receive messages from them. It's NOT for emulating coin charger or something like it (for these purposes you'll need another adapter).

# History
Once I needed to manage the MDB devices from a PC. Googling brought to Aliexpress, where a lot of adapters are sold at a price of $ 50 or more like this: http://www.waferstar.com/en/MDB-PC.html or similar.
The cost of the finished device will not exceed $ 20. I just want to punish these hucksters for irrepressible greed.

# Get Started
It will be simple to repeat, all components are available and cheap (see parts_list.txt). "PCB LayOut Designer 6.0" software required to view and edit PCB design file "mdb-rs232.lay6".

**IMPORTANT**:
- LED mounting height 14mm;
- DO NOT use device without radiator.

Repository contains enclosure files for 3D printing. Assembled enclosure (render, more in Enclosure folder):

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/Enclosure/assembly.JPG" width="600">

PCB design included is one-sided FR4 70x80mm (render):

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB_layout4.JPG" width="360">

# Credits
Inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer

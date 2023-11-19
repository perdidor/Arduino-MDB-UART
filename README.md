**Русская версия описания находится ниже**

# UPDATE 11 Oct 2023: source public release

# Arduino-MDB-UART
Atmega1284 PLC (or Arduino Mega, or any other ATmega chip with 2 UARTs) acts as man-in-the-middle between MDB peripheral and master PC (VMC) serial port. It off-loads MDB bus polling and converting data between 9-bit MDB and 8-bit RS232 tasks from VMC, effectively delivers data by using hardware interrupts and provides conformity for required timing restrictions (see MDB datasheet for details).
Command received from master PC (VMC) via serial port (RXD1 on AtMega1284), then sent to MDB serial port (TXD0) with 9-bit format.

Compiled_firmware folder contains ready-to-burn firmware images in hex format, for both "Through-hole ATmega1284" and "new rev2a" versions, see instructions inside.
Docs folder contains manuals for operating device with firmware included.

# HOW CAN I USE IT?
It's for connecting PC or other computer with "usual" 8-bit COM port ("Master", "VMC") to MDB devices (slaves such as coin charger, bill validator etc...), to control these devices and receive messages from them. It's NOT for emulating coin charger or something like it (for these purposes you'll need another adapter). It's NOT for MDB sniffing, schematics will differ too, due MDB lines are optoisolated current loops.
See user manual for actual supported device list.

# Get Started
It will be simple to repeat, all components are available and cheap (see parts_list.txt). "Sprint Layout 6.0" software required to view and edit PCB design file "mdb-rs232.lay6".

**IMPORTANT**:
- LED mounting height 14mm;
- DO NOT use device without radiator.

Repository contains enclosure files for 3D printing (for single-side board adapter only).

# Burning firmware into adapter's MCU:

This section was added after I sent out dozens of similar responses to requests for assistance via email. It looks like a lot of new people have come to hardware development due to the fucking covid pandemic. OK here's a quick guide on how to start the device after assembly.

So, we need to do three operations: flash the hex file of the main program, flash the eep file with the adapter settings, and set the correct fuses values. The firmware is compiled with the expectation that the chip clocked from an external crystal at 16 MHz, so if the fuses values are incorrect, the chip will not be able to work at best due to incorrect timing, at worst the chip will be impossible to flash using any serial method for the same reason.

**Prerequisities**:

For the firmware upload, you will need:
- avrdudeprog 3.3 software - https://yourdevice.net/downloads/avrdudeprog33.rar

- UsbASP programmer. The market is full of buggy fakes, order 2-3 at once, they are quite cheap*.

*imho, only 2 will be enough, when you're not as extremely dumb as dude on picture below.

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/Docs/USBasp.PNG" width="600">

For the programmer to work, you need a driver (not for all OS), you can download it on the device developer's website https://www.fischl.de/usbasp/

1. Connect the adapter and the programmer with 6 wires. Look for the pinout of the ICSP10 connectors in Google, for the adapter it is located in the component designation file (for the double-sided version) or on the board.

2. Set the correct fuse values, for example, for Atmega644P they will be as follows:

<img src = "https://github.com/perdidor/Arduino-MDB-UART/blob/master/Compiled_firmware/atmega644p_16MHzExternal_fuses.png">

Find the required values for atmega1284 yourself in the datasheet or numerous calculators on the Internet.

***You can also set fuse values using the Arduino IDE: select the desired chip, frequency and clock source, programmer and / or port, and click "Burn Bootloader" in the same menu.***

3. Select the desired hex firmware file and eep settings file, and flash them in the same order (Programm button).

**Check the result**:

As a result, after a successful firmware upload and the absence of assembly and manufacturing defects, you will have a working device. It will look like this: in the absence of a connected peripheral device (coin acceptor, bill acceptor), the MDB Tx indicator blinks, when power is applied, and the VMC UART in 9600-8-N-1 mode outputs the firmware version and that's it. When connecting supported devices (see the manual), the MDB Rx indicator should blink - this is the answers from them to the adapter, and information about the connected devices will appear in the VMC UART output.

If the behavior differs from the one described - either it is incorrectly assembled or it is incorrectly programmed, nobody's advice will help here, read again and do everything carefully, checking each step.

Live demo with ICT A7\V7 bill validator and Currenza C2 Blue coin changer:

[![Video](http://img.youtube.com/vi/Z58a7f0YK28/0.jpg)](http://www.youtube.com/watch?v=Z58a7f0YK28)

Assembled enclosure (render, more in Enclosure folder)
Собранный корпус (рендер, остальные в том же каталоге)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/Enclosure/assembly.JPG" width="600">

PCB design included is one-sided FR4 70x80mm (render)
Внешний вид печатной платы 70х80мм (рендер)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB/PCB_oneside_updated.jpg" width="360">


PCB rev2a complete package included (render, double-side, RTM)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB/PCB_rev2a.JPG" width="400">

rev2a finished device photo:

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB/device.png" width="400">

# Credits
Inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer


**========================End of English version==========================**

# Конвертер MDB-UART на базе Arduino
Микроконтроллер Atmega1284 или любой другой (например плата Arduino Mega на основе ATmega2560), имеющий два аппаратных UART на борту, может использоваться как конвертер между обычным компьютером или другим устройством с обычным RS-232 портом, и периферийными устройствами, работающими по протоколу MDB. Он разгружает управляющий компьютер от задач, связанных с опросом шины MDB и конвертированием между 9 и 8 битным форматом, обеспечивает соответствие временным ограничениям протокола и эффективно использует аппаратные прерывания.
Обмен данными с управляющих компьютером осуществляется через UART1, с шиной MDB через UART0.

Каталог Compiled_firmware содержит готовые к прошивке образы в офрмате Intel hex, для обеих аппаратных версий адаптера (на базе ATmega1284 и ATmega644PA).
Каталог Docs содержит инструкции по работе с прошитым устройством.

# Зачем нужен этот девайс?
Чтобы управлять устройствами приема наличных и исполнительными устройствами, входящими в состав киосков самообслуживания (торговых автоматов), по протоколу MDB. Девайс НЕ ПРЕДНАЗНАЧЕН для эмуляции вышеперечисленных устройств, для этого нужен другой адаптер.
См. инструкцию для уточнения актуального списка поддерживаемых устройств.

# Приступаем
Это не очень трудно, компоненты дешевы и доступны (список компонентов в файле parts_list.txt, магазин запчастей с доставкой http://www.chipdip.ru). Обычно получается дешевле 20 долларов (без корпуса).
Файл дизайна печатной платы "mdb-rs232.lay6" для просмотра требует ПО "Sprint Layout 6.0"

**ВАЖНО**:
- Высота установки светодиодов 14мм;
- Без радиатора на регуляторе напряжения использовать не рекомендуется.

Для двухстороннего варианта платы (rev2a) есть архив с полным комплектом технологической документации, для заказа изготовления в стороннем сервисе.

Репозиторий содержит файлы для заказа или самостоятельной печати корпуса устройства (для варианта с односторонней платой) на 3D принтере (каталог "Enclosure").

Рендеры корпуса и печатных плат в английской версии выше.

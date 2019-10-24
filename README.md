**Русская версия описания находится ниже**

# Arduino-MDB-UART
Atmega1284 PLC acts as man-in-the-middle between MDB peripheral and master PC (VMC) serial port. It off-loads MDB bus polling and converting data between 9-bit MDB and 8-bit RS232 tasks from VMC, effectively delivers data by using hardware interrupts and provides conformity for required timing restrictions (see MDB datasheet for details).
Command received from master PC (VMC) via serial port (RXD1 on AtMega1284), then sent to MDB serial port (TXD0) with 9-bit format.

Compiled_firmware folder contains ready-to-burn firmware images in hex format, for both "Through-hole ATmega1284" and "new rev2a" versions, see instructions inside.
Docs folder contains manuals for operating device with firmware included.

# HOW CAN I USE IT?
It's for connecting PC or other computer with "usual" 8-bit COM port ("Master", "VMC") to MDB devices (slaves such as coin charger, bill validator etc...), to control these devices and receive messages from them. It's NOT for emulating coin charger or something like it (for these purposes you'll need another adapter).
See user manual for actual supported device list.

# Get Started
It will be simple to repeat, all components are available and cheap (see parts_list.txt). "Sprint Layout 6.0" software required to view and edit PCB design file "mdb-rs232.lay6".

**IMPORTANT**:
- LED mounting height 14mm;
- DO NOT use device without radiator.

Repository contains enclosure files for 3D printing (for single-side board adapter only).

Assembled enclosure (render, more in Enclosure folder)
Собранный корпус (рендер, остальные в том же каталоге)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/Enclosure/assembly.JPG" width="600">

PCB design included is one-sided FR4 70x80mm (render)
Внешний вид печатной платы 70х80мм (рендер)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB/PCB_oneside_updated.jpg" width="360">


PCB rev2a complete package included (double-side, RTM)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB/PCB_rev2a.JPG" width="400">

# Credits
Inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer


**========================End of English version==========================**

# Конвертер MDB-UART на базе Arduino
Микроконтроллер Atmega1284 или любой другой, имеющий два аппаратных UART на борту, может использоваться как конвертер между обычным компьютером или другим устройством с обычным RS-232 портом, и периферийными устройствами, работающими по протоколу MDB. Он разгружает управляющий компьютер от задач, связанных с опросом шины MDB и конвертированием между 9 и 8 битным форматом, обеспечивает соответствие временным ограничениям протокола и эффективно использует аппаратные прерывания.
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

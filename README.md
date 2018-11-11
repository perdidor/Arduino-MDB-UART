**Русская версия описания находится ниже**

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
It will be simple to repeat, all components are available and cheap (see parts_list.txt). "Sprint Layout 6.0" software required to view and edit PCB design file "mdb-rs232.lay6".

**IMPORTANT**:
- LED mounting height 14mm;
- DO NOT use device without radiator.

Repository contains enclosure files for 3D printing.

Assembled enclosure (render, more in Enclosure folder)
Собранный корпус (рендер, остальные в том же каталоге)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/Enclosure/assembly.JPG" width="600">

PCB design included is one-sided FR4 70x80mm (render)
Внешний вид печатной платы 70х80мм (рендер)

<img src="https://github.com/perdidor/Arduino-MDB-UART/blob/master/PCB_layout4.JPG" width="360">

# Credits
Inspired by MDB-Sniffer project https://github.com/MarginallyClever/MDB-Sniffer


**========================End of English version==========================**

# Конвертер MDB-UART на базе Arduino
Микроконтроллер Atmega1284 или любой другой, имеющий два аппаратных UART на борту, может использоваться как конвертер между обычным компьютером или другим устройством с обычным RS-232 портом, и периферийными устройствами, работающими по протоколу MDB. Он разгружает управляющий компьютер от задач, связанных с опросом шины MDB и конвертированием между 9 и 8 битным форматом, обеспечивает соответствие временным ограничениям протокола и эффективно использует аппаратные прерывания.
Обмен данными с управляющих компьютером осуществляется через UART1, с шиной MDB через UART0.
Команды от управляющего компьютера приходят в виде набора байт. Ответы от MDB устройств конвертируются в строковое представление HEX байт, разделитель - пробел, в конце ответа EOL. Первый HEX байт ответа равен адресу устройства, это единственное изменение, которое вносит адаптер в поток данных, это необходимо для определения отвечающего устройства. Задача интерпретации полученных данных должна выполняться на управляющем компьютере (см. даташит MDB 4.2). В каталоге "MDB-RS232-Test" лежат исходники на c# примера программы для работы с устройством.

Демонстрационное видео работы прототипа на макетной плате, управляем купюроприемником ICT A7\V7 и монетоприемником Currenza C2 Blue (см. описание под видео):

[![Video](http://img.youtube.com/vi/YV8bc2hhqS0/0.jpg)](http://www.youtube.com/watch?v=YV8bc2hhqS0)

# Зачем нужен этот девайс?
Чтобы управлять устройствами приема наличных и исполнительными устройствами, входящими в состав киосков самообслуживания (торговых автоматов), по протоколу MDB.
Девайс НЕ ПРЕДНАЗНАЧЕН для эмуляции вышеперечисленных устройств, для этого нужен другой адаптер.

# История создания
При проектировании торгового автомата понадобилось управлять купюроприемником. На Алиэкспресс цены от 50 баксов и выше (например, есть и такие варианты по 100 баксов https://shop.mst-company.ru/pos-ustroystva/vending/konverter-mdb2pc-s-kabeleml), денег стало жалко отдавать барыгам и пришлось сделать свое устройство.

# Приступаем
Это не очень трудно, компоненты дешевы и доступны (список компонентов в файле parts_list.txt, магазин запчастей с доставкой http://www.chipdip.ru). Обычно получается дешевле 20 долларов (без корпуса).
Файл дизайна печатной платы "mdb-rs232.lay6" для просмотра требует ПО "Sprint Layout 6.0"

**ВАЖНО**:
- Высота установки светодиодов 14мм;
- Без радиатора на регуляторе напряжения использовать не рекомендуется.

Репозиторий содержит файлы для заказа или самостоятельной печати корпуса устройства на 3D принтере (каталог "Enclosure"). 

Рендеры корпуса и печатной платы в английской версии выше.

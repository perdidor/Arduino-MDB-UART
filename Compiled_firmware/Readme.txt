Use AVRDUDEPROG 3.3 software to burn compiled hex files to chip (google for it). Or, you can use avrdude.exe from Arduino IDE and build command line manually.
File name directs to chip and frequency. "hex" is the main logic program, "eep" is EEPROM initial content.

Для прошивки используйте программу AVRDUDEPROG 3.3 или avrdude.exe идущий в комплекте с Arduino IDE, в этом случае придется вручную указать аргументы командной строки.
Имя файла содержит информацию о чипе и частоте, для которой предназначена прошивка. hex-файл - главная программа, eep - начальное содержимое энергонезависимой памяти.
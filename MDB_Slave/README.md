**Русская версия описания находится ниже**

# Problem
The MDB standard provides for the presence of up to 2 cashless payment devices (terminals) simultaneously connected to the bus in the vending machine. There is a tradition of launching emulator devices on the market, which, from the VMC's point of view, are MDB Cashless Device type devices, but in fact they are not: they are not tied to banking and international payment systems and, accordingly, there is no information exchange with these systems. The decision on whether the user has made a payment or not is made by an external control signal, which, in turn, can be formed in any way: checking the result of a Bitcoin transaction, the user has presented an NFC card / contact card of discounts, etc. up to the whistling of a secret melody into a microphone installed in a vending machine. The cost of such devices is in the range of $150-400 and depends on the availability of additional functions: sniffing, VMC emulation, etc.<br/>
Advantages of using MDB Cashless device:
1. Ability to accept the purchase amount in one payment
2. No need to dispense change (no bill / coin dispensers needed)
Disadvantages:
1. The exchange of data between VMC and MDB Cashless Device, described in protocol version 4.2, is much more difficult than with any other payment acceptance device, due to the need to interact with external systems to make a decision on whether a payment is credited / rejected. This circumstance leads to the complication of development and the rise in the cost of the final device.
2. Not all vending machines have MDB Cashless support at the VMC level; upgrading such vending machines using such an emulator is impossible without reworking the control device. As a rule, these vending machines work only with cash and support MDB bill and coin acceptors.

Let's finally formulate the problem:<br/>
**It is necessary to organize the possibility of accepting custom payment instruments in vending machines that do not support MDB Cashless, but have support for MDB cash acceptors (bills and/or coins).**

# Theory
The MDB protocol standard stipulates that all settings of the MDB standard bill and coin acceptors are stored inside themselves. For the situation under consideration, the following parameters are important (for a bill acceptor):
- currency / country code
- the number of digits in monetary amounts after the decimal point;
- scaling factor of denominations of banknotes;
- denominations of supported bills (taking into account the scaling factor)

All these data are transferred to the control device of the vending machine when its operation is initialized, the real denominations of the bills are determined by the VMC independently in this way (a fragment of the real VMC firmware):<br/><br/>
**double billvalue = BillScalingFactor * (BillTypeCredit / pow (10, DecimalPlaces));<br/><br/>**
Theoretically, you can set these values in this way and emulate a bill acceptor (coin acceptor) so that from the point of view of VMC it accepts bills (coins) of any given denomination (not only officially issued), for example, coin with value $37.39 or bill with credit value $20.75, etc. Usually such devices support up to 16 denominations, but if we have more products and we are able to determine the required purchase amount, we can reconfigure the emulator on the fly and inform VMC about rebooting the cash acceptor, after which VMC, according to the MDB standard, will re-request information on configuration and settings, and will receive already new denominations of accepted coins / notes.<br/>
Thus, in fact, a bill acceptor / coin acceptor will be located on the MDB bus, informing the control device about the transfer of an arbitrary amount by an external control signal (for example, a request via the Internet) or another (arbitrary) algorithm.<br/>
Benefits:
1. Multicurrency
2. The ability to reconfigure "on the fly" for any currency and denomination
3. There is no need to give change, the user pays exactly the amount that is needed
4. The algorithm of the emulator is an order of magnitude less complicated than the algorithm of the MDB Cashless Device
5. The device can perform functions of sniffing / processing all MDB messages and events

Disadvantages:
1. It is necessary to abandon one of the cash acceptors (the one that will be emulated), because the MDB standard supports only one bill acceptor and one coin acceptor at a time.


Current development status: in progress (as per publish date).

*end of English version*

# Проблема.
Стандарт MDB предусматривает наличие в торговом автомате до 2 одновременно подключенных к шине устройств (терминалов) бесконтактной оплаты. Сложилась традиция выпуска на рынок устройств-эмуляторов, которые с точки зрения VMC являются устройствами типа MDB Cashless Device, но по сути таковыми не являются: в них отсутствует привязка к банковским и международным платежным системам и, соответственно нет обмена информацией с этими системами. Решение о том, осуществлен платеж пользователем или нет, принимается по внешнему управляющему сигналу, который, в свою очередь может формироваться любым способом: проверка результата Bitcoin транзакции, пользователь предъявил NFC-карту/контактную карту скидок, и т.д. вплоть до насвистывания секретной мелодии в микрофон, установленный в торговом автомате. Стоимость подобных устройств находится в пределах 150-400 долларов и зависит от наличия дополнительных функций: сниффинг, эмуляция VMC и т.д.<br/>
Достоинства использования устройства MDB Cashless:
1.	Возможность приема суммы покупки одним платежом
2.	Нет необходимости выдавать сдачу (не нужны устройства выдачи купюр/монет)
Недостатки:
1.	Обмен данными между VMC и MDB Cashless Device, описанный в протоколе версии 4.2, значительно сложнее, чем с любым другим устройством приема платежей, из-за необходимости взаимодействия с внешними системами для принятия решения о зачислении/отклонении платежа. Данное обстоятельство ведет к усложнению разработки и удорожанию конечного устройства.
2.	Не все торговые автоматы имеют поддержку MDB Cashless на уровне VMC, модернизация таких автоматов с помощью подобного эмулятора невозможна без переделки управляющего устройства. Как правило, такие торговые автоматы работают лишь с наличными денежными средствами и поддерживают купюро- и монетоприемники стандарта MDB.

Окончательно сформулируем проблему:<br/>
**Необходимо организовать возможность приема custom платежных средств в торговых автоматах, которые не поддерживают MDB Cashless, но имеют поддержку MDB устройств приема наличных.**

# Теория.
Стандарт протокола MDB предусматривает, что все настройки купюро- и монетоприемников стандарта MDB хранятся в них самих. Для рассматриваемой ситуации имеют значения следующие параметры (для купюроприемника):
- код валюты/страны
- количество знаков в денежных суммах после десятичной запятой;
- коэффициент масштабирования номиналов купюр;
- номиналы поддерживаемых купюр (с учетом коэффициента масштабирования)

Все эти данные передаются в управляющее устройство торгового автомата при инициализации его работы, реальные номиналы купюр определяются VMC самостоятельно таким образом (фрагмент реальной прошивки VMC):<br/><br/>
**double billvalue = BillScalingFactor * (BillTypeCredit/ pow(10, DecimalPlaces));<br/><br/>**
Теоретически, можно таким образом задать эти значения и эмулировать купюроприемник (монетоприемник), чтобы с точки зрения VMC он принимал купюры (монеты) любого заданного номинала (не только официально выпускаемых), например, номиналом $37.39, $20.75 и т.д. Обычно подобные устройства поддерживают до 16 номиналов, но если у нас больше товаров и мы имеем возможность определить необходимую сумму покупки, мы можем переконфигурировать эмулятор «на лету» и сообщить VMC о перезагрузке устройства приема наличных, после этого VMC, согласно стандарту MDB, заново запросит информацию о конфигурации и настройках, и получит уже новые номиналы принимаемых монет/купюр.<br/>
Таким образом, фактически на шине MDB будет находится купюроприемник/монетоприемник, сообщающий управляющему устройству о зачислении произвольной суммы по внешнему управляющему сигналу (например, запрос через Интернет) или другому (произвольному) алгоритму.<br/>
Преимущества:
1.	Мультивалютность
2.	Возможность переконфигурирования «на лету» на любую валюту и номинал
3.	Не нужно выдавать сдачу, пользователь платит ровно ту сумму, какую надо
4.	Алгоритм работы эмулятора на порядок менее сложный, чем алгоритм работы MDB Cashless Device
5.	Устройство может выполнять функции сниффинга/обработки всех сообщений и событий MDB

Недостатки:
1.	Необходимо отказаться от одного из устройств приема наличных (того, который будет эмулироваться), потому что стандарт MDB поддерживает только один купюроприемник и один монетоприемник одновременно.

Текущий статус разработки: в работе.

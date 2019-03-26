using System;
using System.Diagnostics;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using System.Threading;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using System.Text;

namespace MDBLib
{
    /// <summary>
    /// Класс для работы с шиной MDB через адаптер
    /// </summary>
    public class MDB
    {
        public delegate void MDBAdapterStartedDelegate();
        public delegate void MDBCoinsDispensedManuallyDelegate(double CoinValue, int CoinsDispensed, int CoinsLeft);
        public delegate void MDBChangerPayOutBusydDelegate();
        public delegate void MDBBAResetedDelegate();
        public delegate void MDBCCResetedDelegate();
        public delegate void MDBInsertedBillDelegate(double BillValue);
        public delegate void MDBInsertedCoinRoutedToCashBoxDelegate(double CoinValue);
        public delegate void MDBInsertedCoinRoutedToCCTubeDelegate(double CoinValue);
        public delegate void MDBErrorDelegate(string ErrorMessage);
        public delegate void MDBDebugDelegate(string DebugMessage);
        public delegate void MDBDataProcessingErrorDelegate(string DataProcessingErrorMessage);
        public delegate void MDBChangeDispensedDelegate(List<CoinsRecord> DispensedCoinsData);
        public delegate void MDBCCTubesStatusDelegate(List<CoinChangerTubeRecord> CoinChangerTubeStatusData);
        public delegate void MDBBAStackerStatusDelegate(bool StackerFull, int StackerBillsCount);
        public delegate void MDBInformationMessageReceivedDelegate(string MDBInformationMessage);

        /// <summary>
        /// структура записи о количестве монет
        /// </summary>
        public class CoinsRecord
        {
            public double CoinValue = 0.00;
            public int CoinsDispensed = 0;
        }
        /// <summary>
        /// структура записи о статусе трубки монетоприемника
        /// </summary>
        public class CoinChangerTubeRecord
        {
            public double CoinValue = 0.00;
            public int CoinsCount = 0;
            public bool IsFull = false;
        }
        /// <summary>
        /// Последовательный порт адаптера
        /// </summary>
        public static SerialDevice MDBSerialPort = null;
        /// <summary>
        /// Список исходящих команд
        /// </summary>
        private static List<byte[]> CommandList = new List<byte[]> { };
        /// <summary>
        /// флаг перехода купюроприемника в нерабочий
        /// </summary>
        private static bool BAAlreadyDisabled = false;
        /// <summary>
        /// Флаг ожидания сатуса купюроприемника
        /// </summary>
        private static bool CheckBAStatus = false;
        /// <summary>
        /// Флаг ожидания статуса трубок монетоприемника
        /// </summary>
        private static bool CheckCCTubeStatus = false;
        /// <summary>
        /// Флаг ожидания настроек монетоприемника
        /// </summary>
        private static bool GetCCSettings = false;
        /// <summary>
        /// Флаг ожидания настроек купюроприемника
        /// </summary>
        private static bool GetBASettings = false;
        /// <summary>
        /// Флаг отладочного режима
        /// </summary>
        public static bool DebugEnabled = false;
        /// <summary>
        /// Адрес устройства, от которого ожидается ответ
        /// </summary>
        private static byte AwaitingMDBAnswerFrom = 0x00;
        /// <summary>
        /// DataReader
        /// </summary>
        private static DataReader MDBSerialDataReaderObject = null;
        /// <summary>
        /// Флаг выдачи сдачи монетами в прогресе
        /// </summary>
        public static bool DispenseInProgress = false;
        /// <summary>
        /// Флаг ожидания данных о выданной сдаче
        /// </summary>
        public static bool CheckDispenseResult = false;
        /// <summary>
        /// Таймаут ожидания выдачи сдачи (обновляется при каждом получении информации о выдаче)
        /// </summary>
        public static DateTime DispenseTimeout = new DateTime();

        /// <summary>
        /// Адаптер MDB-RS232 стартанул
        /// </summary>
        public static event MDBAdapterStartedDelegate MDBAdapterStarted;
        /// <summary>
        /// В режиме отладки отсылаем сырые данные, которые пришли с адаптера
        /// </summary>
        public static event MDBDebugDelegate MDBDebug;
        /// <summary>
        /// Монеты выданы вручную
        /// </summary>
        public static event MDBCoinsDispensedManuallyDelegate MDBCoinsDispensedManually;
        /// <summary>
        /// Купюра вставлена
        /// </summary>
        public static event MDBInsertedBillDelegate MDBInsertedBill;
        /// <summary>
        /// Ошибка при работе с шиной MDB
        /// </summary>
        public static event MDBErrorDelegate MDBError;
        /// <summary>
        /// Ошибка при обработке данных
        /// </summary>
        public static event MDBDataProcessingErrorDelegate MDBDataProcessingError;
        /// <summary>
        /// Монета упала в кешбокс
        /// </summary>
        public static event MDBInsertedCoinRoutedToCashBoxDelegate MDBInsertedCoinRoutedToCashBox;
        /// <summary>
        /// Монета упала в трубку
        /// </summary>
        public static event MDBInsertedCoinRoutedToCCTubeDelegate MDBInsertedCoinRoutedToCCTube;
        /// <summary>
        /// Выдана сдача
        /// </summary>
        public static event MDBChangeDispensedDelegate MDBChangeDispensed;
        /// <summary>
        /// Статус заполнения монетоприемника
        /// </summary>
        public static event MDBCCTubesStatusDelegate MDBCCTubesStatus;
        /// <summary>
        /// Статус заполнения стекера купюроприемника
        /// </summary>
        public static event MDBBAStackerStatusDelegate MDBBAStackerStatus;
        /// <summary>
        /// Информационное сообщение
        /// </summary>
        public static event MDBInformationMessageReceivedDelegate MDBInformationMessageReceived;
        /// <summary>
        /// Купюроприемник выполнил команду сброс
        /// </summary>
        public static event MDBBAResetedDelegate MDBBAReseted;
        /// <summary>
        /// Монетоприемник выполнил команду сброс
        /// </summary>
        public static event MDBCCResetedDelegate MDBCCReseted;
        /// <summary>
        /// Монетоприемник занят вдачей сдачи
        /// </summary>
        public static event MDBChangerPayOutBusydDelegate MDBCCPayOutBusy;
        /// <summary>
        /// флаг доступа к исходящим командам
        /// </summary>
        private static SemaphoreSlim MDBCommandsListSemaphore = new SemaphoreSlim(1);
        /// <summary>
        /// флаг доступа к адресу устройства, от которого ожидается ответ
        /// </summary>
        private static SemaphoreSlim MDBAbswerFromByteSemaphore = new SemaphoreSlim(1);
        /// <summary>
        /// флаг доступа к обработчику ответов от устройств
        /// </summary>
        private static SemaphoreSlim MDBDataProcessSemaphore = new SemaphoreSlim(1);

        /// <summary>
        /// структура настроек монетоприемника
        /// </summary>
        public static class CoinChangerSetupData
        {
            public static int ChangerFeatureLevel = 0;
            public static int CountryOrCurrencyCode = 0;
            public static int CoinScalingFactor = 1;
            public static int DecimalPlaces = 0;
            public static bool[] CoinsRouteable = new bool[16];
            public static int[] CoinTypeCredit = new int[16];
        }

        /// <summary>
        /// структура настроек купюроприемника
        /// </summary>
        public static class BillValidatorSetupData
        {
            public static int BillValidatorFeatureLevel = 0;
            public static int CountryOrCurrencyCode = 0;
            public static int BillScalingFactor = 1;
            public static int DecimalPlaces = 0;
            public static int StackerCapacity = 0;
            public static bool[] BillSecurityLevel = new bool[16];
            public static bool Escrow = false;
            public static int[] BillTypeCredit = new int[16];
        }

        /// <summary>
        /// Инициализирует последовательный порт для работы с адаптером (Исключая указанный порт)
        /// </summary>
        /// <param name="ExcludedSerialPortsList">Исключить конкретный порт из поиска</param>
        public static async void InitWithSerialPortExclude(List<string> ExcludedSerialPortsList, CancellationToken token)
        {
            try
            {
                MDBCommandsListSemaphore.Release();
                MDBAbswerFromByteSemaphore.Release();
                MDBDataProcessSemaphore.Release();
                if (DebugEnabled) MDBDebug?.Invoke("Поиск последовательного порта шины MDB..."); else MDBInformationMessageReceived?.Invoke("Поиск последовательного порта шины MDB...");
                string aqs = SerialDevice.GetDeviceSelector();
                var dis = await DeviceInformation.FindAllAsync(aqs);
                foreach (var item in dis)
                {
                    if (!ExcludedSerialPortsList.Contains(item.Id))
                    {
                        MDBSerialPort = await SerialDevice.FromIdAsync(item.Id);
                    }
                }
                MDBSerialPort.WriteTimeout = TimeSpan.FromMilliseconds(1000);
                MDBSerialPort.ReadTimeout = TimeSpan.FromMilliseconds(1000);
                MDBSerialPort.BaudRate = 9600;
                MDBSerialPort.Parity = SerialParity.None;
                MDBSerialPort.StopBits = SerialStopBitCount.One;
                MDBSerialPort.DataBits = 8;
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("Порт шины MDB успешно настроен: {0}-{1}-{2}-{3}", MDBSerialPort.BaudRate, MDBSerialPort.DataBits, MDBSerialPort.Parity.ToString(), MDBSerialPort.StopBits)); else
                    MDBInformationMessageReceived?.Invoke(string.Format("Порт шины MDB успешно настроен: {0}-{1}-{2}-{3}", MDBSerialPort.BaudRate, MDBSerialPort.DataBits, MDBSerialPort.Parity.ToString(), MDBSerialPort.StopBits));
                ListenMDBSerialPort(token);
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
            }
        }

        /// <summary>
        /// Инициализирует указанный последовательный порт для работы с адаптером
        /// </summary>
        /// <param name="SerialPort">последовательный порт</param>
        public static async void InitWithSerialPortExact(string SerialPort, CancellationToken token)
        {
            try
            {
                MDBCommandsListSemaphore.Release();
                MDBAbswerFromByteSemaphore.Release();
                MDBDataProcessSemaphore.Release();
                if (DebugEnabled) MDBDebug?.Invoke("Поиск последовательного порта шины MDB..."); else MDBInformationMessageReceived?.Invoke("Поиск последовательного порта шины MDB...");
                string aqs = SerialDevice.GetDeviceSelector();
                var dis = await DeviceInformation.FindAllAsync(aqs);
                foreach (var item in dis)
                {
                    if (SerialPort == item.Id)
                    {
                        MDBSerialPort = await SerialDevice.FromIdAsync(item.Id);
                    }
                }
                MDBSerialPort.WriteTimeout = TimeSpan.FromMilliseconds(1000);
                MDBSerialPort.ReadTimeout = TimeSpan.FromMilliseconds(1000);
                MDBSerialPort.BaudRate = 9600;
                MDBSerialPort.Parity = SerialParity.None;
                MDBSerialPort.StopBits = SerialStopBitCount.One;
                MDBSerialPort.DataBits = 8;
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("Порт шины MDB успешно настроен: {0}-{1}-{2}-{3}", MDBSerialPort.BaudRate, MDBSerialPort.DataBits, MDBSerialPort.Parity.ToString(), MDBSerialPort.StopBits)); else
                    MDBInformationMessageReceived?.Invoke(string.Format("Порт шины MDB успешно настроен: {0}-{1}-{2}-{3}", MDBSerialPort.BaudRate, MDBSerialPort.DataBits, MDBSerialPort.Parity.ToString(), MDBSerialPort.StopBits));
                ListenMDBSerialPort(token);
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
            }
        }

        /// <summary>
        /// Считывает данные с ппоследовательного порта шины MDB
        /// </summary>
        private static async void ListenMDBSerialPort(CancellationToken token)
        {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
            Task.Run(DispensedCoinsInfoTask, token);
            Task.Run(SendCommandTask, token);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
            while (true)
            {
                try
                {
                    if (DebugEnabled) MDBDebug?.Invoke("Поиск последовательного порта шины MDB..."); else MDBInformationMessageReceived?.Invoke("Порт MDB открыт, ожидание данных...");
                    MDBSerialDataReaderObject = new DataReader(MDBSerialPort.InputStream)
                    {
                        InputStreamOptions = InputStreamOptions.Partial
                    };
                    List<byte> tmpres = new List<byte> { };
                    while (true)
                    {
                        Task<uint> loadAsyncTask = MDBSerialDataReaderObject.LoadAsync(64).AsTask(token);
                        uint bytesRead = 0;
                        bytesRead = await loadAsyncTask;
                        if (bytesRead > 0)
                        {
                            byte[] tmpbyte = new byte[bytesRead];
                            MDBSerialDataReaderObject.ReadBytes(tmpbyte);
                            for (int i = 0; i < tmpbyte.Length; i++)
                            {
                                if ((tmpbyte[i] == '\n'))
                                {
                                    byte[] b = tmpres.ToArray();
#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                                    Task.Run(() =>
                                    {
                                        MDBDataProcessSemaphore.Wait();
                                        string tmpstr = Encoding.UTF8.GetString(b).Trim();
                                        tmpstr = tmpstr.Replace("\r", "");
                                        MDBIncomingDataProcess(tmpstr);
                                        MDBDataProcessSemaphore.Release();
                                    });
#pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                                    tmpres = new List<byte> { };
                                }
                                else
                                {
                                    tmpres.Add(tmpbyte[i]);
                                }
                            }
                        }
                    }
                }
                catch (TaskCanceledException)
                {
                    CloseMDBSerialDevice();
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
                }
                finally
                {
                    if (MDBSerialDataReaderObject != null)
                    {
                        MDBSerialDataReaderObject.DetachStream();
                        MDBSerialDataReaderObject = null;
                    }
                }
            }
        }

        /// <summary>
        /// Закрывает последовательный порт шины MDB
        /// </summary>
        private static void CloseMDBSerialDevice()
        {
            if (MDBSerialPort != null)
            {
                MDBSerialPort.Dispose();
            }
            MDBSerialPort = null;
        }


        /// <summary>
        /// Запускается в начале выдачи сдачи монетами (при первом сообщении "Changer Payout Busy").
        /// </summary>
        /// <returns></returns>
        private static void CoinChanger_DispenseWatch()
        {
            while (DispenseTimeout > DateTime.Now)
            {
                Task.Delay(1000).Wait();
            }
            DispenseInProgress = false;
            CheckDispenseResult = true;
        }

        /// <summary>
        /// отслеживает выдачу сдачи
        /// </summary>
        public static Task DispensedCoinsInfoTask()
        {
            Task.Delay(1000).Wait();
            if (DebugEnabled) MDBDebug?.Invoke("Старт отслеживания выданной сдачи..."); else MDBInformationMessageReceived?.Invoke("Старт отслеживания выданной сдачи...");
            while (true)
            {
                try
                {
                    if (CheckDispenseResult)
                    {
                        GetDispensedInfo();
                        if (DebugEnabled) MDBDebug?.Invoke("Ожидаем данные о выданной сдаче..."); else MDBInformationMessageReceived?.Invoke("Ожидаем данные о выданной сдаче...");
                        int RetryCount = 0;
                        while (CheckDispenseResult && RetryCount < 51)//ждать ответа от монетоприемника будем 5 секунд, потом забъем хуй и продолжим работать
                        {
                            Task.Delay(100).Wait();//пауза на 0.1сек
                            RetryCount++;
                        }
                        CheckDispenseResult = false;
                    }
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
                }
                Task.Delay(1000).Wait();
            }
        }

        /// <summary>
        /// MDB answer processing method.
        /// Must fire all MDB events, all events must be handled externally as well.
        /// </summary>
        /// <param name="MDBStringData">String representation if incoming MDB serial data</param>
        /// <returns>none</returns>
        public static void MDBIncomingDataProcess(string MDBStringData)
        {
            string tmpmdbmsg = "";
            if (MDBStringData.Length >= 120)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}", "Слишком большой пакет (>40 байт)")); else MDBInformationMessageReceived?.Invoke("Слишком большой пакет (>40 байт)");
                return;
            }
            byte[] ResponseData = new byte[] { };
            try
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG READ: {0}", MDBStringData));
                if (MDBStringData == "MDB-UART PLC ready") //MDB-UART adapter PLC started
                {
                    return;
                }
                MDBStringData = MDBStringData.Replace(" ", "");
                ResponseData = Enumerable.Range(0, MDBStringData.Length)
                     .Where(x => x % 2 == 0)
                     .Select(x => Convert.ToByte(MDBStringData.Substring(x, 2), 16))
                     .ToArray();
                if (ResponseData.Length == 2)//просто ACK от устройства, обрабатывать не надо
                {
                    return;
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke("Ошибка при обработке данных MDB");
                return;
            }
            MDBAbswerFromByteSemaphore.Wait();
            if (ResponseData[0] == AwaitingMDBAnswerFrom)
            {
                AwaitingMDBAnswerFrom = 0x00;
            }
            MDBAbswerFromByteSemaphore.Release();
            if (ResponseData[0] == 0x30)//данные от купюроприемника
            {
                try
                {
                    if ((GetBASettings) && (ResponseData.Length == 29))//Ожидаем настройки, размер данных совпадает
                    {
                        GetBASettings = false;
                        BillValidatorSetupData.BillValidatorFeatureLevel = ResponseData[1];
                        BillValidatorSetupData.CountryOrCurrencyCode = BCDByteToInt(new byte[2] { ResponseData[2], ResponseData[3] });
                        BillValidatorSetupData.BillScalingFactor = ResponseData[4] << 8 | ResponseData[5];
                        BillValidatorSetupData.DecimalPlaces = ResponseData[6];
                        BillValidatorSetupData.StackerCapacity = ResponseData[7] << 8 | ResponseData[8];
                        var tmpcr = new System.Collections.BitArray(ResponseData[9] << 8 | ResponseData[10]);
                        for (int i = 0; i < tmpcr.Length; i++)
                        {
                            BillValidatorSetupData.BillSecurityLevel[i] = tmpcr[i];
                        }
                        BillValidatorSetupData.Escrow = ((ResponseData[11] & 0xff) == 1);
                        for (int i = 12; i < ResponseData.Length - 1; i++)
                        {
                            BillValidatorSetupData.BillTypeCredit[i - 12] = ResponseData[i];
                        }
                        return;
                    }
                    if (CheckBAStatus && (ResponseData.Length == 4))//Ожидаем статус, размер данных совпадает
                    {
                        CheckBAStatus = false;
                        bool isstackerfull = ((ResponseData[1] & 0x80) == 1);
                        int stackerbillscount = (ResponseData[7] << 8 | ResponseData[8]) & 0x0FFF;
                        if (DebugEnabled) MDBDebug?.Invoke("DEBUG: BA stacker status received"); else MDBBAStackerStatus?.Invoke(isstackerfull, stackerbillscount);
                        return;
                    }
                    switch (ResponseData[1] & 0x80)
                    {
                        case 1://принята купюра
                            int route = ResponseData[1] & 0x70;
                            double value = Math.Round(BillValidatorSetupData.BillScalingFactor * BillValidatorSetupData.BillTypeCredit[ResponseData[1] & 0x0F] * (1 / Math.Pow(10, BillValidatorSetupData.DecimalPlaces)), 2);
                            if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: Bill Inserted value={0}", value)); else MDBInsertedBill?.Invoke(value);
                            return;
                        case 0://сообщение статуса
                            if ((ResponseData[1] & 0x40) == 0)// ошибка
                                switch (ResponseData[1] & 0x7F)
                                {
                                    case 1:
                                        if (DebugEnabled) MDBDebug?.Invoke("Ошибка двигателя купюроприемника"); else MDBError?.Invoke("Ошибка двигателя купюроприемника");
                                        return;
                                    case 2:
                                        if (DebugEnabled) MDBDebug?.Invoke("Ошибка датчика купюроприемника"); else MDBError?.Invoke("Ошибка датчика купюроприемника");
                                        return;
                                    case 3:
                                        if (DebugEnabled) MDBDebug?.Invoke("Валидатор занят обработкой данных"); else MDBError?.Invoke("Валидатор занят обработкой данных");
                                        return;
                                    case 4:
                                        if (DebugEnabled) MDBDebug?.Invoke("Ошибка микропрограммы купюроприемника"); else MDBError?.Invoke("Ошибка микропрограммы купюроприемника");
                                        return;
                                    case 5:
                                        if (DebugEnabled) MDBDebug?.Invoke("Замятие в купюроприемнике"); else MDBError?.Invoke("Замятие в купюроприемнике");
                                        return;
                                    case 6:
                                        if (DebugEnabled) MDBDebug?.Invoke("Changer Reseted"); else MDBCCReseted?.Invoke();
                                        return;
                                    case 7:
                                        tmpmdbmsg = "BA Bill Removed";
                                        break;
                                    case 8:
                                        tmpmdbmsg = "BA Cash Box Out of Position";
                                        break;
                                    case 9:
                                        tmpmdbmsg = "BA Unit Disabled";
                                        break;
                                    case 10:
                                        tmpmdbmsg = "BA Invalid Escrow Request";
                                        break;
                                    case 11:
                                        tmpmdbmsg = "BA Bill Rejected";
                                        break;
                                    case 20:
                                        tmpmdbmsg = "BA Bill Not Recognized";
                                        break;
                                    default:
                                        break;
                                }
                            else//сообщение о попытках вставить купюру, пока купюроприемник был в нерабочем состоянии
                            {
                                if (DebugEnabled) MDBDebug?.Invoke(string.Format("Количество попыток вставить купюру в неактивном состоянии: {0}", ResponseData[1] & 0x1F)); else
                                    MDBError?.Invoke(string.Format("Количество попыток вставить купюру в неактивном состоянии: {0}", ResponseData[1] & 0x1F));
                                return;
                            }
                            break;
                    }
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
                }
                finally
                {
                    if (((tmpmdbmsg == "BA Unit Disabled") && !(BAAlreadyDisabled)) || (tmpmdbmsg != "BA Unit Disabled"))
                    {
                        if (DebugEnabled) MDBDebug?.Invoke(string.Format(tmpmdbmsg)); else MDBInformationMessageReceived?.Invoke(tmpmdbmsg);
                        if (tmpmdbmsg == "BA Unit Disabled") BAAlreadyDisabled = true; else BAAlreadyDisabled = false;
                    }
                }
                return;
            }
            if (ResponseData[0] == 0x08)//информация от монетоприемника
            {
                try
                {
                    if ((GetCCSettings) && (ResponseData.Length >= 9) && (ResponseData.Length <= 25))//настройки
                    {
                        GetCCSettings = false;
                        CoinChangerSetupData.ChangerFeatureLevel = ResponseData[1];
                        CoinChangerSetupData.CountryOrCurrencyCode = BCDByteToInt(new byte[2] { ResponseData[2], ResponseData[3] });
                        CoinChangerSetupData.CoinScalingFactor = ResponseData[4];
                        CoinChangerSetupData.DecimalPlaces = ResponseData[5];
                        var tmpcr = new System.Collections.BitArray(ResponseData[6] << 8 | ResponseData[7]);
                        for (int i = 0; i < tmpcr.Length; i++)
                        {
                            CoinChangerSetupData.CoinsRouteable[i] = tmpcr[i];
                        }
                        for (int i = 8; i < ResponseData.Length - 1; i++)
                        {
                            CoinChangerSetupData.CoinTypeCredit[i - 8] = ResponseData[i];
                        }
                        return;
                    }
                    if ((CheckDispenseResult) && (ResponseData.Length == 18))//информация о выданной сдаче
                    {
                        CheckDispenseResult = false;
                        List<CoinsRecord> tmpcr = new List<CoinsRecord> { };
                        for (int i = 1; i < ResponseData.Length - 1; i++)
                        {
                            if (ResponseData[i] != 0)
                            {
                                tmpcr.Add(new CoinsRecord { CoinsDispensed = ResponseData[i], CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2) });
                            }
                        }
                        if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: dispensed sum value={0}", tmpcr.Sum(x => x.CoinValue))); else MDBChangeDispensed?.Invoke(tmpcr);
                        return;
                    }
                    if ((CheckCCTubeStatus) && (ResponseData.Length == 20))//информация о заполнении трубок
                    {
                        CheckCCTubeStatus = false;;
                        List<CoinChangerTubeRecord> tmptr = new List<CoinChangerTubeRecord> { };
                        var tmpfullflags = new System.Collections.BitArray(ResponseData[1] << 8 | ResponseData[2]);
                        for (int i = 3; i < ResponseData.Length - 1; i++)
                        {
                            if (ResponseData[i] != 0)
                            {
                                tmptr.Add(new CoinChangerTubeRecord { CoinsCount = ResponseData[i], IsFull = tmpfullflags[i - 3], CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2) });
                            }
                        }
                        if (DebugEnabled) MDBDebug?.Invoke("DEBUG: CC tubes status received"); else MDBCCTubesStatus?.Invoke(tmptr);
                        return;
                    }
                    if ((ResponseData[1] & 0x80) == 1)//информация о выданных вручную монетах
                    {
                        //Coins Dispensed Manually
                        int CoinsQuantity = (ResponseData[1] & 0x70);//bits 5-7 of byte 1
                        int CoinType = (ResponseData[1] & 0x0F);//bits 1-4 of byte 1
                        int CoinsLeft = ResponseData[2];//byte 2
                        double CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[CoinType] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2);
                        if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: MDBCoinsDispensedManually value={0}, qty={1}, coinsleft={2}", CoinValue, CoinsQuantity, CoinsLeft)); else MDBCoinsDispensedManually?.Invoke(CoinValue, CoinsQuantity, CoinsLeft);
                        return;
                    }
                    if ((ResponseData[1] & 0x40) == 1)//зааинута монета
                    {
                        //Coins deposited
                        int CoinRouting = (ResponseData[1] & 0x30);//bits 5-6 of byte 1
                        int CoinType = (ResponseData[1] & 0x0F);//bits 1-4 of byte 1
                        int CoinsLeft = ResponseData[2];//byte 2
                        double CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[CoinType] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2);
                        switch (CoinRouting)
                        {
                            case 0://"Кэшбокс";
                                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: MDBInsertedCoinRoutedToCashBox value={0}", CoinValue)); else
                                    MDBInsertedCoinRoutedToCashBox?.Invoke(CoinValue);
                                return;
                            case 1://"Трубка монетоприемника";
                                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: MDBInsertedCoinRoutedToCCTube value={0}", CoinValue)); else
                                    MDBInsertedCoinRoutedToCCTube?.Invoke(CoinValue);
                                return;
                            case 3://Возврат
                                if (DebugEnabled) MDBDebug?.Invoke("Монета нераспознана, возвращена"); else MDBInformationMessageReceived?.Invoke("Монета нераспознана, возвращена");
                                return;
                            default://неизвестно
                                if (DebugEnabled) MDBDebug?.Invoke("Какая-то неведомая хуйня с монетой"); else MDBInformationMessageReceived?.Invoke("Какая-то неведомая хуйня с монетой");
                                return;
                        }
                    }
                    switch (ResponseData[1])//ошибка
                    {
                        case 1:
                            tmpmdbmsg = "CC Escrow Request";
                            break;
                        case 2:
                            tmpmdbmsg = "CC Changer Payout Busy";
                            DispenseTimeout = DateTime.Now.AddSeconds(5);
                            if (DebugEnabled) MDBDebug?.Invoke("CC Changer Payout Busy"); else MDBCCPayOutBusy?.Invoke();
                            if (!DispenseInProgress)
                            {
                                CoinChanger_DispenseWatch();
                            }
                            return;
                        case 3:
                            tmpmdbmsg = "CC No Credit";
                            break;
                        case 4:
                            if (DebugEnabled) MDBDebug?.Invoke("Ошибка датчика в трубе монетоприемника"); else MDBError?.Invoke("Ошибка датчика в трубе монетоприемника");
                            return;
                        case 5:
                            tmpmdbmsg = "CC Double Arrival";
                            break;
                        case 6:
                            if (DebugEnabled) MDBDebug?.Invoke("Монетоприемник открыт"); else MDBError?.Invoke("Монетоприемник открыт");
                            return;
                        case 7:
                            if (DebugEnabled) MDBDebug?.Invoke("Застревание монеты в трубе монетоприемника при выдаче сдачи"); else MDBError?.Invoke("Застревание монеты в трубе монетоприемника при выдаче сдачи");
                            return;
                        case 8:
                            if (DebugEnabled) MDBDebug?.Invoke("Ошибка микропрограммы монетоприемника"); else MDBError?.Invoke("Ошибка микропрограммы монетоприемника");
                            return;
                        case 9:
                            if (DebugEnabled) MDBDebug?.Invoke("Ошибка направляющих механизмов монетоприемника"); else MDBError?.Invoke("Ошибка направляющих механизмов монетоприемника");
                            return;
                        case 10:
                            tmpmdbmsg = "CC Changer Busy";
                            break;
                        case 11:
                            MDBCCReseted?.Invoke();
                            return;
                        case 12:
                            if (DebugEnabled) MDBDebug?.Invoke("Застревание монеты"); else MDBError?.Invoke("Застревание монеты");
                            return;
                        default:
                            break;
                    }
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
                }
                finally
                {
                    if (DebugEnabled) MDBDebug?.Invoke(tmpmdbmsg); else MDBInformationMessageReceived?.Invoke(tmpmdbmsg);
                    BAAlreadyDisabled = false;
                }
                return;
            }
        }

        /// <summary>
        /// Преобразует массив байт BCD в целое число
        /// </summary>
        /// <param name="BCDBytes"></param>
        /// <returns></returns>
        public static int BCDByteToInt(byte[] BCDBytes)
        {
            int res = 0;
            for (int i = 0; i < BCDBytes.Length; i++)
            {
                res *= 100;
                res += (10 * (BCDBytes[i] >> 4));
                res += (BCDBytes[i] & 0xf);
            }
            return res;
        }

        /// <summary>
        /// Возвращаем купюру из Escrow
        /// </summary>
        public static void ReturnBill()
        {
            AddCommand(MDBCommands.ReturnBill);
        }

        /// <summary>
        /// запрос настроек валидатора
        /// </summary>
        public static void GetBillValidatorSettings()
        {
            GetBASettings = true;
            AddCommand(MDBCommands.BillValidatorSetup);
        }

        /// <summary>
        /// запрос настроек монетоприемника
        /// </summary>
        public static void GetChangerSettings()
        {
            GetCCSettings = true;
            AddCommand(MDBCommands.ChangerSetup);
        }

        /// <summary>
        /// Выдаем сдачу монетами
        /// </summary>
        /// <param name="PayOutSum"></param>
        public static void PayoutCoins(int PayOutSum)
        {
            byte[] payouttmpcmd = MDBCommands.PayoutCoins(PayOutSum);
            AddCommand(payouttmpcmd);
        }

        /// <summary>
        /// Запрашиваем информацию о выданной сдаче
        /// </summary>
        public static void GetDispensedInfo()
        {
            AddCommand(MDBCommands.GetDispensedCoinsInfo);
        }

        /// <summary>
        /// Отсылает команды на шину MDB в "как бы полудуплексном" режиме
        /// </summary>
        /// <returns></returns>
        public static async Task SendCommandTask()
        {
            while (MDBSerialPort == null)
            {
                await Task.Delay(100);
            }
            while (true)
            {
                MDBCommandsListSemaphore.Wait();
                try
                {
                    foreach (var cmd in CommandList)
                    {
                        using (DataWriter MDBSerialDataWriteObject = new DataWriter(MDBSerialPort.OutputStream))
                        {
                            var dw = new Stopwatch();
                            dw.Start();//запускаем секундомер
                            MDBAbswerFromByteSemaphore.Wait();//ждем освобождения переменной
                            AwaitingMDBAnswerFrom = cmd[0];//присваиваем значение адреса
                            MDBSerialDataWriteObject.WriteBytes(cmd);
                            await MDBSerialDataWriteObject.StoreAsync();//пишем в порт
                            MDBAbswerFromByteSemaphore.Release();//освобождаем переменную
                            while (dw.ElapsedMilliseconds < 500)//ждем ответа от устройства 0.5с...
                            {
                                await Task.Delay(50);
                                if (AwaitingMDBAnswerFrom == 0x00) break;//...либо если ответ пришел
                            }
                            dw.Stop();
                            if (DebugEnabled)
                            {
                                StringBuilder hex = new StringBuilder(cmd.Length * 2);
                                foreach (byte b in cmd) hex.AppendFormat("{0:x2}", b);
                                MDBDebug?.Invoke(string.Format("DEBUG WRITE: {0}", hex.ToString()));
                            }
                            MDBAbswerFromByteSemaphore.Wait();//ждем освобождения переменной
                            AwaitingMDBAnswerFrom = 0x00;
                            MDBAbswerFromByteSemaphore.Release();//освобождаем переменную
                            MDBSerialDataWriteObject?.DetachStream();
                        }
                    }
                    CommandList.Clear();
                    MDBCommandsListSemaphore.Release();
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
                }
                finally
                {

                }
                await Task.Delay(500);
            }
        }


        /// <summary>
        /// Добавляет элемент в список команд для отправки по шине MDB
        /// </summary>
        /// <param name="cmd"></param>
        /// <param name="dispense"></param>
        private static void AddCommand(byte[] cmd, bool dispense = false)
        {
            MDBCommandsListSemaphore.Wait();
            CommandList.Add(cmd);
            MDBCommandsListSemaphore.Release();
        }

        /// <summary>
        /// Добавляет элементы в конец списка команд для отправки по шине MDB
        /// </summary>
        /// <param name="cmd"></param>
        /// <param name="dispense"></param>
        private static void AddCommand(byte[][] cmds, bool dispense = false)
        {
            MDBCommandsListSemaphore.Wait();
            foreach (var cmd in cmds)
            {
                CommandList.Add(cmd);
            }
            MDBCommandsListSemaphore.Release();
        }

        /// <summary>
        /// Запрещаем прием монет
        /// </summary>
        public static void DisableAcceptCoins()
        {
            AddCommand(MDBCommands.DisableAcceptCoins);
        }

        /// <summary>
        /// Запрещаем прием купюр
        /// </summary>
        public static void DisableAcceptBills()
        {
            AddCommand(MDBCommands.DisableAcceptBills);
        }

        /// <summary>
        /// разрешаем прием монет
        /// </summary>
        public static void EnableAcceptCoins()
        {
            AddCommand(MDBCommands.EnableAcceptCoins);
        }

        /// <summary>
        /// Разрешаем выдачу монет
        /// </summary>
        public static void EnableDispenseCoins()
        {
            AddCommand(MDBCommands.EnableDispenseCoins);
        }

        /// <summary>
        /// разрешаем прием наличных
        /// </summary>
        public static void EnableCashDevices()
        {
            AddCommand(new byte[][] { MDBCommands.BillValidatorSetup, MDBCommands.ChangerSetup, MDBCommands.EnableAcceptCoins, MDBCommands.EnableAcceptBills });
        }

        /// <summary>
        /// запрещаем прием наличных
        /// </summary>
        public static void DisableCashDevices()
        {
            AddCommand(new byte[][] { MDBCommands.DisableAcceptCoins, MDBCommands.DisableAcceptBills });
        }

        /// <summary>
        /// Разрешаем прием купюр
        /// </summary>
        public static void EnableAcceptBills()
        {
            AddCommand(new byte[][] { MDBCommands.BillValidatorSetup, MDBCommands.EnableAcceptBills });
        }

        /// <summary>
        /// Выполняет сброс монетоприемника
        /// </summary>
        public static void ResetCC()
        {
            AddCommand(MDBCommands.ResetCC);//Reset CC
        }

        /// <summary>
        /// Выполняет сброс купюроприемника
        /// </summary>
        public static void ResetBA()
        {
            AddCommand(MDBCommands.ResetBA);//Reset Bill Validator
        }

        /// <summary>
        /// Выполняет сброс купюро и монетоприемника
        /// </summary>
        public static void ResetCashDevices()
        {
            AddCommand(new byte[][] { MDBCommands.ResetBA, MDBCommands.ResetCC });//Reset bill validator and coin changer
        }

        /// <summary>
        /// Запрашиваем состояние стекера купюроприемника
        /// </summary>
        public static void GetBAStatus()
        {
            CheckBAStatus = true;
            AddCommand(MDBCommands.GetBAStatus); //Request Stacker Status
        }

        /// <summary>
        /// Запрашиваем состояние монетоприемника
        /// </summary>
        public static void GetCCStatus()
        {
            CheckCCTubeStatus = true;
            AddCommand(MDBCommands.GetCCStatus); //Request CC Tube Status
        }
        /// <summary>
        /// Структура команд MDB
        /// </summary>
        private static class MDBCommands
        {
            public static byte[] DisableAcceptBills = new byte[] { 0x34, 0x00, 0x00, 0x00, 0x00, 0x34 };
            public static byte[] DisableAcceptCoins = new byte[] { 0x0C, 0x00, 0x00, 0x00, 0xFF, 0x0B };
            public static byte[] DispenseCoinsInProgressMessage = new byte[] { 0x08, 0x02 };
            public static byte[] EnableAcceptBills = new byte[] { 0x34, 0x00, 0x06, 0x00, 0x00, 0x3A };
            public static byte[] EnableAcceptCoins = new byte[] { 0x0C, 0x00, 0xFF, 0x00, 0xFF, 0x0A };
            public static byte[] EnableDispenseCoins = new byte[] { 0x0C, 0x00, 0x00, 0x00, 0xFF, 0x0B };
            public static byte[] GetBAStatus = new byte[] { 0x36, 0x36 };
            public static byte[] GetCCStatus = new byte[] { 0x0A, 0x0A };
            public static byte[] GetDispensedCoinsInfo = new byte[] { 0x0F, 0x03, 0x12 };
            public static byte[] ResetBA = new byte[] { 0x30, 0x30 };
            public static byte[] ResetCC = new byte[] { 0x08, 0x08 };
            public static byte[] ChangerSetup = new byte[] { 0x09, 0x09 };
            public static byte[] BillValidatorSetup = new byte[] { 0x31, 0x31 };
            public static byte[] ReturnBill = new byte[] { 0x35, 0x00, 0x35 };
            /// <summary>
            /// Команда выдачи сдачи
            /// </summary>
            /// <param name="PayOutSum">Максимальная сумма 127 рублей (при коэффициент x2 (на моем экземпляре устройства) в один байт больше не влезет)</param>
            /// <returns></returns>
            public static byte[] PayoutCoins(int PayOutSum)
            {
                byte[] payouttmpcmd = new byte[4] { 0x0F, 0x02, 0x00, 0x00 };
                payouttmpcmd[2] = (byte)((PayOutSum & 0x7f) * 2);
                payouttmpcmd[3] = (byte)((payouttmpcmd[0] + payouttmpcmd[1] + payouttmpcmd[2]) & 0xff);
                return payouttmpcmd;
            }
        }
    }
}

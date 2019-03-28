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
        public delegate void MDBCommmonEventDelegate();
        public delegate void MDBCommonMessageDelegate(string Message);
        public delegate void MDBCommonCashDelegate(double CashValue);
        public delegate void MDBCoinsDispensedManuallyDelegate(double CoinValue, int CoinsDispensed, int CoinsLeft);
        public delegate void MDBChangeDispensedDelegate(List<CoinsRecord> DispensedCoinsData);
        public delegate void MDBCoinChangerTubesStatusDelegate(List<CoinChangerTubeRecord> CoinChangerTubeStatusData);
        public delegate void MDBBillValidatorStackerStatusDelegate(bool StackerFull, int StackerBillsCount);

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
        /// входящие данные с шины MDB
        /// </summary>
        private static List<byte> IncomingMDBData = new List<byte> { };
        /// <summary>
        /// Статус готовности монетоприемника
        /// </summary>
        public static bool CoinChangerReadyStatus = true;
        /// <summary>
        /// Статус монетоприемника
        /// </summary>
        public static bool CoinChangerPluggedStatus = true;
        /// <summary>
        /// флаг готовности купюроприемника к приему команд
        /// </summary>
        public static bool BillValidatorReadyStatus = true;
        /// <summary>
        /// флаг готовности к приему наличных
        /// </summary>
        public static bool BillValidatorEnableStatus = true;
        /// <summary>
        /// Флаг ожидания статуса стекера купюроприемника
        /// </summary>
        private static bool AwaitBillValidatorStackerStatus = false;
        /// <summary>
        /// Флаг ожидания статуса трубок монетоприемника
        /// </summary>
        private static bool AwaitCoinChangerTubeStatus = false;
        /// <summary>
        /// Флаг ожидания настроек монетоприемника
        /// </summary>
        private static bool AwaitCoinChangerSettings = false;
        /// <summary>
        /// Флаг ожидания расширенной информации о монетоприемнике
        /// </summary>
        private static bool AwaitCoinChangerID = false;
        /// <summary>
        /// Флаг ожидания расширенной информации о купюроприемнике
        /// </summary>
        private static bool AwaitBillValidatorID = false;
        /// <summary>
        /// Флаг ожидания настроек купюроприемника
        /// </summary>
        private static bool AwaitBillValidatorSettings = false;
        /// <summary>
        /// Флаг отладочного режима
        /// </summary>
        public static bool DebugEnabled = false;
        /// <summary>
        /// Адрес устройства, от которого ожидается ответ
        /// </summary>
        private static byte AwaitingMDBillValidatornswerFrom = 0x00;
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
        public static bool AwaitDispenseResult = false;
        /// <summary>
        /// Таймаут ожидания выдачи сдачи (обновляется при каждом получении информации о выдаче)
        /// </summary>
        public static DateTime DispenseTimeout = new DateTime();
        /// <summary>
        /// Таймаут ожидания выхода купюроприемника из нерабочего режима
        /// </summary>
        public static DateTime BillValidatorDisabledTimeout = new DateTime();
        /// <summary>
        /// Таймаут ожидания выхода купюроприемника из режима "занят"
        /// </summary>
        public static DateTime BillValidatorBusyTimeout = new DateTime();
        /// <summary>
        /// Таймаут ожидания подключения монетоприемника
        /// </summary>
        public static DateTime CoinChangerUnpluggedTimeout = new DateTime();
        /// <summary>
        /// Таймаут ожидания выхода монетоприемника из режима "занят"
        /// </summary>
        public static DateTime CoinChangerBusyTimeout = new DateTime();

        /// <summary>
        /// Монетоприемник занят
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerBusy;
        /// <summary>
        /// Монетоприемник готов
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerReady;
        /// <summary>
        /// Монетоприемник закрыт
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerPlugged;
        /// <summary>
        /// Монетоприемник открыт
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerUnplugged;
        /// <summary>
        /// Купюроприемник "Занят"
        /// </summary>
        public static event MDBCommmonEventDelegate MDBBillValidatorBusy;
        /// <summary>
        /// Купюроприемник вышел из режима "Занят"
        /// </summary>
        public static event MDBCommmonEventDelegate MDBBillValidatorReady;
        /// <summary>
        /// Стекер снят
        /// </summary>
        public static event MDBCommmonEventDelegate MDBBillValidatorCashBoxRemoved;
        /// <summary>
        /// Монетоприемник закончил выдачу сдачи
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerPayoutComplete;
        /// <summary>
        /// валидатор вышел из нерабочего режима
        /// </summary>
        public static event MDBCommmonEventDelegate MDBBillValidatorEnabled;
        /// <summary>
        /// валидатор не готов к приему наличных
        /// </summary>
        public static event MDBCommmonEventDelegate MDBBillValidatorDisabled;
        /// <summary>
        /// Адаптер MDB-RS232 стартанул
        /// </summary>
        public static event MDBCommmonEventDelegate MDBAdapterStarted;
        /// <summary>
        /// В режиме отладки отсылаем сырые данные, которые пришли с адаптера
        /// </summary>
        public static event MDBCommonMessageDelegate MDBDebug;
        /// <summary>
        /// Монеты выданы вручную
        /// </summary>
        public static event MDBCoinsDispensedManuallyDelegate MDBCoinsDispensedManually;
        /// <summary>
        /// Купюра вставлена
        /// </summary>
        public static event MDBCommonCashDelegate MDBInsertedBill;
        /// <summary>
        /// Ошибка при работе с шиной MDB
        /// </summary>
        public static event MDBCommonMessageDelegate MDBError;
        /// <summary>
        /// Ошибка при обработке данных
        /// </summary>
        public static event MDBCommonMessageDelegate MDBDataProcessingError;
        /// <summary>
        /// Монета упала в кешбокс
        /// </summary>
        public static event MDBCommonCashDelegate MDBInsertedCoinRoutedToCashBox;
        /// <summary>
        /// Монета упала в трубку
        /// </summary>
        public static event MDBCommonCashDelegate MDBInsertedCoinRoutedToCoinChangerTube;
        /// <summary>
        /// Выдана сдача
        /// </summary>
        public static event MDBChangeDispensedDelegate MDBChangeDispensed;
        /// <summary>
        /// Статус заполнения монетоприемника
        /// </summary>
        public static event MDBCoinChangerTubesStatusDelegate MDBCoinChangerTubesStatus;
        /// <summary>
        /// Статус заполнения стекера купюроприемника
        /// </summary>
        public static event MDBBillValidatorStackerStatusDelegate MDBBillValidatorStackerStatus;
        /// <summary>
        /// Информационное сообщение
        /// </summary>
        public static event MDBCommonMessageDelegate MDBInformationMessageReceived;
        /// <summary>
        /// Купюроприемник выполнил команду сброс
        /// </summary>
        public static event MDBCommmonEventDelegate MDBBillValidatorReseted;
        /// <summary>
        /// Монетоприемник выполнил команду сброс
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerReseted;
        /// <summary>
        /// Монетоприемник занят вдачей сдачи
        /// </summary>
        public static event MDBCommmonEventDelegate MDBCoinChangerPayoutStarted;
        /// <summary>
        /// флаг доступа к исходящим командам
        /// </summary>
        private static SemaphoreSlim MDBCommandsListSemaphore = new SemaphoreSlim(1);
        /// <summary>
        /// флаг доступа к адресу устройства, от которого ожидается ответ
        /// </summary>
        private static SemaphoreSlim MDBAnswerFromByteSemaphore = new SemaphoreSlim(1);
        /// <summary>
        /// флаг доступа к обработчику ответов от устройств
        /// </summary>
        private static SemaphoreSlim MDBDataProcessSemaphore = new SemaphoreSlim(1);

        /// <summary>
        /// структура настроек монетоприемника
        /// </summary>
        public static class CoinChangerSetupData
        {
            public static int CoinChangerFeatureLevel = 2;
            public static int CountryOrCurrencyCode = 0;
            public static int CoinScalingFactor = 1;
            public static int DecimalPlaces = 0;
            public static bool[] CoinsRouteable = new bool[16];
            public static int[] CoinTypeCredit = new int[16];
        }

        /// <summary>
        /// структура расширенной информации о монетоприемнике
        /// </summary>
        public static class CoinChangerIDData
        {
            public static string ManufacturerCode = "";
            public static string SerialNumber = "";
            public static string ModelRevision = "";
            public static int SoftwareVersion = 0;
            public static bool AlternativePayout = false;
            public static bool ExtendedDiagnostic = false;
            public static bool ControlledManualFillAndPayout = false;
            public static bool FTLSupported = false;
            public static bool[] FutureUseLast28Flags = new bool[28];
        }

        /// <summary>
        /// структура расширенной информации о купюроприемнике
        /// </summary>
        public static class BillValidatorIDData
        {
            public static string ManufacturerCode = "";
            public static string SerialNumber = "";
            public static string ModelRevision = "";
            public static int SoftwareVersion = 0;
            public static bool BillRecyclingSupported = false;
            public static bool FTLSupported = false;
            public static bool[] FutureUseLast30Flags = new bool[30];
            public static int ExpectedDataLength = 0;
        }

        /// <summary>
        /// структура настроек купюроприемника
        /// </summary>
        public static class BillValidatorSetupData
        {
            public static int BillValidatorFeatureLevel = 1;
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
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                Task.Run(ListenMDBSerialPort, token);
                Task.Run(DispensedCoinsInfoTask, token);
                Task.Run(SendCommandTask, token);
                Task.Run(IncomingKKTDataWatcher, token);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
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
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                Task.Run(ListenMDBSerialPort, token);
                Task.Run(DispensedCoinsInfoTask, token);
                Task.Run(SendCommandTask, token);
                Task.Run(IncomingKKTDataWatcher, token);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
            }
        }

        /// <summary>
        /// Считывает данные с ппоследовательного порта шины MDB
        /// </summary>
        private static async Task ListenMDBSerialPort()
        {
            while (true)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: Reading initiate")); else MDBInformationMessageReceived?.Invoke("Порт MDB открыт, ожидание данных...");
                try
                {
                    MDBSerialDataReaderObject = new DataReader(MDBSerialPort.InputStream)
                    {
                        InputStreamOptions = InputStreamOptions.Partial
                    };
                    while (true)
                    {
                        Task<uint> loadAsyncTask = MDBSerialDataReaderObject.LoadAsync(128).AsTask();
                        uint bytesRead = 0;
                        bytesRead = await loadAsyncTask;
                        if (bytesRead > 0)
                        {
                            try
                            {
                                byte[] tmpbyte = new byte[bytesRead];
                                MDBSerialDataReaderObject.ReadBytes(tmpbyte);
#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                                AddIncomingData(tmpbyte);
#pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                            }
                            catch
                            {

                            }
                            finally
                            {

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
        /// Постоянно проверяем наличие входящих данных от MDB устройств
        /// </summary>
        /// <returns></returns>
#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        public static async Task IncomingKKTDataWatcher()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            while (true)
            {
                Mutex m = new Mutex(true, "MDBIncomingDataAccessMutex", out bool mutexWasCreated);
                if (!mutexWasCreated)
                {
                    try
                    {
                        m.WaitOne();
                    }
                    catch (AbandonedMutexException)
                    {

                    }
                }
                if (IncomingMDBData.Count > 0)
                {
                    int endpos = 0;
                    for (int i = 0; i < IncomingMDBData.Count; i++)
                    {
                        //if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG byte: {0}", IncomingMDBData[i]));
                        if ((IncomingMDBData[i] == 10))
                        {
                            endpos = i;
                            break;
                        }
                    }
                    if (endpos >= 0)
                    {
                        try
                        {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                            MDBIncomingDataProcess(IncomingMDBData.GetRange(0, endpos - 1).ToArray());
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                            IncomingMDBData.RemoveRange(0, endpos + 1);
                        }
                        catch
                        {

                        }
                    }
                }
                m.ReleaseMutex();
                m.Dispose();
                Task.Delay(10).Wait();
            }
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Добавляет данные из входящего потока в массив для дальнейшей обработки
        /// </summary>
        /// <param name="answerdata"></param>
        /// <returns></returns>
        public static async Task AddIncomingData(byte[] answerdata)
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            Mutex m = new Mutex(true, "MDBIncomingDataAccessMutex", out bool mutexWasCreated);
            if (!mutexWasCreated)
            {
                try
                {
                    m.WaitOne();
                }
                catch (AbandonedMutexException)
                {

                }
            }
            if (DebugEnabled)
            {
                StringBuilder hex = new StringBuilder(answerdata.Length * 2);
                foreach (byte b in answerdata) hex.AppendFormat("{0:x2} ", b);
                MDBDebug?.Invoke(string.Format("DEBUG: bytes count = {0}, RAW: {1}", answerdata.Length, hex.ToString()));
            }
            IncomingMDBData.AddRange(answerdata.ToList());
            m.ReleaseMutex();
            m.Dispose();
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
        /// Запускается при первом сообщении "BillValidator Unit Disabled".
        /// </summary>
        /// <returns></returns>
        private static async Task BillValidator_DisabledWatch()
        {
            BillValidatorEnableStatus = false;
            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Unit Disabled"); else MDBBillValidatorDisabled?.Invoke();
            while (BillValidatorDisabledTimeout > DateTime.Now)
            {
                await Task.Delay(100);
            }
            BillValidatorEnableStatus = true;
            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Unit Enabled"); else MDBBillValidatorEnabled?.Invoke();
        }

        /// <summary>
        /// Запускается при первом сообщении "Валидатор занят обработкой данных".
        /// </summary>
        /// <returns></returns>
        private static async Task BillValidator_BusyWatch()
        {
            BillValidatorReadyStatus = false;
            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Unit Busy"); else MDBBillValidatorBusy?.Invoke();
            while (BillValidatorBusyTimeout > DateTime.Now)
            {
                await Task.Delay(100);
            }
            BillValidatorReadyStatus = true;
            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Unit Ready"); else MDBBillValidatorReady?.Invoke();
        }

        /// <summary>
        /// Запускается при первом сообщении "Монетоприемник открыт".
        /// </summary>
        /// <returns></returns>
        private static async Task CoinChanger_UnpluggedWatch()
        {
            CoinChangerPluggedStatus = false;
            if (DebugEnabled) MDBDebug?.Invoke("Acceptor Unplugged"); else MDBCoinChangerUnplugged?.Invoke();
            while (CoinChangerUnpluggedTimeout > DateTime.Now)
            {
                await Task.Delay(100);
            }
            CoinChangerPluggedStatus = true;
            if (DebugEnabled) MDBDebug?.Invoke("Acceptor Plugged"); else MDBCoinChangerPlugged?.Invoke();
        }

        /// <summary>
        /// Запускается в начале выдачи сдачи монетами (при первом сообщении "CoinChanger Payout Busy").
        /// </summary>
        /// <returns></returns>
        private static async Task CoinChanger_DispenseWatch()
        {
            DispenseInProgress = true;
            if (DebugEnabled) MDBDebug?.Invoke("CoinChanger Payout Started"); else MDBCoinChangerPayoutStarted?.Invoke();
            while (DispenseTimeout > DateTime.Now)
            {
                await Task.Delay(100);
            }
            DispenseInProgress = false;
            AwaitDispenseResult = true;
            if (DebugEnabled) MDBDebug?.Invoke("CoinChanger Payout Complete"); else MDBCoinChangerPayoutComplete?.Invoke();
        }

        /// <summary>
        /// Запускается при первом сообщении "CoinChanger Busy".
        /// </summary>
        /// <returns></returns>
        private static async Task CoinChanger_BusyWatch()
        {
            CoinChangerReadyStatus = false;
            if (DebugEnabled) MDBDebug?.Invoke("CoinChanger Busy"); else MDBCoinChangerBusy?.Invoke();
            while (DispenseTimeout > DateTime.Now)
            {
                await Task.Delay(100);
            }
            CoinChangerReadyStatus = true;
            if (DebugEnabled) MDBDebug?.Invoke("CoinChanger Ready"); else MDBCoinChangerReady?.Invoke();
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
                    if (AwaitDispenseResult)
                    {
#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                        GetDispensedInfo();
#pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                        if (DebugEnabled) MDBDebug?.Invoke("Ожидаем данные о выданной сдаче..."); else MDBInformationMessageReceived?.Invoke("Ожидаем данные о выданной сдаче...");
                        int RetryCount = 0;
                        while (AwaitDispenseResult && RetryCount < 51)//ждать ответа от монетоприемника будем 5 секунд, потом забъем хуй и продолжим работать
                        {
                            Task.Delay(100).Wait();//пауза на 0.1сек
                            RetryCount++;
                        }
                        AwaitDispenseResult = false;
                    }
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
                }
            }
        }

#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        /// <summary>
        /// MDB answer processing method.
        /// Must fire all MDB events, all events must be handled externally as well.
        /// </summary>
        /// <param name="MDBStringData">String representation if incoming MDB serial data</param>
        /// <returns>none</returns>
        private static async Task MDBIncomingDataProcess(byte[] MDBStringData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            byte[] ResponseData = new byte[] { };
            bool process = true;
            try
            {
                string tmpstr = Encoding.UTF8.GetString(MDBStringData).Trim();
                tmpstr = tmpstr.Replace("\r", "").Replace("\n", "");
                if (MDBStringData.Length >= 120)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}", "Слишком большой пакет (>40 байт)")); else MDBInformationMessageReceived?.Invoke("Слишком большой пакет (>40 байт)");
                    process = false;
                }
                if (tmpstr == "MDB-UART PLC ready") //MDB-UART adapter PLC started
                {
                    if (DebugEnabled) MDBDebug?.Invoke("DEBUG: MDBAdapterStarted"); else MDBAdapterStarted?.Invoke();
                    process = false;
                }
                string trimmedstr = tmpstr.Replace(" ", "");
                ResponseData = Enumerable.Range(0, trimmedstr.Length)
                     .Where(x => x % 2 == 0)
                     .Select(x => Convert.ToByte(trimmedstr.Substring(x, 2), 16))
                     .ToArray();
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: actual length: {0}, data: {1}", ResponseData.Length, tmpstr));
                if (ResponseData.Length == 2)//просто ACK от устройства, обрабатывать не надо
                {
                    process = false;
                }
                if (process)
                {
                    MDBAnswerFromByteSemaphore.Wait();
                    if (ResponseData[0] == AwaitingMDBillValidatornswerFrom)
                    {
                        AwaitingMDBillValidatornswerFrom = 0x00;
                    }
                    MDBAnswerFromByteSemaphore.Release();
                    if (ResponseData[0] == 0x30)//данные от купюроприемника
                    {
                        ProcessBillValidatorResponse(ResponseData);
                    }
                    if (ResponseData[0] == 0x08)//информация от монетоприемника
                    {
                        ProcessCoinChangerResponse(ResponseData);
                    }
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke("Ошибка при обработке данных MDB");
            }
            finally
            {
                
            }
        }

        private static void ProcessCoinChangerResponse(byte[] ResponseData)
        {
            try
            {
                if (AwaitCoinChangerSettings && (ResponseData.Length >= 9) && (ResponseData.Length <= 25))//настройки
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessChangetSettings(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                if ((AwaitDispenseResult) && (ResponseData.Length == 18))//информация о выданной сдаче
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessCoinChangerDispensedChangeData(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                if ((AwaitCoinChangerTubeStatus) && (ResponseData.Length == 20))//информация о заполнении трубок
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessCoinChangerTubeStatus(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                if (AwaitCoinChangerID && (ResponseData.Length == 35))//расширенная информация о монетоприемнике
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessCoinChangerExtendedInfo(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                //The CoinChanger may send several of one type activity *, up to 16 bytes
                //total.This will permit zeroing counters such as slug, inventory, and
                //status.
                //*Type activity is defined as Coins Dispensed Manually, Coins Deposited,
                //Status, and Slug. All may be combined in a response to a POLL command
                //providing the total number of bytes does not exceed 16.Note that Coins
                //Dispensed Manually and Coins Deposited are dual byte codes.
                for (int i = 1; i < ResponseData.Length - 1; i++)
                {
                    if ((ResponseData[i] & 0x80) >> 7 == 1)//информация о выданных вручную монетах
                    {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                        ProcessManualDispenseData(new byte[2] { ResponseData[i], ResponseData[i + 1] });
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                        i++;
                    } else
                    if ((ResponseData[i] & 0x40) >> 6 == 1)//закинута монета
                    {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                        ProcessCoinChangerDepositCoin(new byte[2] { ResponseData[i], ResponseData[i + 1] });
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                        i++;
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    }
                    else ProcessCoinChangerError(ResponseData[i]);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {
                
            }
        }

        /// <summary>
        /// Обработка информации о настройках монетоприемника
        /// </summary>
        /// <param name="CoinChangerSettingsData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessChangetSettings(byte[] CoinChangerSettingsData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                AwaitCoinChangerSettings = false;
                CoinChangerSetupData.CoinChangerFeatureLevel = CoinChangerSettingsData[1];
                CoinChangerSetupData.CountryOrCurrencyCode = BCDByteToInt(new byte[2] { CoinChangerSettingsData[2], CoinChangerSettingsData[3] });
                CoinChangerSetupData.CoinScalingFactor = CoinChangerSettingsData[4];
                CoinChangerSetupData.DecimalPlaces = CoinChangerSettingsData[5];
                var tmpcr = new System.Collections.BitArray(CoinChangerSettingsData[6] << 8 | CoinChangerSettingsData[7]);
                for (int i = 0; i < tmpcr.Length; i++)
                {
                    CoinChangerSetupData.CoinsRouteable[i] = tmpcr[i];
                }
                for (int i = 8; i < CoinChangerSettingsData.Length - 1; i++)
                {
                    CoinChangerSetupData.CoinTypeCredit[i - 8] = CoinChangerSettingsData[i];
                }
                if (DebugEnabled) MDBDebug?.Invoke("DEBUG: CoinChangerSetupData received"); else MDBInformationMessageReceived?.Invoke("CoinChangerSetupData received");
                if (CoinChangerSetupData.CoinChangerFeatureLevel == 3)
    #pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    GetCoinChangerIdentification();
    #pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обработка информации о выданной сдаче
        /// </summary>
        /// <param name="ChangeData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessCoinChangerDispensedChangeData(byte[] ChangeData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                AwaitDispenseResult = false;
                List<CoinsRecord> tmpcr = new List<CoinsRecord> { };
                for (int i = 1; i < ChangeData.Length - 1; i++)
                {
                    if (ChangeData[i] != 0)
                    {
                        tmpcr.Add(new CoinsRecord { CoinsDispensed = ChangeData[i], CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i - 1] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2) });
                    }
                }
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: dispensed sum value={0}", tmpcr.Sum(x => x.CoinValue))); else MDBChangeDispensed?.Invoke(tmpcr);
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// обработка информации о заполнении трубок монетоприемника
        /// </summary>
        /// <param name="TubeStatusData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessCoinChangerTubeStatus(byte[] TubeStatusData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                AwaitCoinChangerTubeStatus = false;
                List<CoinChangerTubeRecord> tmptr = new List<CoinChangerTubeRecord> { };
                var tmpfullflags = new System.Collections.BitArray(TubeStatusData[1] << 8 | TubeStatusData[2]);
                for (int i = 3; i < TubeStatusData.Length - 1; i++)
                {
                    tmptr.Add(new CoinChangerTubeRecord { CoinsCount = TubeStatusData[i], IsFull = tmpfullflags[i - 3], CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i - 3] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2) });
                }
                if (DebugEnabled) MDBDebug?.Invoke("DEBUG: CoinChanger tubes status received"); else MDBCoinChangerTubesStatus?.Invoke(tmptr);
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обработка расширенной информации о монетоприемнике
        /// </summary>
        /// <param name="CoinChangerExtInfo"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessCoinChangerExtendedInfo(byte[] CoinChangerExtInfo)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                AwaitCoinChangerID = false;
                CoinChangerIDData.ManufacturerCode = Encoding.ASCII.GetString(new byte[] { CoinChangerExtInfo[1], CoinChangerExtInfo[2], CoinChangerExtInfo[3], });
                byte[] snarray = new byte[12];
                Array.Copy(CoinChangerExtInfo, 4, snarray, 0, 12);
                CoinChangerIDData.SerialNumber = Encoding.ASCII.GetString(snarray);
                byte[] modarray = new byte[12];
                Array.Copy(CoinChangerExtInfo, 16, modarray, 0, 12);
                CoinChangerIDData.ModelRevision = Encoding.ASCII.GetString(modarray);
                CoinChangerIDData.SoftwareVersion = BCDByteToInt(new byte[] { CoinChangerExtInfo[28], CoinChangerExtInfo[29] });
                byte[] futusearray = new byte[4];
                Array.Copy(CoinChangerExtInfo, 30, futusearray, 0, 4);
                var tmpfullflags = new System.Collections.BitArray(futusearray);
                CoinChangerIDData.AlternativePayout = tmpfullflags[0];
                CoinChangerIDData.ExtendedDiagnostic = tmpfullflags[1];
                CoinChangerIDData.ControlledManualFillAndPayout = tmpfullflags[2];
                CoinChangerIDData.FTLSupported = tmpfullflags[3];
                for (int i = 4; i < tmpfullflags.Length; i++)
                {
                    CoinChangerIDData.FutureUseLast28Flags[i - 4] = tmpfullflags[i];
                }
                if (DebugEnabled) MDBDebug?.Invoke("DEBUG: CoinChangerIDData received"); else MDBInformationMessageReceived?.Invoke("CoinChangerIDData received");
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обработка информации о сдаче выданной вручную
        /// </summary>
        /// <param name="ManualDispenseData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessManualDispenseData(byte[] ManualDispenseData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            {
                int CoinsQuantity = ((ManualDispenseData[0] & 0x70) >> 4);//bits 5-7 of byte 1
                int CoinType = (ManualDispenseData[0] & 0x0F);//bits 1-4 of byte 1
                int CoinsLeft = ManualDispenseData[1];//byte 2
                double CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[CoinType] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2);
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: MDBCoinsDispensedManually type={3}, value={0}, qty={1}, coinsleft={2}", CoinValue, CoinsQuantity, CoinsLeft, CoinType)); else MDBCoinsDispensedManually?.Invoke(CoinValue, CoinsQuantity, CoinsLeft);
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обработка информации об ошибке монетоприемника
        /// </summary>
        /// <param name="CoinChangerErrorData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        private static async Task ProcessCoinChangerError(byte CoinChangerErrorData)
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            try
            { 
                switch (CoinChangerErrorData)//ошибка
                {
                    case 1:
                        if (DebugEnabled) MDBDebug?.Invoke("CoinChanger Escrow Request"); else MDBInformationMessageReceived?.Invoke("CoinChanger Escrow Request");
                        break;
                    case 2:
                        DispenseTimeout = DateTime.Now.AddSeconds(3);
                        if (!DispenseInProgress)
                        {
    #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                            CoinChanger_DispenseWatch();
    #pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                        }
                        break;
                    case 3:
                        if (DebugEnabled) MDBDebug?.Invoke("CoinChanger No Credit"); else MDBInformationMessageReceived?.Invoke("CoinChanger No Credit");
                        break;
                    case 4:
                        if (DebugEnabled) MDBDebug?.Invoke("Ошибка датчика в трубе монетоприемника"); else MDBError?.Invoke("Ошибка датчика в трубе монетоприемника");
                        break;
                    case 5:
                        if (DebugEnabled) MDBDebug?.Invoke("CoinChanger Double Arrival"); else MDBInformationMessageReceived?.Invoke("CoinChanger Double Arrival");
                        break;
                    case 6:
                        CoinChangerUnpluggedTimeout = DateTime.Now.AddSeconds(3);
                        if (CoinChangerPluggedStatus)
                        {
    #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                            CoinChanger_UnpluggedWatch();
    #pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                        }
                        break;
                    case 7:
                        if (DebugEnabled) MDBDebug?.Invoke("Застревание монеты в трубе монетоприемника при выдаче сдачи"); else MDBError?.Invoke("Застревание монеты в трубе монетоприемника при выдаче сдачи");
                        break;
                    case 8:
                        if (DebugEnabled) MDBDebug?.Invoke("Ошибка микропрограммы монетоприемника"); else MDBError?.Invoke("Ошибка микропрограммы монетоприемника");
                        break;
                    case 9:
                        if (DebugEnabled) MDBDebug?.Invoke("Ошибка направляющих механизмов монетоприемника"); else MDBError?.Invoke("Ошибка направляющих механизмов монетоприемника");
                        break;
                    case 10:
                        CoinChangerBusyTimeout = DateTime.Now.AddSeconds(3);
                        if (CoinChangerReadyStatus)
                        {
    #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                            CoinChanger_BusyWatch();
    #pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                        }
                        break;
                    case 11:
                        MDBCoinChangerReseted?.Invoke();
                        break;
                    case 12:
                        if (DebugEnabled) MDBDebug?.Invoke("Застревание монеты"); else MDBError?.Invoke("Застревание монеты");
                        break;
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

            }
        }

        /// <summary>
        /// обрабатываем информацию о закинутых монетках
        /// </summary>
        /// <param name="DepositCoinBytes"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessCoinChangerDepositCoin(byte[] DepositCoinBytes)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                int CoinRouting = ((DepositCoinBytes[0] & 0x30) >> 4);//bits 5-6 of byte 1
                int CoinType = (DepositCoinBytes[0] & 0x0F);//bits 1-4 of byte 1
                int CoinsLeft = DepositCoinBytes[1];//byte 2
                double CoinValue = Math.Round(CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[CoinType] * (1 / Math.Pow(10, CoinChangerSetupData.DecimalPlaces)), 2);
                switch (CoinRouting)
                {
                    case 0://"Кэшбокс";
                        if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: MDBInsertedCoinRoutedToCashBox type={1}, value={0}, total={2}", CoinValue, CoinType, CoinsLeft));
                        else
                            MDBInsertedCoinRoutedToCashBox?.Invoke(CoinValue);
                        break;
                    case 1://"Трубка монетоприемника";
                        if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: MDBInsertedCoinRoutedToCoinChangerTube type={1}, value={0}, total={2}", CoinValue, CoinType, CoinsLeft));
                        else
                            MDBInsertedCoinRoutedToCoinChangerTube?.Invoke(CoinValue);
                        break;
                    case 3://Возврат
                        if (DebugEnabled) MDBDebug?.Invoke("Монета нераспознана, возвращена"); else MDBInformationMessageReceived?.Invoke("Монета нераспознана, возвращена");
                        break;
                    case 2://неизвестно
                        if (DebugEnabled) MDBDebug?.Invoke("Какая-то неведомая хуйня с монетой"); else MDBInformationMessageReceived?.Invoke("Какая-то неведомая хуйня с монетой");
                        break;
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обрабатываем ответ от купюроприемника
        /// </summary>
        /// <param name="ResponseData"></param>
        private static void ProcessBillValidatorResponse(byte[] ResponseData)
        {
            try
            {
                if ((AwaitBillValidatorSettings) && (ResponseData.Length == 29))//Ожидаем настройки, размер данных совпадает
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessBillValidatorSettings(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                if (AwaitBillValidatorStackerStatus && (ResponseData.Length == 4))//Ожидаем статус стекера, размер данных совпадает
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessBillValidatorStackerStatus(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                if (AwaitBillValidatorID && (ResponseData.Length == BillValidatorIDData.ExpectedDataLength))//расширенная информация о купюроприемнике
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessBillValidatorExtendedData(ResponseData);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    return;
                }
                //The validator may send several of one type activity* up to 16 bytes total.
                //*Type activity is defined as Bills Accepted and Status.All may be combined in a
                //response to a POLL command providing the total number of bytes does not
                //exceed 16.
                for (int i = 1; i < ResponseData.Length - 1; i++)
                {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                    ProcessBillValidatorActivity(ResponseData[i]);
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обработка статуса купюроприемника
        /// </summary>
        /// <param name="BillValidatorStackerStatusData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessBillValidatorStackerStatus(byte[] BillValidatorStackerStatusData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
            AwaitBillValidatorStackerStatus = false;
            bool isstackerfull = ((BillValidatorStackerStatusData[1] & 0x80) == 1);
            int stackerbillscount = (BillValidatorStackerStatusData[1] << 8 | BillValidatorStackerStatusData[2]) & 0x7FFF;
            if (DebugEnabled) MDBDebug?.Invoke("DEBUG: BillValidator stacker status received"); else MDBBillValidatorStackerStatus?.Invoke(isstackerfull, stackerbillscount);
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// Обработка информации об активности купюроприемника
        /// </summary>
        /// <param name="BillValidatorActivityByte"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessBillValidatorActivity(byte BillValidatorActivityByte)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                switch ((BillValidatorActivityByte & 0x80) >> 7)
                {
                    case 1://принята купюра
                        int route = (BillValidatorActivityByte & 0x70 >> 4);//bits 5-7 of byte 1
                        double value = Math.Round(BillValidatorSetupData.BillScalingFactor * BillValidatorSetupData.BillTypeCredit[BillValidatorActivityByte & 0x0F] * (1 / Math.Pow(10, BillValidatorSetupData.DecimalPlaces)), 2);
                        if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: Bill Inserted value={0}", value)); else MDBInsertedBill?.Invoke(value);
                        break;
                    case 0://сообщение статуса
    #pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                        ProcessBillValidatorStatus(BillValidatorActivityByte);
    #pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                        break;
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// обрабатываем ответ с настройками валидатора
        /// </summary>
        /// <param name="SettingsData"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessBillValidatorSettings(byte[] SettingsData)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                AwaitBillValidatorSettings = false;
                BillValidatorSetupData.BillValidatorFeatureLevel = SettingsData[1];
                if (BillValidatorSetupData.BillValidatorFeatureLevel == 1) BillValidatorIDData.ExpectedDataLength = 31;
                if (BillValidatorSetupData.BillValidatorFeatureLevel == 2) BillValidatorIDData.ExpectedDataLength = 35;
                BillValidatorSetupData.CountryOrCurrencyCode = BCDByteToInt(new byte[2] { SettingsData[2], SettingsData[3] });
                BillValidatorSetupData.BillScalingFactor = SettingsData[4] << 8 | SettingsData[5];
                BillValidatorSetupData.DecimalPlaces = SettingsData[6];
                BillValidatorSetupData.StackerCapacity = SettingsData[7] << 8 | SettingsData[8];
                var tmpcr = new System.Collections.BitArray(SettingsData[9] << 8 | SettingsData[10]);
                for (int i = 0; i < tmpcr.Length; i++)
                {
                    BillValidatorSetupData.BillSecurityLevel[i] = tmpcr[i];
                }
                BillValidatorSetupData.Escrow = ((SettingsData[11] & 0xff) == 1);
                for (int i = 12; i < SettingsData.Length - 1; i++)
                {
                    BillValidatorSetupData.BillTypeCredit[i - 12] = SettingsData[i];
                }
                if (DebugEnabled) MDBDebug?.Invoke("DEBUG: BillValidatorSetupData received"); else MDBInformationMessageReceived?.Invoke("BillValidatorSetupData received");
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
                GetBillValidatorIdentification();
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// обработка расширенной информации от купюроприемника
        /// </summary>
        /// <param name="Data"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessBillValidatorExtendedData(byte[] Data)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                AwaitBillValidatorID = false;
                BillValidatorIDData.ManufacturerCode = Encoding.ASCII.GetString(new byte[] { Data[1], Data[2], Data[3], });
                byte[] snarray = new byte[12];
                Array.Copy(Data, 4, snarray, 0, 12);
                BillValidatorIDData.SerialNumber = Encoding.ASCII.GetString(snarray);
                byte[] modarray = new byte[12];
                Array.Copy(Data, 16, modarray, 0, 12);
                BillValidatorIDData.ModelRevision = Encoding.ASCII.GetString(modarray);
                BillValidatorIDData.SoftwareVersion = BCDByteToInt(new byte[] { Data[28], Data[29] });
                if (BillValidatorSetupData.BillValidatorFeatureLevel == 2)
                {
                    byte[] futusearray = new byte[4];
                    Array.Copy(Data, 30, futusearray, 0, 4);
                    var tmpfullflags = new System.Collections.BitArray(futusearray);
                    BillValidatorIDData.FTLSupported = tmpfullflags[0];
                    BillValidatorIDData.BillRecyclingSupported = tmpfullflags[1];
                    for (int i = 4; i < tmpfullflags.Length; i++)
                    {
                        BillValidatorIDData.FutureUseLast30Flags[i - 4] = tmpfullflags[i];
                    }
                }
                if (DebugEnabled) MDBDebug?.Invoke("DEBUG: BillValidatorIDData received"); else MDBInformationMessageReceived?.Invoke("BillValidatorIDData received");
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

            }
        }

        /// <summary>
        /// ОБработка сообщения статуса от купюроприемника
        /// </summary>
        /// <param name="StatusByte"></param>
        /// <returns></returns>
#pragma warning disable CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        private static async Task ProcessBillValidatorStatus(byte StatusByte)
#pragma warning restore CS1998 // В асинхронном методе отсутствуют операторы await, будет выполнен синхронный метод
        {
            try
            { 
                if ((StatusByte & 0x40) >> 5 == 2)// сообщение о попытках вставить купюру, пока купюроприемник был в нерабочем состоянии
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("Количество попыток вставить купюру в неактивном состоянии: {0}", StatusByte & 0x1F));
                    else
                        MDBError?.Invoke(string.Format("Количество попыток вставить купюру в неактивном состоянии: {0}", StatusByte & 0x1F));
                }
                else// Ошибка
                {
                    switch (StatusByte & 0x0F)
                    {
                        case 1:
                            if (DebugEnabled) MDBDebug?.Invoke("Ошибка двигателя купюроприемника"); else MDBError?.Invoke("Ошибка двигателя купюроприемника");
                            break;
                        case 2:
                            if (DebugEnabled) MDBDebug?.Invoke("Ошибка датчика купюроприемника"); else MDBError?.Invoke("Ошибка датчика купюроприемника");
                            break;
                        case 3:
                            BillValidatorBusyTimeout = DateTime.Now.AddSeconds(3);
                            if (BillValidatorReadyStatus)
                            {
    #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                                BillValidator_BusyWatch();
    #pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                            }
                            break;
                        case 4:
                            if (DebugEnabled) MDBDebug?.Invoke("Ошибка микропрограммы купюроприемника"); else MDBError?.Invoke("Ошибка микропрограммы купюроприемника");
                            break;
                        case 5:
                            if (DebugEnabled) MDBDebug?.Invoke("Замятие в купюроприемнике"); else MDBError?.Invoke("Замятие в купюроприемнике");
                            break;
                        case 6:
                            if (DebugEnabled) MDBDebug?.Invoke("Bill validator Reseted"); else MDBBillValidatorReseted?.Invoke();
                            break;
                        case 7:
                            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Bill Removed"); else MDBInformationMessageReceived?.Invoke("BillValidator Bill Removed");
                            break;
                        case 8:
                            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Cash Box Out of Position"); else MDBBillValidatorCashBoxRemoved?.Invoke();
                            break;
                        case 9:
                            BillValidatorDisabledTimeout = DateTime.Now.AddSeconds(3);
                            if (BillValidatorEnableStatus)
                            {
    #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                                BillValidator_DisabledWatch();
    #pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
                            }
                            break;
                        case 10:
                            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Invalid Escrow Request"); else MDBInformationMessageReceived?.Invoke("BillValidator Invalid Escrow Request");
                            break;
                        case 11:
                            if (DebugEnabled) MDBDebug?.Invoke("BillValidator Bill Rejected"); else MDBInformationMessageReceived?.Invoke("BillValidator Bill Rejected");
                            break;
                        case 12:
                            if (DebugEnabled) MDBDebug?.Invoke("Possible Credited Bill Removal"); else MDBInformationMessageReceived?.Invoke("Possible Credited Bill Removal");
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception ex)
            {
                if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBDataProcessingError?.Invoke(ex.Message);
            }
            finally
            {

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

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Возвращаем купюру из Escrow
        /// </summary>
        public static async Task ReturnBill()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.ReturnBill);
        }

        /// <summary>
        /// запрос настроек валидатора
        /// </summary>
        public static async Task GetBillValidatorSettings()
        {
            AwaitBillValidatorSettings = true;
            AddCommand(MDBCommands.BillValidatorSetup); //Request Stacker Status
            int RetryCount = 0;
            while (AwaitBillValidatorSettings && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitBillValidatorSettings = false;
        }

        /// <summary>
        /// запрос настроек монетоприемника
        /// </summary>
        public static async Task GetCoinChangerSettings()
        {
            AwaitCoinChangerSettings = true;
            AddCommand(MDBCommands.CoinChangerSetup); //Request Stacker Status
            int RetryCount = 0;
            while (AwaitCoinChangerSettings && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitCoinChangerSettings = false;
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Выдаем сдачу монетами
        /// </summary>
        /// <param name="PayOutSum"></param>
        public static async Task PayoutCoins(int PayOutSum)
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            byte[] payouttmpcmd = MDBCommands.PayoutCoins(PayOutSum);
            AddCommand(payouttmpcmd);
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Запрашиваем информацию о выданной сдаче
        /// </summary>
        public static async Task GetDispensedInfo()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.GetDispensedCoinsInfo);
        }

        /// <summary>
        /// Отсылает команды на шину MDB в "как бы полудуплексном" режиме
        /// </summary>
        /// <returns></returns>
        private static async Task SendCommandTask()
        {
            while (true)
            {
                try
                {
                    MDBCommandsListSemaphore.Wait();
                    List<byte[]> TmpCmdList = new List<byte[]>(CommandList);
                    CommandList = new List<byte[]> { };
                    MDBCommandsListSemaphore.Release();
                    for (int i = 0; i < TmpCmdList.Count; i++)
                    {
                        using (DataWriter MDBSerialDataWriteObject = new DataWriter(MDBSerialPort.OutputStream))
                        {
                            var dw = new Stopwatch();
                            dw.Start();//запускаем секундомер
                            MDBAnswerFromByteSemaphore.Wait();//ждем освобождения переменной
                            AwaitingMDBillValidatornswerFrom = TmpCmdList[i][0];//присваиваем значение адреса
                            MDBSerialDataWriteObject.WriteBytes(TmpCmdList[i]);
                            await MDBSerialDataWriteObject.StoreAsync();//пишем в порт
                            MDBAnswerFromByteSemaphore.Release();//освобождаем переменную
                            while (dw.ElapsedMilliseconds < 500)//ждем ответа от устройства 0.5с...
                            {
                                await Task.Delay(10);
                                if (AwaitingMDBillValidatornswerFrom == 0x00) break;//...либо если ответ пришел
                            }
                            dw.Stop();
                            if (DebugEnabled)
                            {
                                StringBuilder hex = new StringBuilder(TmpCmdList[i].Length * 2);
                                foreach (byte b in TmpCmdList[i]) hex.AppendFormat("{0:x2}", b);
                                MDBDebug?.Invoke(string.Format("DEBUG WRITE: {0}, Commands left: {1}", hex.ToString(), TmpCmdList.Count - (i + 1)));
                            }
                            MDBAnswerFromByteSemaphore.Wait();//ждем освобождения переменной
                            AwaitingMDBillValidatornswerFrom = 0x00;
                            MDBAnswerFromByteSemaphore.Release();//освобождаем переменную
                            MDBSerialDataWriteObject?.DetachStream();
                        }
                       await Task.Delay(100);
                    }
                }
                catch (Exception ex)
                {
                    if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG ERROR: {0}, extended: {1}", ex.Message, ex.InnerException?.Message)); else MDBError?.Invoke(ex.Message);
                }
                finally
                {
                    
                }
            }
        }


#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Добавляет элемент в список команд для отправки по шине MDB
        /// </summary>
        /// <param name="cmd"></param>
        /// <param name="dispense"></param>
        private static void AddCommand(byte[] cmd, bool dispense = false)
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            MDBCommandsListSemaphore.Wait();
            CommandList.Add(cmd);
            MDBCommandsListSemaphore.Release();
            if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: CommandList count: {0}", CommandList.Count)); else MDBInformationMessageReceived?.Invoke(CommandList.Count.ToString());
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Добавляет элементы в конец списка команд для отправки по шине MDB
        /// </summary>
        /// <param name="cmd"></param>
        /// <param name="dispense"></param>
        private static void AddCommand(byte[][] cmds, bool dispense = false)
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            MDBCommandsListSemaphore.Wait();
            CommandList.AddRange(cmds.ToList());
            MDBCommandsListSemaphore.Release();
            if (DebugEnabled) MDBDebug?.Invoke(string.Format("DEBUG: CommandList count: {0}", CommandList.Count)); else MDBInformationMessageReceived?.Invoke(CommandList.Count.ToString());
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Запрещаем прием монет
        /// </summary>
        public static async Task DisableAcceptCoins()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.DisableAcceptCoins);
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Запрещаем прием купюр
        /// </summary>
        public static async Task DisableAcceptBills()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.DisableAcceptBills);
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// разрешаем прием монет
        /// </summary>
        public static async Task EnableAcceptCoins()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.EnableAcceptCoins);
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Разрешаем выдачу монет
        /// </summary>
        public static async Task EnableDispenseCoins()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.EnableDispenseCoins);
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// разрешаем прием наличных
        /// </summary>
        public static async Task EnableCashDevices()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(new byte[][] { MDBCommands.BillValidatorSetup, MDBCommands.CoinChangerSetup, MDBCommands.EnableAcceptCoins, MDBCommands.EnableAcceptBills });
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// запрещаем прием наличных
        /// </summary>
        public static async Task DisableCashDevices()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(new byte[][] { MDBCommands.DisableAcceptCoins, MDBCommands.DisableAcceptBills });
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Разрешаем прием купюр
        /// </summary>
        public static async Task EnableAcceptBills()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(new byte[][] { MDBCommands.BillValidatorSetup, MDBCommands.EnableAcceptBills });
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Выполняет сброс монетоприемника
        /// </summary>
        public static async Task ResetCoinChanger()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.ResetCoinChanger);//Reset CoinChanger
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Выполняет сброс купюроприемника
        /// </summary>
        public static async Task ResetBillValidator()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(MDBCommands.ResetBillValidator);//Reset Bill BillValidator
        }

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
        /// <summary>
        /// Выполняет сброс купюро и монетоприемника
        /// </summary>
        public static async Task ResetCashDevices()
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously
        {
            AddCommand(new byte[][] { MDBCommands.ResetBillValidator, MDBCommands.ResetCoinChanger });//Reset bill validator and coin changer
        }

        /// <summary>
        /// Запрашиваем состояние стекера купюроприемника
        /// </summary>
        public static async Task GetBillValidatorStatus()
        {
            AwaitBillValidatorStackerStatus = true;
            AddCommand(MDBCommands.GetBillValidatorStackerStatus); //Request Stacker Status
            int RetryCount = 0;
            while (AwaitBillValidatorStackerStatus && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitBillValidatorStackerStatus = false;
        }

        /// <summary>
        /// Запрашиваем состояние монетоприемника
        /// </summary>
        public static async Task GetCoinChangerStatus()
        {
            AwaitCoinChangerTubeStatus = true;
            AddCommand(MDBCommands.GetCoinChangerTubeStatus); //Request CoinChanger Tube Status
            int RetryCount = 0;
            while (AwaitCoinChangerTubeStatus && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitCoinChangerTubeStatus = false;
        }

        /// <summary>
        /// Запрашиваем расширенную информацию
        /// </summary>
        public static async Task GetCashDevicesIdentification()
        {
            AwaitCoinChangerID = true;
            AwaitBillValidatorID = true;
            byte[][] tmpidcmds = new byte[2][]{ MDBCommands.RequestCoinChangerIdentification, (BillValidatorSetupData.BillValidatorFeatureLevel == 2) ? MDBCommands.RequestBillValidatorIdentification_Level2 : MDBCommands.RequestBillValidatorIdentification_Level1 };
            AddCommand(tmpidcmds);
            int RetryCount = 0;
            while ((AwaitBillValidatorID || AwaitCoinChangerID) && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitCoinChangerID = false;
            AwaitBillValidatorID = false;
        }

        /// <summary>
        /// Запрашиваем расширенную информацию о монетоприемнике
        /// </summary>
        public static async Task GetCoinChangerIdentification()
        {
            AwaitCoinChangerID = true;
            AddCommand(MDBCommands.RequestCoinChangerIdentification); //Request CoinChanger Expanded ID
            int RetryCount = 0;
            while (AwaitCoinChangerID && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitCoinChangerID = false;
        }

        /// <summary>
        /// Запрашиваем расширенную информацию о купюроприемнике
        /// </summary>
        public static async Task GetBillValidatorIdentification()
        {
            AwaitBillValidatorID = true;
            if (BillValidatorSetupData.BillValidatorFeatureLevel == 1) AddCommand(MDBCommands.RequestBillValidatorIdentification_Level1); else AddCommand(MDBCommands.RequestBillValidatorIdentification_Level2); //Request BillValidator Expanded ID
            int RetryCount = 0;
            while (AwaitBillValidatorID && RetryCount < 21)//таймаут ожидания ответа
            {
                await Task.Delay(100);//пауза на 0.1сек
                RetryCount++;
            }
            AwaitBillValidatorID = false;
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
            public static byte[] GetBillValidatorStackerStatus = new byte[] { 0x36, 0x36 };
            public static byte[] GetCoinChangerTubeStatus = new byte[] { 0x0A, 0x0A };
            public static byte[] GetDispensedCoinsInfo = new byte[] { 0x0F, 0x03, 0x12 };
            public static byte[] ResetBillValidator = new byte[] { 0x30, 0x30 };
            public static byte[] ResetCoinChanger = new byte[] { 0x08, 0x08 };
            public static byte[] CoinChangerSetup = new byte[] { 0x09, 0x09 };
            public static byte[] BillValidatorSetup = new byte[] { 0x31, 0x31 };
            public static byte[] ReturnBill = new byte[] { 0x35, 0x00, 0x35 };
            public static byte[] RequestCoinChangerIdentification = new byte[] { 0x0F, 0x00, 0x0F };
            public static byte[] RequestBillValidatorIdentification_Level1 = new byte[] { 0x37, 0x00, 0x37 };
            public static byte[] EnableBillValidatorFeatures_Level2 = new byte[] { 0x37, 0x01, 0x38 };
            public static byte[] RequestBillValidatorIdentification_Level2 = new byte[] { 0x37, 0x02, 0x39 };
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

using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Threading;
using System.Threading.Tasks;
using Windows.UI.Core;
using Windows.ApplicationModel.Core;
using MDBLib;

// Документацию по шаблону элемента "Пустая страница" см. по адресу https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x419

namespace MDBLib_Example
{

    /// <summary>
    /// Пустая страница, которую можно использовать саму по себе или для перехода внутри фрейма.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public static CancellationTokenSource GlobalCancellationTokenSource = new CancellationTokenSource();
        public static MainPage MainPageInstance;
        public MainPage()
        {
            this.InitializeComponent();
            MainPageInstance = this;
            Task.Run(StartMDB);
        }

        public static async Task StartMDB()
        {
            try
            {
                MDB.MDBDebug += MDB_MDBDebug;
                MDB.DebugEnabled = true;
                MDB.InitWithSerialPortExact("\\\\?\\FTDIBUS#VID_0403+PID_6001+6&38a36d0c&0&4#0000#{86e0d1e0-8089-11d0-9ce4-08003e301f73}", GlobalCancellationTokenSource.Token);
                while (MDB.MDBSerialPort == null)
                {
                    await Task.Delay(100);
                }
            }
            catch (Exception ex)
            {
                AddItemToLogBox(ex.Message);
            }
        }

        private static void MDB_MDBDebug(string DebugMessage)
        {
            AddItemToLogBox(DebugMessage);
        }

        public static void AddItemToLogBox(string logstr)
        {
#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
            CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                Mutex m = new Mutex(true, "AddLogItemsMutex", out bool mutexWasCreated);
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
                try
                {
                    string sstr = "";
                    int startpos = 0;
                    int LengthLimit = 120;
                    if (logstr.Length <= LengthLimit) sstr = logstr.Substring(startpos, logstr.Length); else sstr = logstr.Substring(startpos, LengthLimit);
                    startpos += sstr.Length;
                    LengthLimit = 140;
                    string LogString = DateTime.Now.ToString("dd.MM.yyyy HH:mm:ss.fff") + " * " + sstr;
                    while (startpos < logstr.Length || sstr.Length > 0)
                    {
                        MainPageInstance.LogBox.Items.Add(LogString);
                        if (logstr.Length - startpos <= LengthLimit) sstr = logstr.Substring(startpos); else sstr = logstr.Substring(startpos, LengthLimit);
                        startpos += sstr.Length;
                        LogString = string.Concat(">>> ", sstr);
                        Task.Delay(10).Wait();
                    }
                    MainPageInstance.LogBox.ScrollIntoView(MainPageInstance.LogBox.Items[MainPageInstance.LogBox.Items.Count - 1]);
                }
                catch
                {

                }
                finally
                {
                    m.ReleaseMutex();
                    m.Dispose();
                }
            });
#pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            MDB.EnableCashDevices();
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            MDB.DisableCashDevices();
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
#pragma warning disable CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
            MDB.GetCashDevicesIdentification();
#pragma warning restore CS4014 // Так как этот вызов не ожидается, выполнение существующего метода продолжается до завершения вызова
        }
    }
}

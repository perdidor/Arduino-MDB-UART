using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.IO.Ports;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MDB_RS232_Test
{
    public partial class Form1 : Form
    {
        public SerialPort ComPort = null;
        private static Form1 Instance = null;

        public Form1()
        {
            InitializeComponent();
            Instance = this;
        }

        private string ByteArrayToString(byte[] ba)
        {
            StringBuilder hex = new StringBuilder(ba.Length * 2);
            foreach (byte b in ba)
                hex.AppendFormat("{0:x2}", b);
            return hex.ToString();
        }

        private byte[] StringToByteArray(String hex)
        {
            int NumberChars = hex.Length;
            byte[] bytes = new byte[NumberChars / 2];
            for (int i = 0; i < NumberChars; i += 2)
                bytes[i / 2] = Convert.ToByte(hex.Substring(i, 2), 16);
            return bytes;
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            if (ComPort != null)
            {
                byte[] cmd = StringToByteArray(CmdTextBox.Text);
                ComPort.Write(cmd, 0, cmd.Length);
            }
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            SerialSettingsForm sf = null;
            if (ComPort == null) sf = new SerialSettingsForm(); else sf = new SerialSettingsForm(ComPort);
            DialogResult res = sf.ShowDialog();
            if (res == DialogResult.OK)
            {
                if (ComPort != null)
                {
                    ComPort.Close();
                }
                ComPort = new SerialPort(sf.SerialSelect.Text);
                ComPort.DataReceived += ComPort_DataReceived;
                ComPort.BaudRate = 9600;
                ComPort.Parity = Parity.None;
                ComPort.DataBits = 8;
                ComPort.StopBits = StopBits.One;
                ComPort.Open();
                ComPort.DiscardInBuffer();
            }
        }

        private void ComPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string ResponseData = ComPort.ReadLine().Replace("\r", "").Trim();
                Instance.Invoke((MethodInvoker)delegate
                {
                    ListViewItem a = answerslist.Items.Add(ResponseData);
                    answerslist.EnsureVisible(answerslist.Items.Count - 1);
                    Application.DoEvents();
                });
            }
            catch
            {

            }
        }


    }
}

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
            StringBuilder hex = new StringBuilder(ba.Length * 3);
            foreach (byte b in ba)
                hex.AppendFormat("{0:x2} ", b);
            string tmpstr = hex.ToString();
            tmpstr = tmpstr.Remove(tmpstr.Length - 1);
            return tmpstr;
        }

        private byte[] StringToByteArray()
        {
            string hex = CmdComboBox.Text.Replace(" ", "");
            int NumberChars = hex.Length;
            byte[] bytes = new byte[(int)(NumberChars / 2)];
            try
            {
                for (int i = 0; i < NumberChars; i += 2)
                    bytes[i / 2] = Convert.ToByte(hex.Substring(i, 2), 16);
            }
            catch
            {

            }
            return bytes;
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            if (ComPort != null)
            {
                byte[] cmd = StringToByteArray();
                ComPort.Write(cmd, 0, cmd.Length);
                Instance.Invoke((MethodInvoker)delegate
                {
                    ListViewItem a = answerslist.Items.Add("Sent");
                    a.SubItems.Add(ByteArrayToString(cmd));
                    a.BackColor = Color.LightBlue;
                    answerslist.EnsureVisible(answerslist.Items.Count - 1);
                    AddRecentCommand();
                });
            }
        }

        private void AddRecentCommand()
        {
            NormalizeCommandText();
            if (CmdComboBox.FindStringExact(CmdComboBox.Text) == -1)
            {
                ComboboxItem item = new ComboboxItem();
                item.Text = CmdComboBox.Text;
                item.Value = CmdComboBox.Text;
                CmdComboBox.Items.Insert(0,item);
                CmdComboBox.SelectedIndex = 0;
                SaveRecentCommands();
            }
        }

        public class ComboboxItem
        {
            public string Text { get; set; }
            public object Value { get; set; }

            public override string ToString()
            {
                return Text;
            }
        }

        private void NormalizeCommandText()
        {
            CmdComboBox.Text = ByteArrayToString(StringToByteArray());
            CmdComboBox.Focus();
            CmdComboBox.SelectionStart = CmdComboBox.Text.Length;
        }

        private bool IsCommandValid()
        {
            byte[] cmd = StringToByteArray();
            bool res = false;
            int sum = 0;
            if (CmdComboBox.Text.Replace(" ", "").Length % 2 == 1)
                res = false; else
                    try
                    {
                        for (int i = 0; i < cmd.Length - 1; i++)
                        {
                            sum += cmd[i];
                        }
                        res = ((sum & 0xff) == cmd[cmd.Length - 1]);
                    }
                    catch
                    {

                    }
            if (res)
            {
                toolStripStatusLabel1.ForeColor = Color.Green;
                toolStripStatusLabel1.Text = "Command OK";
            } else
            {
                toolStripStatusLabel1.ForeColor = Color.Red;
                toolStripStatusLabel1.Text = "Command incorrect, incomplete or empty";
            }
            return res;
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            SerialSettingsForm sf = null;
            if (ComPort == null) sf = new SerialSettingsForm(); else sf = new SerialSettingsForm(ComPort);
            DialogResult res = sf.ShowDialog();
            if (res == DialogResult.OK)
            {
                try
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
                    toolStripStatusLabel2.ForeColor = Color.Green;
                    toolStripStatusLabel2.Text = sf.SerialSelect.Text + " Ready";
                }
                catch
                {
                    toolStripStatusLabel2.ForeColor = Color.Red;
                    toolStripStatusLabel2.Text = "COM not ready";
                    MessageBox.Show("Error connecting " + sf.SerialSelect.Text);
                }
                EnableSendButton();
            }
        }

        private void ComPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string ResponseData = ComPort.ReadLine().Replace("\r", "").Trim();
                Instance.Invoke((MethodInvoker)delegate
                {
                    ListViewItem a = answerslist.Items.Add("Received");
                    a.SubItems.Add(ResponseData);
                    a.BackColor = Color.LightGreen;
                    answerslist.EnsureVisible(answerslist.Items.Count - 1);
                    Application.DoEvents();
                });
            }
            catch
            {

            }
        }

        private void CmdTextBox_TextChanged(object sender, EventArgs e)
        {
            EnableSendButton();
        }

        private void EnableSendButton()
        {
            SendButton.Enabled = (IsCommandValid() && (ComPort != null));
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            System.Reflection.Assembly a = System.Reflection.Assembly.GetExecutingAssembly();
            Version appVersion = a.GetName().Version;
            string appVersionString = appVersion.ToString();
            if (Properties.Settings.Default.PrevAppVersion != appVersion.ToString())
            {
                Properties.Settings.Default.Upgrade();
                Properties.Settings.Default.PrevAppVersion = appVersionString;
                Properties.Settings.Default.Save();
            }
            try
            {
                CmdComboBox.Items.Clear();
                foreach (string item in Properties.Settings.Default.RecentCommandList)
                {
                    if (item != "") CmdComboBox.Items.Add(item);
                }
            }
            catch
            {

            }
        }

        private void SaveRecentCommands()
        {
            try
            {
                Properties.Settings.Default.RecentCommandList?.Clear();
                foreach (var item in CmdComboBox.Items)
                {
                    Properties.Settings.Default.RecentCommandList.Add(item.ToString());
                }
                Properties.Settings.Default.Save();
            }
            catch
            {

            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            DialogResult res = MessageBox.Show("Command history will be deleted. Are you sure?", "Confirmation", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
            if (res == DialogResult.Yes)
            {
                CmdComboBox.Items.Clear();
                Properties.Settings.Default.RecentCommandList?.Clear();
                Properties.Settings.Default.Save();
            }
        }
    }
}

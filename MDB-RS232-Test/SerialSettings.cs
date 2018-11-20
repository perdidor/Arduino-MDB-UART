using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MDB_RS232_Test
{
    public partial class SerialSettingsForm : Form
    {
        public SerialSettingsForm()
        {
            InitializeComponent();
            ListPorts();
        }

        public SerialSettingsForm(SerialPort port)
        {
            InitializeComponent();
            ListPorts();
            SerialSelect.SelectedIndex = SerialSelect.Items.IndexOf(port.PortName);
        }

        private void ListPorts()
        {
            SerialSelect.Items.Clear();
            SerialSelect.Items.Add("Select...");
            SerialSelect.SelectedIndex = 0;
            SerialSelect.Items.AddRange(SerialPort.GetPortNames());
        }

        private void OKButton_Click(object sender, EventArgs e)
        {

        }

        private void SerialSelect_SelectedIndexChanged(object sender, EventArgs e)
        {
            OKButton.Enabled = (SerialSelect.SelectedIndex > 0);
        }
    }
}

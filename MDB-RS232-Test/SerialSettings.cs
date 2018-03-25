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
            SerialSelect.Items.AddRange(SerialPort.GetPortNames());
        }

    }
}

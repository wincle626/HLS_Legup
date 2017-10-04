using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Mips_Communication_Server
{
    public partial class MainForm : Form
    {
        LogWindow log;
        NetworkController network = null;
        bool closing = false;
        private BoardDevice DebugDevice = null;

        private delegate void ConnectionNotify();
        
        public MainForm()
        {
            InitializeComponent();

            DebugDevice = new BoardDevice();
            network = new NetworkController(DebugDevice);

            network.Error += delegate(object s)
            {
                ConnectionNotify ErrorFunc = ConnectionError;
                BeginInvoke(ErrorFunc);
            };

            network.ConnectionSuccess += delegate(object s)
            {
                ConnectionNotify SuccessFunc = ConnectionSucceeded;
                BeginInvoke(SuccessFunc);
            };

            log = new LogWindow();
            log.Show();
            log.Hide();

            PopulateCableDropdown();
        }

        public void ConnectionError()
        {
            MessageBox.Show("Disconnected, see log window for details", "Connection Failed", MessageBoxButtons.OK,
                MessageBoxIcon.Error);

            DoDisconnect();
        }

        public void ConnectionSucceeded()
        {
            FinishConnect();
        }

        private void PopulateCableDropdown()
        {
            try
            {
                JTAGClient.AJI_HARDWARE[] hardware = JTAGClient.GetHardware();

                cableDropdown.Items.Clear();
                foreach (JTAGClient.AJI_HARDWARE h in hardware)
                {
                    cableDropdown.Items.Add(h);
                }
            }
            catch (JTAGClientException e)
            {
                MessageBox.Show(String.Format("Error getting JTAG cables: {0}", e.Message), "JTAG Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void logButton_Click(object sender, EventArgs e)
        {
            log.Show();
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            if (WindowState == FormWindowState.Minimized)
                Hide();
        }

        private void notifyIcon1_DoubleClick(object sender, EventArgs e)
        {
            Show();
            WindowState = FormWindowState.Normal;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (closing)
                return;
            
            e.Cancel = true;
            WindowState = FormWindowState.Minimized;
            Hide();
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            closing = true;
            DoDisconnect();
            Close();
        }

        private void connectButton_Click(object sender, EventArgs e)
        {
            try
            {
                object selectedHardwareItem = cableDropdown.SelectedItem;

                if (selectedHardwareItem == null)
                    return;

                if (deviceList.SelectedIndex == -1)
                    return;

                JTAGClient.AJI_HARDWARE selectedHardware = (JTAGClient.AJI_HARDWARE)selectedHardwareItem;

                DebugDevice.Connect(string.Format("{0} [{1}]", selectedHardware.cableName, selectedHardware.cablePort), deviceList.SelectedIndex, 1);
                network.Connect();

                connectButton.Enabled = false;
            }
            catch (JTAGException ex)
            {
                MessageBox.Show(String.Format("Could not connect to JTAG, error was: {0}", ex.Message), "Connection Error", 
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void disconnectButton_Click(object sender, EventArgs e)
        {
            DoDisconnect();
        }

        private void DoDisconnect()
        {
            network.Disconnect();
            DebugDevice.Disconnect();

            connectLabel.Text = "Not Connected";

            connectButton.Enabled = true;
            disconnectButton.Enabled = false;
        }

        private void FinishConnect()
        {
            JTAGClient.AJI_HARDWARE selectedHardware = (JTAGClient.AJI_HARDWARE)cableDropdown.SelectedItem;
            connectLabel.Text = string.Format("Connected to {0} on {1}", deviceList.SelectedItem, selectedHardware);

            disconnectButton.Enabled = true;
        }

        private void cableDropdown_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                JTAGClient.AJI_HARDWARE selectedHardware = (JTAGClient.AJI_HARDWARE)cableDropdown.SelectedItem;

                JTAGClient.AJI_DEVICE[] devices = JTAGClient.GetDevices(selectedHardware.chain);

                deviceList.Items.Clear();
                foreach (JTAGClient.AJI_DEVICE d in devices)
                {
                    deviceList.Items.Add(string.Format("{0} (0x{1,8:X8})", d.deviceName, d.deviceID));
                }
            }
            catch (JTAGClientException ex)
            {
                MessageBox.Show(String.Format("Error getting devices for cable. {0}", ex.Message), "JTAG Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
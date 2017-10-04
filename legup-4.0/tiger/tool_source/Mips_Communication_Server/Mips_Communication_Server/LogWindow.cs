using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace Mips_Communication_Server
{
    public partial class LogWindow : Form
    {
        public delegate void LogTextWrite();

        private TextWriter OldOut;
        
        public LogWindow()
        {
            InitializeComponent();

            NotifyTextWriter NewOut = new NotifyTextWriter();
            NewOut.TextWritten += new TextWriteHandler(NewLogText);

            OldOut = System.Console.Out;

            System.Console.SetOut(TextWriter.Synchronized(NewOut));
        }

        private void NewLogText(object sender, string str)
        {
            LogTextWrite Writer = new LogTextWrite(delegate() { logBox.AppendText(str); });

            logBox.BeginInvoke(Writer);
        }

        private void LogWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            LogWindow log = (LogWindow)sender;
            log.Hide();
        }

        private void LogWindow_FormClosed(object sender, FormClosedEventArgs e)
        {
            System.Console.SetOut(OldOut);
        }
    }
}
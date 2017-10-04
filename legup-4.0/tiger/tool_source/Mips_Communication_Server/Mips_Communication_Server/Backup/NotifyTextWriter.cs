using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Mips_Communication_Server
{
    public delegate void TextWriteHandler(object sender, string str);
    
    class NotifyTextWriter : TextWriter
    {
        public event TextWriteHandler TextWritten;

        protected virtual void OnStringWrite(string str)
        {
            if(TextWritten != null)
                TextWritten(this, str);
        }

        public override Encoding Encoding
        {
            get { return Encoding.Default; }
        }

        public override void WriteLine()
        {
            Write(base.NewLine);
        }

        public override void WriteLine(string str)
        {
            Write(str);
            Write(base.NewLine);
        }

        public override void WriteLine(string format, params object[] args)
        {
            base.WriteLine(String.Format(format, args));
        }

        public override void Write(string str)
        {
            OnStringWrite(str.Replace("\n", base.NewLine));
        }

        public override void Write(string format, params object[] args)
        {
            base.Write(String.Format(format, args));
       } 
    }
}

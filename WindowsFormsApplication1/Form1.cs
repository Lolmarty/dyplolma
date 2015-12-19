using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Diagnostics;
using System.IO;
using System.Threading;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace gui
{
    public partial class Form1 : Form
    {
        delegate void SetTextCallback(string text);

        private string SIGTERM = "SIGTERM";
        private string SMOKE_ALARM = "SMOKE_ALARM";
        private string EOF = "EOF";
        private string ExeName = "engine.exe";
        private string Server = "localhost";
        private int Port = 27015;
        private int timeout = 1000;
        private TcpClient client;
        private NetworkStream stream;

        public Form1()
        {
            InitializeComponent();

            engineInitializer.RunWorkerAsync();
            try
            {
                client = new TcpClient(Server, Port);
                stream = client.GetStream();
                SetStatus("Started successfully");
            }
            catch (ArgumentNullException ex)
            {
                SetStatus(ex.Message);
            }
            catch (SocketException ex)
            {
                SetStatus(ex.Message);
            }
        }

        private void engineInitializer_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker bw = sender as BackgroundWorker;

            engineStartup();

            // If the operation was canceled by the user, 
            // set the DoWorkEventArgs.Cancel property to true.
            if (bw.CancellationPending)
            {
                e.Cancel = true;
            }
        }

        private void engineStartup()
        {
            Process engine = new Process();
            engine.StartInfo.FileName = ExeName;
            //engine.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            //engine.StartInfo.CreateNoWindow = true;
            //engine.StartInfo.UseShellExecute = false;
            engine.Start();
        }

        private void btCompute_Click(object sender, EventArgs e)
        {
            SetStatus("Working...");
            engineInformer.RunWorkerAsync();
        }

        private void btFileSelect_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofdLoadingDialog = new OpenFileDialog();
            ofdLoadingDialog.InitialDirectory = Environment.CurrentDirectory;
            if (ofdLoadingDialog.ShowDialog() == DialogResult.OK)
            {
                tbPath.Text = ofdLoadingDialog.FileName.ToString();
                btCompute.Enabled = true;
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Byte[] data = Encoding.ASCII.GetBytes(SIGTERM);
            stream.Write(data, 0, data.Length);
            stream.Close();
            client.Close();
        }

        private void engineInformer_DoWork(object sender, DoWorkEventArgs e)
        {
            string Message;

            ParamType parameters = new ParamType();

            parameters.path = tbPath.Text;

            if (rbLKmethod.Checked)
            {
                parameters.flow_calc = 0;
            }
            if (rbFBmethod.Checked)
            {
                parameters.flow_calc = 1;
            }
            if (rbDMmethod.Checked)
            {
                parameters.flow_calc = 2;
            }

            if (rbMOG.Checked)
            {
                parameters.bg_sub = 0;
            }
            if (rbMOG2.Checked)
            {
                parameters.bg_sub = 1;
            }

            //JObject obj = new JObject(parameters);
            JsonSerializer serializer = new JsonSerializer();
            TextWriter writer = new StringWriter();
            serializer.Serialize(writer, parameters);
            Message = writer.ToString(); //obj.ToString();
            try
            {
                // Translate the passed message into ASCII and store it as a Byte array.
                Byte[] data = Encoding.ASCII.GetBytes(Message);

                // Send the message to the connected TcpServer. 
                stream.Write(data, 0, data.Length);

                //Console.WriteLine("Sent: {0}", Message);

                engineResponce.RunWorkerAsync();
                
            }
            catch (SocketException ex)
            {
                SetStatus(ex.Message);
            }
            catch (ArgumentNullException ex)
            {
                SetStatus(ex.Message);
            }
            catch(IOException ex)
            {
                SetStatus(ex.Message);
            }
            writer.Dispose();
        }

        private void SetStatus(string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (lbStatus.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(SetStatus);
                Invoke(d, new object[] { text });
            }
            else
            {
                lbStatus.Text = text;
            }
        }

        private void engineResponce_DoWork(object sender, DoWorkEventArgs e)
        {
            String responseData = String.Empty;
            do
            {
                Byte[] data = new Byte[1024];

                // Read the first batch of the TcpServer response bytes.
                Thread.Sleep(timeout);
                Int32 bytes = 0;
                try
                {
                    bytes = stream.Read(data, 0, data.Length);
                }
                catch (SocketException ex)
                {
                    SetStatus(ex.Message);
                }
                catch (ArgumentNullException ex)
                {
                    SetStatus(ex.Message);
                }
                catch(IOException ex)
                {
                    SetStatus(ex.Message);
                }
                responseData = Encoding.ASCII.GetString(data, 0, bytes);
                Console.WriteLine("Received: {0}", responseData);
                if (responseData.Equals(SMOKE_ALARM)) SetStatus("Smoke Detected");
            } while (!responseData.Contains(EOF));
        }
    }

    class ParamType
    {
        public string path;
        public int bg_sub;
        public int flow_calc;
        public ParamType() { }
    }
}

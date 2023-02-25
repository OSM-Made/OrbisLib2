using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace OrbisLib2.Targets
{
    public class Payload
    {
        private Target Target;

        public Payload(Target Target)
        {
            this.Target = Target;
        }

        /// <summary>
        /// Sends Custom Payloads to Playstation 4 Console
        /// </summary>
        /// <param name="IP">PlayStation 4 IP address</param>
        /// <param name="PayloadBuffer">Byte array of payload</param>
        /// <param name="Port">Port used to recieve payload default value is 9020</param>
        public bool InjectPayload(byte[] PayloadBuffer)
        {
            try
            {
                Socket socket;

                socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                socket.ReceiveTimeout = 1000;
                socket.SendTimeout = 1000;
                IAsyncResult result = socket.BeginConnect(Target.IPAddress, Target.PayloadPort, null, null);

                result.AsyncWaitHandle.WaitOne(3000, true);

                if (!socket.Connected)
                {
                    Console.WriteLine("Failed to connect to socket.");

                    socket.Close();
                    return false;
                }

                // we have connected
                socket.EndConnect(result);

                //Send Payload
                socket.Send(PayloadBuffer);

                socket.Close();

                return true;
            }
            catch
            {
                Console.WriteLine("Failed to load Payload");
                return false;
            }
        }
    }
}

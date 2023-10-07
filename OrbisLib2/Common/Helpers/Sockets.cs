using System.Collections;
using System.Data;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Text;

namespace OrbisLib2.Common.Helpers
{
    public static class Sockets
    {
        /// <summary>
        /// Recieve large amounts of data from a socket that is larger than the recieve buffer size.
        /// </summary>
        /// <param name="s">The current socket.</param>
        /// <param name="data">The data to be recieved.</param>
        public static int RecvLarge(this Socket s, byte[] data)
        {
            int Left = data.Length;
            int Received = 0;

            try
            {
                while (Left > 0)
                {
                    var chunkSize = Math.Min(8192, Left);
                    var res = s.Receive(data, Received, chunkSize, 0);

                    Received += res;
                    Left -= res;
                }
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

            return Received;
        }

        public static void SendLarge(this Socket s, byte[] data)
        {
            try
            {
                int Left = data.Length;
                int CurrentPosition = 0;

                while (Left > 0)
                {
                    var chunkSize = Math.Min(8192, Left);
                    var res = s.Send(data, CurrentPosition, chunkSize, 0);

                    Left -= res;
                    CurrentPosition += res;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        /// <summary>
        /// Sends an int32 over socket.
        /// </summary>
        /// <param name="s"></param>
        /// <param name="Data"></param>
        public static void SendInt32(this Socket s, int Data)
        {
            s.Send(BitConverter.GetBytes(Data));
        }


        /// <summary>
        /// Receives an int32 over sockets.
        /// </summary>
        /// <param name="s"></param>
        /// <returns></returns>
        public static int RecvInt32(this Socket s)
        {
            var Data = new byte[sizeof(int)];
            s.Receive(Data);
            return BitConverter.ToInt32(Data, 0);
        }

        /// <summary>
        /// Attempts to ping a host.
        /// </summary>
        /// <param name="Host">Host Address</param>
        /// <returns></returns>
        public static bool PingHost(string Host)
        {
            try
            {
                var pingSender = new Ping();
                var options = new PingOptions();
                options.DontFragment = true;

                var reply = pingSender.Send(Host, 120, Encoding.ASCII.GetBytes("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), options);
                return (reply.Status == IPStatus.Success);
            }
            catch
            {

            }

            return false;
        }

        /// <summary>
        /// Tests the availability of a tcp host.
        /// </summary>
        /// <param name="Host">Host Address.</param>
        /// <param name="Port">Host Port.</param>
        /// <returns></returns>
        public static bool TestTcpConnection(string Host, int Port)
        {
            try
            {
                var client = new TcpClient();
                var result = client.BeginConnect(Host, Port, null, null);

                var success = result.AsyncWaitHandle.WaitOne(TimeSpan.FromSeconds(1));

                if (!success)
                {
                    client.Close(); // Close the TcpClient if the connection attempt fails.
                    return false;
                }

                client.EndConnect(result); // Complete the connection.

                // Get the network stream for sending data.
                NetworkStream stream = client.GetStream();

                // Send the byte array over the network stream.
                byte[] byteArray = BitConverter.GetBytes(0xFEED);
                stream.Write(byteArray, 0, byteArray.Length);

                // Close the network stream and client when done.
                stream.Close();
                client.Close();

                return true;
            }
            catch
            {
                
            }

            return false;
        }

        /// <summary>
        /// Easily connect to a socket and handle the time out.
        /// </summary>
        /// <param name="s"></param>
        /// <param name="IPAddress">The address we would like to connect to.</param>
        /// <param name="Port">The port of the socket we would like to connect to.</param>
        /// <param name="TimeOut">The time we would like to wait for connection.</param>
        /// <returns></returns>
        public static bool EasyConnect(this Socket s, string IPAddress, int Port, int TimeOut)
        {
            s.ReceiveTimeout = s.SendTimeout = TimeOut * 1000;
            var result = s.BeginConnect(IPAddress, Port, null, null);

            result.AsyncWaitHandle.WaitOne(3000, true);

            if (!s.Connected)
            {
                Console.WriteLine("Failed to connect to socket.");

                s.Close();
                return false;
            }

            // we have connected
            s.EndConnect(result);

            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="s"></param>
        /// <returns></returns>
        public static byte[] ReceiveSize(this Socket s)
        {
            // First we get the size of the request packet.
            var packetSize = s.RecvInt32();

            // Allocate space and recieve the data.
            var packet = new byte[packetSize];
            s.RecvLarge(packet);

            // return the result
            return packet;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="s"></param>
        /// <param name="data"></param>
        public static void SendSize(this Socket s, byte[] data)
        {
            // Send the size.
            s.SendInt32(data.Length);

            // Send the data now.
            s.SendLarge(data);
        }
    }
}

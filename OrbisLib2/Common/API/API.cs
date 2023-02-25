using OrbisLib2.Common.Database;
using OrbisLib2.Common.Helpers;
using OrbisLib2.Targets;
using System.Net.Sockets;
using System.Runtime.InteropServices;

namespace OrbisLib2.Common.API
{
    public static class API
    {
        /// <summary>
        /// Connects to the api.
        /// </summary>
        /// <param name="IPAddress">IP Address of the remote target.</param>
        /// <param name="Port">The port of the remote target.</param>
        /// <param name="TimeOut">The time we should wait before timing out represented as seconds.</param>
        /// <param name="Sock">The socket created when connecting.</param>
        /// <returns>Returns true if successful.</returns>
        private static bool Connect(string IPAddress, int Port, int TimeOut, out Socket Sock)
        {
            Sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            return Sock.EasyConnect(IPAddress, Port, TimeOut);
        }

        /// <summary>
        /// Makes an API call to the remote target.
        /// </summary>
        /// <param name="DesiredTarget">The desired target to recieve the command.</param>
        /// <param name="TimeOut">The time the socket should wait before timing out represented as seconds.</param>
        /// <param name="Command">The command to be run.</param>
        /// <param name="AdditionalCommunications">Optional lambda to send/recv additional data.</param>
        /// <returns>Returns result of the communications with the API.</returns>
        public static APIResults SendCommand(Target DesiredTarget, int TimeOut, APICommands Command, Action<Socket, APIResults>? AdditionalCommunications = null)
        {
            if(DesiredTarget.Info.IsAPIAvailable == false)
            {
                return APIResults.API_ERROR_COULDNT_CONNECT;
            }

            try 
            {
                if (Connect(DesiredTarget.IPAddress, Settings.CreateInstance().APIPort, TimeOut, out Socket Sock))
                {
                    // Send Inital Packet.
                    var result = SendNextPacket(Sock, new APIPacket() { PacketMagic="ORBIS_SUITE", PacketVersion = Config.PacketVersion, Command = Command });

                    // Call lambda for additional calls.
                    if (result == APIResults.API_OK && AdditionalCommunications != null)
                    {
                        AdditionalCommunications.Invoke(Sock, result);
                    }

                    // Clean up.
                    Sock.Close();

                    return result;
                }
                else
                    return APIResults.API_ERROR_COULDNT_CONNECT;
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"SendCommand() Sock Error: {ex.Message}");
                return APIResults.API_ERROR_COULDNT_CONNECT;
            }
        }

        /// <summary>
        /// Sends additional data if required.
        /// </summary>
        /// <typeparam name="T">The packet type.</typeparam>
        /// <param name="Sock">The socket instance were using.</param>
        /// <param name="Packet">Any Packet structure.</param>
        /// <returns>Returns the result of the action.</returns>
        public static APIResults SendNextPacket<T>(Socket Sock, T Packet)
        {
            // Send Next Packet.
            Sock.Send(Helper.StructToBytes(Packet));

            // Get API Response.
            return (APIResults)Sock.RecvInt32();
        }

        /// <summary>
        /// Recieves the next packet.
        /// </summary>
        /// <typeparam name="T">The packet type.</typeparam>
        /// <param name="Sock">Socket to recieve the packet on.</param>
        /// <param name="Packet">The packet to be recieved on.</param>
        /// <returns>Returns true if successful.</returns>
        public static bool RecieveNextPacket<T>(Socket Sock, ref T Packet)
        {
            try
            {
                var RawPacket = new byte[Marshal.SizeOf(Packet)];
                var bytes = Sock.Receive(RawPacket);

                if (bytes <= 0)
                    return false;

                // Convert the recieved bytes to a struct.
                Helper.BytesToStruct(RawPacket, ref Packet);

                return true;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Sock"></param>
        /// <param name="Value"></param>
        /// <returns></returns>
        public static APIResults SendInt32(Socket Sock, int Value)
        {
            try
            {
                // Send Next Packet.
                Sock.Send(BitConverter.GetBytes(Value));

                // Get API Response.
                return (APIResults)Sock.RecvInt32();
            }
            catch
            {

            }

            return APIResults.API_ERROR_GENERAL;
        }
    }
}

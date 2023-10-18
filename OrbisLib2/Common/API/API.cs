using Google.Protobuf;
using OrbisLib2.Common.Database;
using OrbisLib2.Common.Helpers;
using OrbisLib2.Targets;
using System.Net.Sockets;

namespace OrbisLib2.Common.API
{
    public static class API
    {
        private static readonly uint MagicNumber = 0xDEADBEEF;
        private static readonly int PacketVersion = 5;

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
        public static ResultState SendCommand(Target DesiredTarget, int TimeOut, APICommand Command, Func<Socket, ResultState>? AdditionalCommunications = null)
        {
            // If the API isnt up were just giving up here.
            if(DesiredTarget.Info.IsAPIAvailable == false)
                return new ResultState { Succeeded = false, ErrorMessage = $"The API is not available on the selected target {DesiredTarget.Name} ({DesiredTarget.IPAddress})." };

            try 
            {
                if (Connect(DesiredTarget.IPAddress, Settings.CreateInstance().APIPort, TimeOut, out Socket Sock))
                {
                    // Send the Magic Number.
                    Sock.Send(BitConverter.GetBytes(MagicNumber));

                    // Make sure the target is happy and ready to move on.
                    if (Sock.RecvInt32() != 1)
                        return new ResultState { Succeeded = false, ErrorMessage = $"The target {DesiredTarget.Name} ({DesiredTarget.IPAddress}) has rejected our initial communications." };

                    // Send the Initial Packet.
                    var initialResult = SendNextPacket(Sock, new InitialPacket { Command = (int)Command, PacketVersion = PacketVersion });

                    // Check to see if we failed here and report back the message.
                    if (!initialResult.Succeeded)
                        return initialResult;

                    // Set up the default respose.
                    var result = new ResultState { Succeeded = true, ErrorMessage = string.Empty };

                    // See if we have extra work to do.
                    if (AdditionalCommunications != null)
                        result = AdditionalCommunications.Invoke(Sock);

                    // Were done here, Clean up.
                    Sock.Close();

                    // Return either the default response or the edited response from the additional communications.
                    return result;
                }
                else
                    return new ResultState { Succeeded = false, ErrorMessage = $"Failed to connect to the target {DesiredTarget.Name} ({DesiredTarget.IPAddress})." };
            }
            catch (SocketException ex)
            {
                return new ResultState { Succeeded = false, ErrorMessage = $"Failed with the error: {ex.Message}" };
            }
        }

        /// <summary>
        /// Gets the result state of the API.
        /// </summary>
        /// <param name="s">The socket open to the API.</param>
        /// <returns>The result state.</returns>
        public static ResultState GetState(Socket s)
        {
            // Recieve the result state.
            var rawResult = s.ReceiveSize();

            // Return the parsed state.
            return ResultState.Parser.ParseFrom(rawResult);
        }

        /// <summary>
        /// Sends the next protobuf packet.
        /// </summary>
        /// <param name="s">The socket to send the proto packet on.</param>
        /// <param name="Packet">The packet that contains the data.</param>
        /// <returns>The result state of the packet request.</returns>
        public static ResultState SendNextPacket(Socket s, IMessage Packet)
        {
            // Send the packet.
            s.SendSize(Packet.ToByteArray());

            // Return the result state.
            return GetState(s);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Sock"></param>
        /// <param name="Value"></param>
        /// <returns></returns>
        public static ResultState SendInt32(Socket Sock, int Value)
        {
            try
            {
                // Send Next Packet.
                Sock.Send(BitConverter.GetBytes(Value));

                // Return the parsed state.
                return GetState(Sock);
            }
            catch (SocketException ex)
            {
                return new ResultState { Succeeded = false, ErrorMessage = $"Failed to send int. {ex.Message}" };
            }
        }
    }
}

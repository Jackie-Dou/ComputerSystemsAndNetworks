namespace SimpleFtpServer
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
    using System.Net;
    using System.Net.Sockets;
    using System.Text;

    /// <summary>
    /// Used for connection of the client to server.
    /// </summary>
    public class ClientConnection
    {
        #region Fields

        /// <summary>
        /// Used to transfer commands.
        /// </summary>
        private readonly TcpClient controlClient;

        /// <summary>
        /// Used to transfer files.
        /// </summary>
        private TcpClient dataClient;

        /// <summary>
        /// Used for "Control" connection (e.g. to send commands to the server and for the server to send responses back to the client).
        /// </summary>
        private NetworkStream controlStream;

        /// <summary>
        /// Reader for "Control" connection (e.g. reads stream from client).
        /// </summary>
        private readonly StreamReader controlReader;

        /// <summary>
        /// Writer for "Control" connection (e.g. writes stream to client).
        /// </summary>
        private readonly StreamWriter controlWriter;

        /// <summary>
        /// Used to receive data connection in passive mode.
        /// </summary>
        private TcpListener passiveListener;

        /// <summary>
        /// Type of transferring data.
        /// </summary>
        private TransferType connectionType = TransferType.Ascii;

        /// <summary>
        /// Format control for TransferType.
        /// </summary>
        private FormatControlType formatControlType;

        /// <summary>
        /// Type of data connection (passive/active).
        /// </summary>
        private DataConnectionType dataConnectionType = DataConnectionType.Active;

        private string root = @"F:\";
        private string currentDirectory = @"F:\";


        /// <summary>
        /// Used for data connection.
        /// </summary>
        private IPEndPoint dataEndpoint;

        private string username;

        #endregion

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ClientConnection"/> class.
        /// </summary>
        /// <param name="client">The client that will be connected to the server.</param>
        public ClientConnection(TcpClient client)
        {
            this.controlClient = client;

            this.controlStream = this.controlClient.GetStream();

            this.controlReader = new StreamReader(this.controlStream);
            this.controlWriter = new StreamWriter(this.controlStream);
        }

        #endregion

        #region Enums

        private enum TransferType
        {
            Ascii,
            Ebcdic,
            Image,
            Local,
        }

        private enum FormatControlType
        {
            NonPrint,
            Telnet,
            CarriageControl,
        }

        private enum DataConnectionType
        {
            Passive,
            Active,
        }

        #endregion

        #region Copy Stream Implementations

        private static long CopyStream(Stream input, Stream output, int bufferSize)
        {
            byte[] buffer = new byte[bufferSize];
            int count;
            long total = 0;

            while ((count = input.Read(buffer, 0, buffer.Length)) > 0)
            {
                output.Write(buffer, 0, count);
                total += count;
            }

            return total;
        }

        private static long CopyStreamAscii(Stream input, Stream output, int bufferSize)
        {
            char[] buffer = new char[bufferSize];
            long total = 0;

            using (StreamReader rdr = new StreamReader(input, Encoding.ASCII))
            {
                using (StreamWriter wtr = new StreamWriter(output, Encoding.ASCII))
                {
                    int count;

                    while ((count = rdr.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        wtr.Write(buffer, 0, count);
                        total += count;
                    }
                }
            }

            return total;
        }

        private long CopyStream(Stream input, Stream output)
        {
            Stream limitedStream = output;

            if (this.connectionType == TransferType.Image)
            {
                return CopyStream(input, limitedStream, 4096);
            }
            else
            {
                return CopyStreamAscii(input, limitedStream, 4096);
            }
        }

        #endregion

        string[] UserList = { "katrin", "login" };
        string[] Passwords = { "123", "password" };
        public bool EnterClientName = false;
        public bool ClientStatus = false;
        public int UserNum = -1;

        public void HandleClient(object obj)
        {
            this.controlWriter.WriteLine("220 Service Ready.");

            // Sends to client.
            this.controlWriter.Flush();

            try
            {
                string line;

                while (!string.IsNullOrEmpty(line = this.controlReader.ReadLine()))
                {
                    string response = null;

                    // Splitting line received from client.
                    string[] command = line.Split(' ');

                    // Command is the first "word" in line.
                    // Coverts to UpperCase according to specification: casing doesn't matter for commands.
                    string cmd = command[0].ToUpperInvariant();

                    // Agruments - another "word(s)"
                    string arguments = command.Length > 1 ? line.Substring(command[0].Length + 1) : null;

                    if (string.IsNullOrWhiteSpace(arguments))
                    {
                        arguments = null;
                    }

                    switch (cmd)
                    {
                        case "USER":
                            if (User(arguments) == true)
                            {
                                //331 Username ok, need password
                                response = $"331 USER {arguments}";
                                EnterClientName = true;
                            }
                            else {
                                response = "Incorrect username";
                            }
                            break;
                        case "PASS":
                            if (Password(arguments) == true && EnterClientName==true)
                            {
                                ClientStatus = true;
                                response = $"230 PASS {arguments}";
                            }
                            else
                            {
                                if (EnterClientName == true)
                                    response = "Incorrect password";
                                else
                                    response = "Enter login";
                            }
                            break;
                        case "CWD":
                            if (ClientStatus==true)
                                response = this.ChangeWorkingDirectory(arguments);
                            else
                                response = "User is not logged in";
                            break;
                        case "CDUP":
                            if (ClientStatus == true)
                                response = this.ChangeWorkingDirectory("..");
                            else
                                response = "User is not logged in";
                            break;
                        case "QUIT":
                            if (ClientStatus == true)
                                response = "221 Service closing control connection";
                            else
                                response = "User is not logged in";
                            break;
                        case "PORT":
                            if (ClientStatus == true) 
                                response = this.Port(arguments);
                            else
                                response = "User is not logged in";
                            break;
                      /*  case "PASV":
                            if (ClientStatus == true)
                                response = this.Passive();
                            else
                                response = "User is not logged in";
                            break; */
                        case "TYPE":
                            if (ClientStatus == true)
                            {
                                string[] splitArgs = arguments.Split(' ');
                                response = this.Type(splitArgs[0], splitArgs.Length > 1 ? splitArgs[1] : null);
                            }
                            else
                                response = "User is not logged in";
                            break;
                        case "PWD":
                            if (ClientStatus == true)
                                response = this.PrintWorkingDirectory();
                            else
                                response = "User is not logged in";
                            break;
                        case "RETR":
                            if (ClientStatus == true)
                                response = this.Retrieve(arguments);
                            else
                                response = "User is not logged in";
                            break;
                        case "LIST":
                            if (ClientStatus == true)
                                response = this.List(arguments);
                            else
                                response = "User is not logged in";
                            break;
                        default:
                            response = "502 Command not implemented";
                            break;
                    }

                    Console.WriteLine(response);

                    if (this.controlClient == null || !this.controlClient.Connected)
                    {
                        break;
                    }
                    else
                    {
                        this.controlWriter.WriteLine(response);
                        this.controlWriter.Flush();

                        if (response.StartsWith("221"))
                        {
                            break;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        private string PrintWorkingDirectory()
        {
            string current = this.currentDirectory.Replace(this.root, string.Empty);

            if (current.Length == 0)
            {
                current = "/";
            }

            return $"257 \"{current}\" is current directory.";
        }

        private bool IsPathValid(string path)
        {
            return path.StartsWith(this.root);
        }

        private string NormalizeFilename(string path)
        {
            if (path == null)
            {
                path = string.Empty;
            }

            if (path == "/")
            {
                return this.root;
            }
            else if (path.StartsWith("/"))
            {
                path = new FileInfo(Path.Combine(this.root, path.Substring(1))).FullName;
            }
            else
            {
                path = new FileInfo(Path.Combine(this.currentDirectory, path)).FullName;
            }

            return this.IsPathValid(path) ? path : null;
        }

        public bool User(string user)
        {
            for (int i = 0; i < UserList.Length; i++)
                if (UserList[i] == user)
                {
                    UserNum = i;
                    this.username = user;
                    return true;
                }
            return false;
        }

        private bool Password(string password)
        {
            if (Passwords[UserNum] == password)
            {
                return true;
            }
            return false;
        }

        private string ChangeWorkingDirectory(string pathname)
        {
            if (pathname == "/")
            {
                this.currentDirectory = this.root;
            }
            else
            {
                string newDir;

                if (pathname.StartsWith("/"))
                {
                    pathname = pathname.Substring(1);
                    newDir = Path.Combine(this.root, pathname);
                }
                else
                {
                    newDir = Path.Combine(this.currentDirectory, pathname);
                }

                if (Directory.Exists(newDir))
                {
                    this.currentDirectory = new DirectoryInfo(newDir).FullName;

                    if (!this.IsPathValid(this.currentDirectory))
                    {
                        this.currentDirectory = this.root;
                    }
                }
                else
                {
                    this.currentDirectory = this.root;
                }
            }

            return "250 Changed to new directory";
        }

        private string Port(string hostPort)
        {
            this.dataConnectionType = DataConnectionType.Active;

            string[] ipAndPort = hostPort.Split(',');

            byte[] ipAddress = new byte[4];
            byte[] port = new byte[2];

            for (int i = 0; i < 4; i++)
            {
                ipAddress[i] = Convert.ToByte(ipAndPort[i]);
            }

            for (int i = 4; i < 6; i++)
            {
                port[i - 4] = Convert.ToByte(ipAndPort[i]);
            }

            if (BitConverter.IsLittleEndian)
            {
                Array.Reverse(port);
            }

            this.dataEndpoint = new IPEndPoint(new IPAddress(ipAddress), BitConverter.ToInt16(port, 0));

            return "200 Data Connection Established";
        }

        private string Passive()
        {
            this.dataConnectionType = DataConnectionType.Passive;

            IPAddress localIp = ((IPEndPoint)this.controlClient.Client.LocalEndPoint).Address;

            this.passiveListener = new TcpListener(localIp, 0);
            this.passiveListener.Start();

            IPEndPoint passiveListenerEndpoint = (IPEndPoint)this.passiveListener.LocalEndpoint;

            byte[] address = passiveListenerEndpoint.Address.GetAddressBytes();
            short port = (short)passiveListenerEndpoint.Port;

            byte[] portArray = BitConverter.GetBytes(port);

            if (BitConverter.IsLittleEndian)
            {
                Array.Reverse(portArray);
            }

            return $"227 Entering Passive Mode ({address[0]},{address[1]},{address[2]},{address[3]},{portArray[0]},{portArray[1]})";
        }

        private string Type(string typeCode, string formatControl)
        {
            switch (typeCode.ToUpperInvariant())
            {
                case "A":
                    this.connectionType = TransferType.Ascii;
                    break;
                case "I":
                    this.connectionType = TransferType.Image;
                    break;
                default:
                    return "504 Command not implemented for that parameter";
            }

            if (!string.IsNullOrWhiteSpace(formatControl))
            {
                switch (formatControl.ToUpperInvariant())
                {
                    case "N":
                        this.formatControlType = FormatControlType.NonPrint;
                        break;
                    default:
                        return "504 Command not implemented for that parameter";
                }
            }

            return $"200 Type set to {this.connectionType}";
        }

        private string Retrieve(string pathname)
        {
            pathname = this.NormalizeFilename(pathname);

            if (this.IsPathValid(pathname))
            {
                if (File.Exists(pathname))
                {
                    if (this.dataConnectionType == DataConnectionType.Active)
                    {
                        this.dataClient = new TcpClient();
                        this.dataClient.BeginConnect(this.dataEndpoint.Address, this.dataEndpoint.Port, this.DoRetrieve, pathname);
                    }
                    else
                    {
                        this.passiveListener.BeginAcceptTcpClient(this.DoRetrieve, pathname);
                    }

                    return $"150 Opening {this.dataConnectionType} mode data transfer for RETR";
                }
            }

            return "550 File Not Found";
        }

        private void DoRetrieve(IAsyncResult result)
        {
            if (this.dataConnectionType == DataConnectionType.Active)
            {
                this.dataClient.EndConnect(result);
            }
            else
            {
                this.dataClient = this.passiveListener.EndAcceptTcpClient(result);
            }

            string pathname = (string)result.AsyncState;

            using (NetworkStream dataStream = this.dataClient.GetStream())
            using (FileStream fs = new FileStream(pathname, FileMode.Open, FileAccess.Read))
            {
                this.CopyStream(fs, dataStream);
                this.dataClient.Close();
                this.dataClient = null;
                this.controlWriter.WriteLine("226 Closing data connection, file transfer successful");
                this.controlWriter.Flush();
            }
        }

        private string List(string pathname)
        {
            pathname = this.NormalizeFilename(pathname);

            if (pathname != null)
            {
                if (this.dataConnectionType == DataConnectionType.Active)
                {
                    this.dataClient = new TcpClient();
                    this.dataClient.BeginConnect(this.dataEndpoint.Address, this.dataEndpoint.Port, this.DoList, pathname);
                }
                else
                {
                    this.passiveListener.BeginAcceptTcpClient(this.DoList, pathname);
                }

                return $"150 Opening {this.dataConnectionType} mode data transfer for LIST";
            }

            return "450 Requested file action not taken";
        }

        private void DoList(IAsyncResult result)
        {
            if (this.dataConnectionType == DataConnectionType.Active)
            {
                this.dataClient.EndConnect(result);
            }
            else
            {
                this.dataClient = this.passiveListener.EndAcceptTcpClient(result);
            }

            string pathname = (string)result.AsyncState;

            NetworkStream dataStream = this.dataClient.GetStream();
            using (StreamWriter dataWriter = new StreamWriter(dataStream, Encoding.ASCII))
            {
                IEnumerable<string> directories = Directory.EnumerateDirectories(pathname);

                foreach (string dir in directories)
                {
                    DirectoryInfo d = new DirectoryInfo(dir);

                    string date = d.LastWriteTime < DateTime.Now - TimeSpan.FromDays(180) ?
                        d.LastWriteTime.ToString("MMM dd  yyyy", CultureInfo.CurrentCulture) :
                        d.LastWriteTime.ToString("MMM dd HH:mm", CultureInfo.CurrentCulture);

                    string line = string.Format(CultureInfo.CurrentCulture, "drwxr-xr-x  {0} {1}", date, d.Name);

                    dataWriter.WriteLine(line);
                    dataWriter.Flush();
                }

                IEnumerable<string> files = Directory.EnumerateFiles(pathname);

                foreach (string file in files)
                {
                    FileInfo f = new FileInfo(file);

                    string date = f.LastWriteTime < DateTime.Now - TimeSpan.FromDays(180) ?
                        f.LastWriteTime.ToString("MMM dd  yyyy", CultureInfo.CurrentCulture) :
                        f.LastWriteTime.ToString("MMM dd HH:mm", CultureInfo.CurrentCulture);

                    string line = string.Format(CultureInfo.CurrentCulture, "-rw-r--r-- {0} {1} {2}", f.Length, date, f.Name);


                    dataWriter.WriteLine(line);
                    dataWriter.Flush();
                }
            }

            this.dataClient.Close();
            this.dataClient = null;

            this.controlWriter.WriteLine("226 Transfer complete");
            this.controlWriter.Flush();
        }
    }
}
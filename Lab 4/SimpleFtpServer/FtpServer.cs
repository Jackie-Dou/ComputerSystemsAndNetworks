namespace SimpleFtpServer
{
    using System;
    using System.Net;
    using System.Net.Sockets;
    using System.Threading;

    /// Listens for connections from TCP network clients using TcpListtener.
  
    public class FtpServer
    {
        /// Listens for connections from TCP network clients.
        private TcpListener listener;

        /// Starts listening for connections.
        public void Start()
        {
            const int Port = 21;
            this.listener = new TcpListener(IPAddress.Any, Port);
            this.listener.Start();

            // Begins an asynchronous operation to accept an incoming connection attempt.
            this.listener.BeginAcceptTcpClient(this.HandleAcceptTcpClient, this.listener);
        }

        /// Stops listening for connections.
        public void Stop()
        {
            this.listener.Stop();
        }

        /// Called when someone connected.
        /// <param name="result">Represents the status of asynchronous operation.</param>
        private void HandleAcceptTcpClient(IAsyncResult result)
        {
            TcpClient client = this.listener.EndAcceptTcpClient(result);

            // Continue listening for more connections.
            this.listener.BeginAcceptTcpClient(this.HandleAcceptTcpClient, this.listener);

            ClientConnection connection = new ClientConnection(client);

            // Creates a new background thread for client (this keeps foreground free).
            ThreadPool.QueueUserWorkItem(connection.HandleClient, client);
        }
    }
}
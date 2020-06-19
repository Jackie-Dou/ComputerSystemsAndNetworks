namespace SimpleFtpServer
{
    using System;

    public class Program
    {
        public static void Main()
        {
            FtpServer ftpServer = new FtpServer();

            ftpServer.Start();

            Console.WriteLine("Press any key to stop...");
            Console.ReadKey();

            ftpServer.Stop();
        }
    }
}
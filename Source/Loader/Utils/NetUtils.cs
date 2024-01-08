using System;
using System.Net;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;

namespace Loader
{
    public static class NetUtils
    {
        public static string HostnameToIPv4(string Hostname)
        {
            try
            {
                IPAddress[] Addresses = Dns.GetHostAddresses(Hostname);
                foreach (IPAddress Addr in Addresses)
                {
                    if (Addr.AddressFamily == AddressFamily.InterNetwork)
                    {
                        return Addr.ToString();
                    }
                }
            }
            catch (Exception)
            {
                // Shitty catch all exception ...
            }
            return "";
        }

        public static string GetMachineIPv4(bool GetPublicAddress)
        {
            try
            { 
                if (GetPublicAddress)
                {
                    using (WebClient client = new WebClient())
                    {
                        WebProxy proxy = new WebProxy();
                        proxy.Address = null; // Bypass the proxy
                        client.Proxy = proxy;
                        return client.DownloadString("http://api.ipify.org");
                    }
                }
                else
                {
                    return HostnameToIPv4(Dns.GetHostName());
                }
            }
            catch (Exception)
            {
                // Shitty catch all exception ...
            }
            return "";
        }
    }
}

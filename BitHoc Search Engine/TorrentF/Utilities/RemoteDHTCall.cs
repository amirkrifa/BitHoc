using System;

using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using TorrentF.Utilities;

namespace TorrentF.Utilities
{
    
    class RemoteDHTCall
    {
        [DllImport("RemoteDhtApi.dll")]
        private static extern System.Int32 SendPing([MarshalAs(UnmanagedType.LPWStr)] string stweblogName, [MarshalAs(UnmanagedType.LPWStr)]string weblogUrl);
        [DllImport("RemoteDhtApi.dll")]
        private static extern System.Int32 SendPutRequest([MarshalAs(UnmanagedType.LPWStr)] string url, System.Int32 urlLength, [MarshalAs(UnmanagedType.LPWStr)] string key, System.Int32 keyLength, [MarshalAs(UnmanagedType.LPWStr)] string value, System.Int32 valueLength, System.Int32 ttl, [MarshalAs(UnmanagedType.LPWStr)] string applicationName, System.Int32 applicationNameLength);

        static public int SendPingTest()
        {
            string a = "It's Just Code!";
            string b = "http://www.pocketsoap.com/weblog/";
            return SendPing(a, b);
            
        }
        
        static public int SendPutRequestToRemoteDHT(string key, string value)
        {

            //return SendPingTest(); 
            return SendPutRequest(TorrentFConfig.GetConfig().xmlRpcServiceUrl, TorrentFConfig.GetConfig().xmlRpcServiceUrl.Length, key, key.Length, value, value.Length, TorrentFConfig.GetConfig().xmlRequestTTL, TorrentFConfig.GetConfig().applicationName, TorrentFConfig.GetConfig().applicationName.Length);
        }
    }
}

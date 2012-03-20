using System;

using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using TorrentF.Utilities;


namespace TorrentF.Utilities
{
    // Wrapper class for the DLL we use to create the torrent file starting
    // from the data file
    class CreateTorrentFile
    {
        [DllImport("TorrentCreationApi.dll")]
        private static extern void ProcessSourceFile([MarshalAs(UnmanagedType.LPWStr)]string sSourceFilePath, int length1, [MarshalAs(UnmanagedType.LPWStr)] string sTrackerAddress, int length2, [MarshalAs(UnmanagedType.LPWStr)] string sTorrentName, int length3);
        public static void CreateTFile(string sourceFileName, string sTrackerAddress, string sTorrentName)
        {
            ProcessSourceFile(sourceFileName, sourceFileName.Length, sTrackerAddress, sTrackerAddress.Length, sTorrentName, sTorrentName.Length);
        }
    }
}

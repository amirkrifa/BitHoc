/*

  Copyright (C) 2009  INRIA, Planète Team

  Authors: 
  ------------------------------------------------------------
  Amir Krifa			:  Amir.Krifa@sophia.inria.fr
  Mohamed Karim Sbai	:  Mohamed_Karim.Sbai@sophia.inria.fr
  Chadi Barakat			:  Chadi.Barakat@sophia.inria.fr
  Thierry Turletti		:  Thierry.Turletty@sophia.inria.fr
  ------------------------------------------------------------

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

using System;

using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
namespace TorrentF.FilesStatus
{
    class FileDownloadStatus:FileDetails
    {
        private bool downloaded;
        private Int32 downloadedBytes = 0;
        public bool Downloaded
        {
            get
            {
                bool res;
                lock (this)
                {
                    res = downloaded;
                }
                return downloaded;
            }
            set
            {
                lock (this)
                {
                    downloaded = value;
                }
            }
        }
        public Int32 DownloadedBytes
        {
            get
            {
                return downloadedBytes;
            }
            set
            {
                downloadedBytes = value;
            }
        }
        public FileDownloadStatus(bool _strorageStatus, string _fileName, long _fileSize, string _remoteHostIp,Int32 _remoteHostPort, string _localFilePath):base(_strorageStatus,_fileName,_fileSize,_remoteHostIp,_remoteHostPort,_localFilePath)
        {
            Trace.Assert(_remoteHostIp.Length > 0, "FileDownloadStatus, invalid _remoteHostIp: " + _remoteHostIp);
            Trace.Assert(_remoteHostPort > 0, "FileDownloadStatus, invalid _remoteHostPort: " + _remoteHostPort.ToString());

            downloaded = false;
            downloadedBytes = 0;
        }
        // Return The message that should be sent to the remote host to ask for this file
        public string GetDownloadMessage()
        {
            Trace.Assert(fileName.Length > 0, "GetDownloadMessage, invalid fileName: "+fileName);
            Trace.Assert(fileSize > 0, "GetDownloadMessage, invalid fileSize: " + fileSize);

            StringBuilder sb = new StringBuilder();
            sb.Append(fileName);
            sb.Append("#");
            sb.Append(fileSize.ToString());
            return sb.ToString();
        }
    }
}

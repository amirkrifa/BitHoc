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
    public class RemoteHostCoordinates
    {
        public RemoteHostCoordinates(ref string ip, ref int port)
        {
            hostIp = ip;
            hostPort = port;
        }
        public string hostIp;
        public int hostPort;
    }
        
    public class FileDetails
    {

        public FileDetails(bool _strorageStatus, string _fileName, long _fileSize, string _remoteHostIp,Int32 _remoteHostPort, string _localFilePath)
        {
            Trace.Assert(_fileName.Length > 0, "FileDetails, invalid _fileName: " + _fileName);
            Trace.Assert(_fileSize > 0, "FileDetails, invalid _fileSize: " + _fileSize.ToString());
           
            StorageStatus = _strorageStatus;
            FileName = _fileName;
            FileSize = _fileSize;
            RemoteHostIp = _remoteHostIp;
            LocalFilePath = _localFilePath;
            RemoteHostPort = _remoteHostPort;
            hostList = new List<RemoteHostCoordinates>();
            AddHostCoordinates(ref _remoteHostIp, ref _remoteHostPort);
        }

        protected bool storageStatus;
        protected string fileName = null;
        protected long fileSize = 0;
        
        // Used to save the related sahring session details
        protected SharingSessionDetails sessionDetails = null;
        public SharingSessionDetails SessionDetails
        {
            get
            {
                SharingSessionDetails tmp = null;
                lock (this)
                {
                    tmp = sessionDetails;
                }
                return tmp;
            }
        }
        public void AppendRelatedSessionDetails(ref SharingSessionDetails d)
        {
            if(sessionDetails == null)
                sessionDetails = d;
        }

        // Used to save a reference to the DataFileDescription Object
        protected string dataFileDescription = null;
        public string DataDescription
        {
            get
            {
                    return dataFileDescription;
            }
            set 
            {
                lock (this)
                {
                    dataFileDescription = value;
                }
            }
        }

        // Selected Host for downloading 
        protected string remoteHostIp = null;
        protected Int32 remoteHostPort = 0;
        
        // List of possible hosts for downloading
        List<RemoteHostCoordinates> hostList = null;
        public List<RemoteHostCoordinates> HostList
        {
            get
            {
                return hostList;
            }
        }

        // Method used to add a new remote host coordinates
        public void AddHostCoordinates(ref string ip, ref int port )
        {
            // Verify that the coordinates was not already added
            bool exists = false;
            foreach(RemoteHostCoordinates rhc in hostList)
            {
                
                if (rhc.hostIp.CompareTo(ip) == 0 && rhc.hostPort == port)
                {
                    exists = true;
                    break;
                }
            }

            if(!exists)
                hostList.Add(new RemoteHostCoordinates(ref ip, ref port));

        }

        protected string localFilePath = null;

        public bool StorageStatus
        {
            get
            {
                return storageStatus;
            }
            set
            {
                storageStatus = value;
            }
        }

        public string FileName
        {
            get
            {
               return fileName;
            }
            set
            {
                fileName = value;
            }
        }

        public long FileSize
        {
            get
            {
                return fileSize;
            }
            set
            {
                fileSize = value;
            }
        }

        public string RemoteHostIp
        {
            get
            {
                string tmp = null;
                lock (this)
                {
                    tmp = remoteHostIp;
                }
                return tmp;
            }

            set
            {
                lock (this)
                {
                    remoteHostIp = value;
                }
            }
        }

        public Int32 RemoteHostPort
        {
            get
            {
                return remoteHostPort;
            }

            set
            {
                remoteHostPort = value;
            }
        }

        public  string LocalFilePath
        {
            get
            {
                return localFilePath;
            }

            set
            {
                localFilePath = value;
            }
        }

        #region IDisposable Members



        #endregion
    }
}

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
using System.Net.Sockets;
using System.Threading;
using TorrentF.ThreadParam;
using TorrentF.FilesStatus;

namespace TorrentF.Managers
{
    class UploadManager : Managers
    {
        private FilePublisher filesPublisher = null;
        private bool uploadServerStarted;
        private bool stopUploadServer;
        private FilesUploadingThreadParam futp;
        Thread serverThread = null;
        public bool StopUploadServer
        {
            get
            {
                bool res;
                lock(this)
                { 
                    res = stopUploadServer;
                }
                return res;
            }
        }
        private Dictionary<string, FilePublishingStatus> uploadDic = null;
        
        // Used for the singleton design patterns
        static private UploadManager uploadManager = null;
        static public UploadManager GetUploadManager(FilePublisher _filePublisher)
        {
            if (uploadManager == null)
            {
                uploadManager = new UploadManager(_filePublisher);
            }

            return uploadManager;
        }

       
        private UploadManager(FilePublisher _filePublisher)
        {
            filesPublisher = _filePublisher;
            uploadDic = new Dictionary<string, FilePublishingStatus>();
            uploadServerStarted = false;
        }
        public void StartUploadServer()
        {
            if (!uploadServerStarted)
            {
                futp = new FilesUploadingThreadParam(this);
                ThreadStart ts = new ThreadStart(futp.MainServerMethod);
                serverThread = new Thread(ts);
                serverThread.Start();
                uploadServerStarted = true;
            }
            
        }

        #region Managers Members

        public void StopRelatedServers()
        {
            lock (this)
            {
                stopUploadServer = true;
            }
        }

        #endregion
    }
}

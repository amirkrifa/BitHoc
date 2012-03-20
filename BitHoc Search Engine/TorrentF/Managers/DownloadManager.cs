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
using TorrentF.ThreadParam;
using System.Threading;
using System.Windows.Forms;
using System.Diagnostics;

namespace TorrentF.Managers
{
    class DownloadManager: Managers
    {
        private FilesManager filesManager = null;
        private List<string> downloadList = null;
        private bool stopDownloading = false;
        public bool StopDownloading
        {
            get
            {
                bool ret = false;
                lock (this)
                {
                    ret = stopDownloading;
                }
                return ret;
            }
            set
            {
                lock (this)
                {
                    stopDownloading = value;
                }
            }
        }
        public List<string> DownloadList
        {
            get
            {
                return downloadList;
            }
        }

        public void AddDownloadingFile(ref string fileName)
        {
            Trace.Assert(fileName.Length > 0, "DownloadManager::AddDownloadingFile, invalid fileName: " + fileName);
            lock (this)
            {
                downloadList.Add(fileName);
            }
        }

        public void RemoveDownloadFile(ref string fileName)
        {
            Trace.Assert(fileName.Length > 0, "DownloadManager::RemoveDownloadFile, invalid fileName: " + fileName);
            lock (this)
            {
                downloadList.Remove(fileName);
            }
        }

        public bool Downloading(ref string fileName)
        {
            Trace.Assert(fileName.Length > 0, "DownloadManager::Downloading, invalid fileName: " + fileName);
            bool res= false;
            lock (this)
            {
                if (downloadList.Contains(fileName))
                    res = true;
            }
            return res;
        }

        static private DownloadManager downloadManager = null;
        static public DownloadManager GetDownloadManager(FilesManager _filesManager)
        {
            if (downloadManager == null)
            {
                downloadManager = new DownloadManager(ref _filesManager);
            }

            return downloadManager;
        }

        private DownloadManager(ref FilesManager _filesManager)
        {
            filesManager = _filesManager;
            downloadList = new List<string>();
        }

        public void DownloadFile(ref string _fileName)
        {
            Trace.Assert(_fileName.Length > 0, "DownloadManager::DownloadFile, invalid fileName: " + _fileName);
            
            if (Downloading(ref _fileName))
            {
                MessageBox.Show("This file is being downloaded, please wait.", _fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
            }
            else
            {

                FileDownloadingThreadParam fdp = new FileDownloadingThreadParam(filesManager, this);
                fdp.CurrentFileName = _fileName;
                ThreadStart ts = new ThreadStart(fdp.StartDownloading);
                Thread downloader = new Thread(ts);
                downloader.Start();
            }
        }

        #region Managers Members

        public void StopRelatedServers()
        {
            StopDownloading = true;
        }

        #endregion
    }
}

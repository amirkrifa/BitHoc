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
GNU General Public License for more details.h

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

using System;
using System.Threading;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.IO;
using TorrentF.ThreadParam;
using TorrentF.FilesStatus;
using System.Diagnostics;
using TorrentF.Utilities;
using System.Windows.Forms;

namespace TorrentF.Managers
{

    class FilesLookupManager:Managers
    {
        // specify the lookup method
        // 0: not specified 
        // 1: Ask for all available Torrent files
        // 2: Ask for a specific Torrent file
        private Int32 lookUpMethod;
       
        public Int32 LookUpMethod
        {
            get
            {
                return lookUpMethod;
            }
            set
            {
                lookUpMethod = value;
            }
        }

        // used for the singleton design pattern
        static private FilesLookupManager filesLookupManager;

        static public FilesLookupManager GetFilesLookupManager()
        {
            if (filesLookupManager == null)
            {
                filesLookupManager = new FilesLookupManager();
            }
            return filesLookupManager;
        }

        private FilesLookupManager()
        {
            lookUpMethod = 2;
        }

        // Default metho used to find a partucular file via the local tracker
        public List<FileDetails> FindFile(string fileName, ref string existingFileName)
        {
            Trace.Assert(fileName.Length > 0, "FilesLookupManager::FindFile, invalid fileName: ", fileName);
            
            SingleFileLookupThreadParam sft = new SingleFileLookupThreadParam();
            sft.CurrentFileDescription = fileName;
            ThreadStart ts = new ThreadStart(sft.RequestSingleFile);
            Thread t = new Thread(ts);
            t.Start();
            // Wait for the thread until it finish
            t.Join();
            if (sft.CurrentFileDetails.Count == 0)
                existingFileName = sft.ExistingFileName;
            return sft.CurrentFileDetails;
        }

        // Method used to find a specific file via the remote DHT
        public List<FileDetails> FindFileViaDht(string fileName, ref string existingFileName)
        {
            Trace.Assert(fileName.Length > 0, "FilesLookupManager::FindFile, invalid fileName: ", fileName);
            List<FileDetails> lFd = new List<FileDetails>();
            MessageBox.Show("The \"get\" function is disabled for the moment.", "RemoteDhtApi",MessageBoxButtons.OK,MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
            return null;
        }



        // Request all the available files from the local tracker
        public List<FileDetails> RequestAllAvailableFiles()
        {
            MultiFileLookupThreadParam mft = new MultiFileLookupThreadParam();
            // Create a thread to request files
            ThreadStart ts = new ThreadStart(mft.RequestAllAvailableThread);
            Thread t = new Thread(ts);
            t.Start();
            // Wait for the thread to finish
            t.Join();
            return mft.LookupList;
        }

        #region Managers Members

        public void StopRelatedServers()
        {

        }

        #endregion
    }
}

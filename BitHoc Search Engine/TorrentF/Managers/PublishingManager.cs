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
using System.Threading;
using System.Windows.Forms;
using TorrentF.ThreadParam;
using TorrentF.Utilities;
using System.Net;
using TorrentF.FilesStatus;
namespace TorrentF.Managers
{ 
    class FilePublisher : Managers
    {
        // Indicates the publishing method to use:
        // 0: No publishing method has been selected yet
        // 1: Publish the files via the local tracker
        // 2: Publish the files via the DHT
        private Int32 publishingMethod;
        private FilesManager filesManager = null;
        private UploadManager uploadManager = null;
        static private FilePublisher filePublisher = null;
        static public FilePublisher GetFilePublisher(FilesManager _filesManager)
        {
            if(filePublisher == null)
            {
                filePublisher = new FilePublisher(_filesManager);
            }
            return filePublisher;
        }
        private FilePublisher(FilesManager _filesManager)
        {

            publishingMethod = 1;
            filesManager = _filesManager;

            // Starting the upload server
            uploadManager = UploadManager.GetUploadManager(this);
            uploadManager.StartUploadServer();
        }

        public Int32 PublishingMethod
        {
            get
            {
                return publishingMethod;
            }

            set
            {
                if (value == 1)
                    TorrentFConfig.GetConfig().useLocalTracker = true;
                else if(value == 2)
                    TorrentFConfig.GetConfig().useLocalTracker = false;

                publishingMethod = value;
            }
        }

        // Default publishing method
        public void PulishFile(FilePublisherThreadParam threadParam)
        {
            try
            {
                // Start The thread That will take in charge the publishing
                ThreadStart ts = new ThreadStart(threadParam.PublishFile);
                Thread t = new Thread(ts);
                t.Start();
            }

            catch
            {
                // Error occured while trying to publish a new torrent file
                MessageBox.Show("Error occured while trying to publish the new torrent file.");
            }

        }
        
        // Publishing the file via the DHT
        public int PulishFileViaDht(FileDetails fd)
        {
            try
            {
                // The key = file name 
                // The value = local node Ip @
                IPAddress[] ips = Dns.GetHostByName(Dns.GetHostName()).AddressList;
                // TODO: Verify the correct Ip@
                if (RemoteDHTCall.SendPutRequestToRemoteDHT(fd.FileName, ips[0].ToString()) == 1)
                {
                    // Published
                    return 1;
                }
                else
                {
                    // Error occured on the underlying RemoteDhtApi.dll
                    MessageBox.Show("Error occured while trying to publish the new torrent file.","DHT API",MessageBoxButtons.OK,MessageBoxIcon.Asterisk,MessageBoxDefaultButton.Button1);
                    return 0;
                }

                
            }

            catch
            {
                // Error occured while trying to publish a new torrent file
                MessageBox.Show("Error occured while trying to publish the new torrent file.");
                return 0;
            }

        }

        #region Managers Members

        public void StopRelatedServers()
        {
            uploadManager.StopRelatedServers();
        }

        #endregion
    }
}

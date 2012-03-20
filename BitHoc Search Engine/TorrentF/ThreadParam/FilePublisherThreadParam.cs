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
using System.Windows.Forms;
using TorrentF.Managers;
using TorrentF.FilesStatus;
using TorrentF.Utilities;
using System.Diagnostics;
namespace TorrentF.ThreadParam
{
    class FilePublisherThreadParam:ThreadParam
    {
        public FilePublisherThreadParam(FilesManager _filesManager)
        {
            filesManager = _filesManager;
        }

        private FileDetails currentFileDetails = null;
        private string currentFileDescription = null;
        private bool publishingStatus;
        private FilesManager filesManager = null;

        public bool PublishingStatus
        {
            get
            {
                return publishingStatus;
            }
            set
            {
                publishingStatus = value;
            }
        }

        public FileDetails CurrentFileDetails
        {
            get
            {
                return currentFileDetails;
            }
            set
            {
                currentFileDetails = value;
            }
        }

        public string CurrentFileDescription
        {
            get
            {
                return currentFileDescription;
            }
            set
            {
                currentFileDescription = value;
            }
        }

        // Method that will be executed by the thread in order to publish the torrent file
        public void PublishFile()
        {
            TcpClient tcp = null;
            NetworkStream ns = null;
            try
            {
                // Construct the message that will be sent to the tracker
                StringBuilder message = new StringBuilder("GET /PublishTorrent?file_name=");
                // Adding the torrent file name
                message.Append(currentFileDescription);
                
                message.Append("&file_size=");
                
                message.Append(currentFileDetails.FileSize.ToString());
                message.Append("&file_description=");
                message.Append(this.currentFileDetails.DataDescription);
                message.Append("");
                // Adding the torrent file size
                message.Append(" HTTP/1.1\n\n");

                tcp = new TcpClient(TorrentFConfig.GetConfig().trackerIp, TorrentFConfig.GetConfig().trackerHttpPort);
                Trace.Assert(tcp != null, "FilePublisherThreadParam::PublishFile, Cannot open a connexion to the local tracker.");
                ns = tcp.GetStream();
                byte[] buf = System.Text.Encoding.ASCII.GetBytes(message.ToString());

                ns.Write(buf, 0, buf.Length);
                ns.Flush();
                
                publishingStatus = true;
                MessageBox.Show("File successfuly published.", currentFileDetails.FileName, MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                filesManager.UpdateFilePublishingStatus(ref currentFileDescription, true);

                filesManager.numberOfPublishedFiles++;
            }

            catch
            {
                publishingStatus = false;
                MessageBox.Show("Error occured while publishing the torrent file.","Error",MessageBoxButtons.OK,MessageBoxIcon.Exclamation,MessageBoxDefaultButton.Button1);
                filesManager.UpdateFilePublishingStatus(ref currentFileDescription, false);
            }

            finally
            {
                if (ns != null)
                    ns.Close();

                if(tcp != null)
                    tcp.Close();

            }
        }

    }
}

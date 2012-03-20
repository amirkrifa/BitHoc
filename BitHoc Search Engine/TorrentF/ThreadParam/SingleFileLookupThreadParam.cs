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
using TorrentF.Utilities;
using TorrentF.FilesStatus;
using System.Diagnostics;

namespace TorrentF.ThreadParam
{
    class SingleFileLookupThreadParam:ThreadParam
    {

        // The list of possible answers
        private List<FileDetails> currentFileDetails = null;
        //  The number of possible answers
        private int numberOfAnswers = 0;
        public int NumberOfAnswers
        {
            get
            {
                int tmp = 0;
                lock (this)
                {
                    tmp = numberOfAnswers;
                }
                return tmp;
            }

            set
            {
                lock (this)
                {
                    numberOfAnswers = value;
                }
            }
        }

        // The keyword to look for
        private string currentFileDescription = null;
        
        // indicates if the file is already here
        private string existingFileName = null;
        public string ExistingFileName
        {
            get
            {
                string tmp = null;
                lock (this)
                {
                    tmp = existingFileName;
                }
                return tmp;
            }
            set
            {
                lock (this)
                {
                    existingFileName = value;
                }
            }
        }
           
        public SingleFileLookupThreadParam()
        {
            currentFileDetails = new List<FileDetails>();
        }

        public List<FileDetails> CurrentFileDetails
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


        public void RequestSingleFile()
        {
            Trace.Assert(CurrentFileDescription != null, "SingleFileLookupThreadParam::RequestSingleFile, invalid keyword.");
            TcpClient tcpClient = null;
            NetworkStream nStream = null;
            //try
            {
                tcpClient = new TcpClient(TorrentFConfig.GetConfig().trackerIp, TorrentFConfig.GetConfig().trackerHttpPort);
                Trace.Assert(tcpClient != null, "SingleFileLookupThreadParam::RequestSingleFile, cannot open connexion to the local tracker for lookup purpose.");
                // Construct the message to send 
                StringBuilder message = new StringBuilder("GET /GetTorrentOwner?file_name=");
                // Add the file name to look for
                message.Append(currentFileDescription);
                message.Append("&black_list=local");
                message.Append(" HTTP/1.1\n\n");

                nStream = tcpClient.GetStream();
                byte[] buffer = System.Text.Encoding.ASCII.GetBytes(message.ToString());
                nStream.Write(buffer, 0, buffer.Length);
                nStream.Flush();
                // Receive response
                byte[] receiveBuffer = new byte[1024];
                int nRead = nStream.Read(receiveBuffer, 0, receiveBuffer.Length);
                // received data format: node1Ip#file1*file1.size
                string receivedMessage = null;
                receivedMessage = System.Text.Encoding.ASCII.GetString(receiveBuffer, 0, nRead);
                if (receivedMessage.Length > 0)
                {
                    // Parse received message
                    ParseTrackerMessage ptm = new ParseTrackerMessage();
                    
                    ptm.ParseMultiFileMessage(receivedMessage,ref existingFileName);
                    if (ptm.ResultingFiles.Count > 0)
                    {
                        if (ptm.ResultingFiles.Count == 1)
                        {
                            // Just one answer
                            CurrentFileDetails.Add(ptm.ResultingFiles[0]);
                            NumberOfAnswers = 1;
                        }
                        else
                        {
                            // Multiple possible choices
                            foreach (FileDetails fd in ptm.ResultingFiles)
                            {
                                CurrentFileDetails.Add(fd);
                            }
                            NumberOfAnswers = ptm.ResultingFiles.Count;
                        }
                    }
                }
                else
                {
                    MessageBox.Show("Invalid received data from the local tracker.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                }

            }

           // catch
            {
                // Problem occured while trying to request a file from the tracker
//                MessageBox.Show("Problem occured while trying to request a file from the tracker", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
            }

           // finally
            {
                if (nStream != null)
                    nStream.Close();

                if (tcpClient != null)
                    tcpClient.Close();
            }
        }
        

    }
}

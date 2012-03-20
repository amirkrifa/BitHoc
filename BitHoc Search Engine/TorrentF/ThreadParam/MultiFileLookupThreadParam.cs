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
    class MultiFileLookupThreadParam:ThreadParam
    {
        private List<FileDetails> lookupList = null;
        private string existingFileName = null;
        public MultiFileLookupThreadParam()
        {
         }

        public List<FileDetails> LookupList
        {
            get
            {
                return lookupList;
            }
            set
            {
                lookupList = value;
            }
        }
        
        public void RequestAllAvailableThread()
        {
            TcpClient tcpClient = null;
            NetworkStream nStream = null;

            try
            {
                tcpClient = new TcpClient(TorrentFConfig.GetConfig().trackerIp, TorrentFConfig.GetConfig().trackerHttpPort);
                Trace.Assert(tcpClient != null, "MultiFileLookupThreadParam::RequestAllAvailableThread, cannot open connexion to the local tracker for lookup purpose.");
                
                // Sending the request
                string message = "GET /GetListOfPublishedTorrents HTTP/1.1\n\n";
                nStream = tcpClient.GetStream();
                byte[] buffer = System.Text.Encoding.ASCII.GetBytes(message);
                nStream.Write(buffer, 0, buffer.Length);
                nStream.Flush();

                // Treating the answer
                byte[] receiveBuffer = new byte[3000];
                string receivedMessage = null;
                Int32 nRead = nStream.Read(receiveBuffer, 0, receiveBuffer.Length);
                // received list format :node1Ip#file1*file1.size-file2*file2.size-file3*file3.size\n
                receivedMessage = System.Text.Encoding.ASCII.GetString(receiveBuffer, 0, nRead);
                if (receivedMessage.Length > 0)
                {
                    receivedMessage = System.Text.Encoding.ASCII.GetString(receiveBuffer, 0, nRead);
                    ParseTrackerMessage ptm = new ParseTrackerMessage();
                    ptm.ParseMultiFileMessage(receivedMessage, ref existingFileName);
                    lookupList = ptm.ResultingFiles;
                }
                else
                {
                    MessageBox.Show("Invalid received data from the local tracker.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                }
            }

            catch
            {
                // Problem occured while trying to request a file from the tracker
                MessageBox.Show("Problem occured while trying to request a file from the tracker","Error",MessageBoxButtons.OK,MessageBoxIcon.Exclamation,MessageBoxDefaultButton.Button1);
            }

            finally
            {
                if (tcpClient != null)
                {
                    if(tcpClient!=null)
                        tcpClient.Close();
                    if (nStream != null)
                        nStream.Close();
                }
            }
        }

    }
}

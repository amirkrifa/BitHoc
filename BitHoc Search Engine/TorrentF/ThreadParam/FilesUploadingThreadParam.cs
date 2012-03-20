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
using System.Net;
using System.Net.Sockets;
using System.Windows.Forms;
using System.IO;
using System.Threading;

using TorrentF.Managers;
using TorrentF.Utilities;
using TorrentF.FilesStatus;
using System.Xml.Serialization;
using System.Diagnostics;

namespace TorrentF.ThreadParam
{
    class FilesUploadingThreadParam : ThreadParam
    {

        private UploadManager uploadManager = null;
        private TcpListener listner = null;
        public TcpListener Listner
        {
            get
            {
                return listner;
            }
        }

        public FilesUploadingThreadParam(UploadManager _uploadManager)
        {
            uploadManager = _uploadManager;
      
        }

        public void MainServerMethod()
        {

            TcpClient remoteClient = null;
            FileStream fs = null;

            try
            {

                IPAddress[] ips = Dns.GetHostByName(Dns.GetHostName()).AddressList;
                // TODO: Verify the correct Ip@
                listner = new TcpListener(ips[0], TorrentFConfig.GetConfig().uploadingServerPort);
                Trace.Assert(listner != null, "FileUploadingThreadParam::MainSeerverMethod, cannot start the TorrentF server at the port: " + TorrentFConfig.GetConfig().uploadingServerPort.ToString());
                listner.Start();
                while (!uploadManager.StopUploadServer)
                {
                    // Start a server
                    if (listner.Pending())
                    {
                        remoteClient = listner.AcceptTcpClient();
                    }
                    else
                    {
                        // No pending connections, just sleep
                        Thread.Sleep(1);
                        continue;
                    }

                    // New connecion accepted --> Creating a new thread that will take care of this new connection
                    SingleFileUploading sfu = new SingleFileUploading(remoteClient, uploadManager);
                    ThreadStart ts = new ThreadStart(sfu.ThreadUploadingFunction);
                    Thread t = new Thread(ts);
                    t.Start();
                }


            }

            catch (SocketException)
            {
                MessageBox.Show("Probem occcured while trying to connect to the remote host.", "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
            }

            finally
            {
                if (fs != null)
                    fs.Close();
                if (remoteClient != null)
                    remoteClient.Close();
                if (listner != null)
                    listner.Stop();
            }

        }


        // A single file uploading Thread Param object
        private class SingleFileUploading
        {
            private TcpClient remoteClient = null;
            private NetworkStream stream = null;
            private UploadManager uploadManager = null;
            public SingleFileUploading(TcpClient _remoteClient, UploadManager _uploadManager)
            {
                remoteClient = _remoteClient;
                uploadManager = _uploadManager;
            }

            public void ThreadUploadingFunction()
            {
                try
                {
                    
                    // Read Request Message
                    byte[] message = new byte[1024];
                    stream = remoteClient.GetStream();
                    int nRead = stream.Read(message, 0, 1024);
                    if (nRead > 0)
                    {
                        
                        string receivedMessage = System.Text.Encoding.ASCII.GetString(message, 0, nRead);
                        
                        // File properties
                        string fileName = ParseP2PMessage.GetFileName(receivedMessage);
                        long fileSize = ParseP2PMessage.GetFileSize(receivedMessage);
                        
                        // Get the file details
                        FilePublishingStatus fps = (FilePublishingStatus)FilesManager.GetFileManager().GetFileDetails(ref fileName);
                        
                        if (fps != null)
                        {
                            FileStream fs = new FileStream(fps.LocalFilePath, FileMode.Open, FileAccess.Read);
                            Trace.Assert(fs != null, "FileUploadingThreadParam::ThreadUploadingFunction, cannot open file for uploading: " + fps.LocalFilePath);
                            byte[] tmp = new byte[1024];
                            int s = 0;
                            while ((s = fs.Read(tmp, 0, 1024)) != 0)
                            {
                                stream.Write(tmp, 0, s);
                            }
                            fs.Close();
                            
                            /*
                             * Solution Based on the XML Serialization of the object DataFileBox
                            // Box the data file
                            DataFileBox dataBox = new DataFileBox();
                            dataBox.BoxDataFile(fps.LocalFilePath);
                            // Box the data file description
                            dataBox.BoxDataFileDescription(fps.DataDescription);
                            // Serialize and send the dataBox
                            XmlSerializer xs = new XmlSerializer(typeof(DataFileBox));
                            xs.Serialize(stream, (object)dataBox);
                            stream.Close();
                            */
                            
                            stream.Close();
                            remoteClient.Close();
                            MessageBox.Show("Sending the file ...", fileName, MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);

                        }
                        else
                        {
                            MessageBox.Show("File not found.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                        }

                    }
                }

                catch (SocketException)
                {
                    MessageBox.Show("Probem occcured while trying to connect to the remote host.", "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                }

                finally
                {
                    if (remoteClient != null)
                        remoteClient.Close();
                }
            }
        }
    }
}



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
using System.IO;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Net;
using TorrentF.FilesStatus;
using TorrentF.ThreadParam;
using TorrentF.Main;
using TorrentF.RelatedForms;
using TorrentF.Utilities;
using System.Diagnostics;

namespace TorrentF.Managers
{
    class FilesManager:Managers
    {
        // used for the Singleton design pattern
        static private FilesManager filesManager = null;
        
        // Files Managers
        private DownloadManager downloadManager = null;
        public DownloadManager DownloadManager
        {
            get
            {
                return downloadManager;
            }
        }
        public int numberOfDownloadedFiles = 0;

        private FilesLookupManager lookupManager = null;
        public FilesLookupManager LookupManager
        {
            get
            {
                return lookupManager;
            }
        }

        private FilePublisher filePublisher;
        public FilePublisher FilePublisher
        {
            get
            {
                return filePublisher;
            }
        }
        public int numberOfPublishedFiles = 0;

        // Main Form
        private MainForm mainForm = null;
        public MainForm MainForm
        {
            get
            {
                return mainForm;
            }
        }
    
        // Main Files Dict
        private Dictionary<string, FileDetails> mainFilesDic = null;
        // Method that enable the creation of a single instance of FilesManager
        static public FilesManager GetFileManager(MainForm mf)
        {
            if (filesManager == null)
            {

                filesManager = new FilesManager(ref mf);
            }   
            return filesManager;
        }
        static public FilesManager GetFileManager()
        {
            return filesManager;
        }
        private FilesManager(ref MainForm mf)
        {
    
            mainFilesDic = new Dictionary<string, FileDetails>();
            mainForm = mf;

            // Creation of managers            
            downloadManager = DownloadManager.GetDownloadManager(this);
            lookupManager = FilesLookupManager.GetFilesLookupManager();
            filePublisher = FilePublisher.GetFilePublisher(this);
        }

        //Current file description string 
        private string currentDataDescription = null;
        public string CurrentDataDescription
        {
            get
            {
                string tmp = null;
                lock (this)
                {
                    tmp = currentDataDescription;
                }
                return tmp;
            }
            set
            {
                currentDataDescription = value;
            }
        }

        public List<FileDetails> FindAndAddFile(string fileName, ref string existingFileName)
        {
            List<FileDetails> listFd = null;
            
            if(TorrentFConfig.GetConfig().useLocalTracker)
                listFd = lookupManager.FindFile(fileName, ref existingFileName);
            else listFd = lookupManager.FindFileViaDht(fileName, ref existingFileName);

            if (listFd != null && listFd.Count > 0)
            {
                foreach (FileDetails fd in listFd)
                {
                    if (fd != null)
                    {
                        if (!mainFilesDic.ContainsKey(fd.FileName))
                        {
                            mainFilesDic.Add(fd.FileName, fd);
                        }
                    }
                }
                return listFd;
            }
            else
            {
                return null;
            }
                    
        }

        public List<FileDetails> GetAllAvailableFiles()
        {
            List<FileDetails> tmp = lookupManager.RequestAllAvailableFiles();
            List<FileDetails> newFiles = new List<FileDetails>();

            if (tmp != null)
            {
                
                foreach (FileDetails fd in tmp)
                {
                    if(!mainFilesDic.ContainsKey(fd.FileName))
                    {
                        mainFilesDic.Add(fd.FileName, fd);
                        newFiles.Add(fd);
                    }
                }
            }

            return newFiles;
        }
        
        public FileDetails GetFileDetails(ref string _fileName)
        {
            if (mainFilesDic.ContainsKey(_fileName))
                return mainFilesDic[_fileName];
            else return null;
        }
        
        // Default publishing method via the local tracker 
        public void PublishFile(string fileAbsolutePath, out FileDetails _fd)
        {
            FilePublisherThreadParam tp = new FilePublisherThreadParam(this);
           
            // Verify if we have already tried to publish this file or not 
            if (!mainFilesDic.ContainsKey(Path.GetFileName(fileAbsolutePath)))
            {
                // Get file details Bsed on its path
                FileInfo fi = new FileInfo(fileAbsolutePath);
                FilePublishingStatus fd = new FilePublishingStatus(true, fi.Name, fi.Length, "", 0, fileAbsolutePath);

                FileDescription fileDescriptionDialog = new FileDescription();
                fileDescriptionDialog.Closed += new EventHandler(fileDescriptionDialog_Closed);
                fileDescriptionDialog.ShowDialog();

                fd.DataDescription = CurrentDataDescription;
                
                tp.CurrentFileDetails = fd;
                tp.CurrentFileDescription = fi.Name;

                filePublisher.PulishFile(tp);
                
                _fd = fd;
            
                // Adding the published file to the local dict
                AddFile(fi.Name, (FileDetails)fd);
            }
            else
            {
                // Already exists in the dict
                FileDetails fd = mainFilesDic[Path.GetFileName(fileAbsolutePath)];
                _fd = fd;
                tp.CurrentFileDetails = fd;
                tp.CurrentFileDescription = fd.FileName;
                filePublisher.PulishFile(tp);

            }
        }

        // Publishing the file via the DHT
        public Int32 PublishFileViaDHT(string fileAbsolutePath, out FileDetails _fd)
        {
            
            // Verify if we have already tried to publish this file or not 
            if (!mainFilesDic.ContainsKey(Path.GetFileName(fileAbsolutePath)))
            {
                // Get file details Bsed on its path
                FileInfo fi = new FileInfo(fileAbsolutePath);
                FilePublishingStatus fd = new FilePublishingStatus(true, fi.Name, fi.Length, "", 0, fileAbsolutePath);

                fd.DataDescription = null;


                int res = filePublisher.PulishFileViaDht(fd);
                if (res == 1)
                {
                    _fd = fd;
                    // Adding the published file to the local dict
                    AddFile(fi.Name, (FileDetails)fd);
                }
                else _fd = null;

                return res;
            }
            else
            {
                // Already exists in the dict
                FileDetails fd = mainFilesDic[Path.GetFileName(fileAbsolutePath)];
                
                int res = filePublisher.PulishFileViaDht(fd);
                if (res == 1)
                    _fd = fd;
                else _fd = null;
                return res;
            }
        }

        void fileDescriptionDialog_Closed(object sender, EventArgs e)
        {
            FileDescription tmp =  (FileDescription)sender;
            lock(this)
            {
                currentDataDescription = tmp.description;
            }
        }

        public void UpdateFilePublishingStatus(ref string fileName, bool status)
        {
            if (mainFilesDic.ContainsKey(fileName))
            {
                FileDetails fd = (FileDetails)mainFilesDic[fileName];
                if (fd is FileDownloadStatus )
                {
                    // Move the file from the downloaded status to published local file
                    // We have to provide some description for the file we downloaded, or just
                    // Download the remote description
                    FilePublishingStatus fps = new FilePublishingStatus(fd.StorageStatus, fd.FileName, fd.FileSize, fd.RemoteHostIp, fd.RemoteHostPort, fd.LocalFilePath);
                    fps.Published = status;
                    mainFilesDic.Remove(fileName);
                    mainFilesDic.Add(fileName, fps);
                }
                else
                {
                    FilePublishingStatus fps = (FilePublishingStatus)mainFilesDic[fileName];
                    fps.Published = status;
                }
            }
        }

        public bool GetFilePublishingStatus(string fileName)
        {
            if (mainFilesDic.ContainsKey(fileName))
            {
                FilePublishingStatus fps = (FilePublishingStatus)mainFilesDic[fileName];
                return fps.Published;
            }else
            {
                // File not found 
                MessageBox.Show("Invalid File Name!","Error", MessageBoxButtons.OK,MessageBoxIcon.Exclamation,MessageBoxDefaultButton.Button1);
                return false;
            }
        }

        public FileDownloadStatus MoveFileToDownloadList(ref string fileName)
        {
            if (mainFilesDic.ContainsKey(fileName))
            {
                FileDetails fd = mainFilesDic[fileName];
                FileDownloadStatus fds = new FileDownloadStatus(fd.StorageStatus, fd.FileName, fd.FileSize, fd.RemoteHostIp, fd.RemoteHostPort, fd.LocalFilePath);
                // Copy the data description
                fds.DataDescription = fd.DataDescription;
                // Copy the related session details
                SharingSessionDetails ssd = fd.SessionDetails;
                fds.AppendRelatedSessionDetails(ref ssd);

                // Copy the other hosts
                if (fd.HostList.Count > 1)
                {
                    foreach (RemoteHostCoordinates rhc in fd.HostList)
                    {
                        if (rhc.hostIp.CompareTo(fds.RemoteHostIp) != 0)
                        {
                            fds.AddHostCoordinates(ref rhc.hostIp, ref rhc.hostPort);
                        }
                    }
                }

                mainFilesDic.Remove(fileName);
                mainFilesDic.Add(fileName, fds);
                return fds;
            }
            else
            {
                MessageBox.Show("File does not exist.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                return null;
            }
        }

        private void AddFile(ref bool _strorageStatus, ref string _fileName, ref Int32 _fileSize, ref string _remoteHostIp, ref Int32 _remoteHostPort, ref string _localFilePath)
        {
            lock (this)
            {
                if (mainFilesDic.Count == 0 || !mainFilesDic.ContainsKey(_fileName))
                {
                    // Adding the file to the main dic
                    FileDetails fd = new FileDetails(_strorageStatus, _fileName, _fileSize, _remoteHostIp,_remoteHostPort, _localFilePath);
                    mainFilesDic.Add(_fileName, fd);
                }
            }
        }

        private void AddFile(string _fileName, FileDetails _fileDetails)
        {
            lock (this)
            {
                if (!mainFilesDic.ContainsKey(_fileName))
                {
                    mainFilesDic.Add(_fileName, _fileDetails);
                }
            }
        }
       
        public void RemoveFile(ref string fileName)
        {
            if (mainFilesDic.ContainsKey(fileName))
            {
                mainFilesDic.Remove(fileName);
            }
            else
            {
                // File not found 
                MessageBox.Show("Invalid File Name!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
            }
        }

        public string AskForLocalInfos()
        {
            TcpClient tcpClient = null;
            try
            {
                
                tcpClient = new TcpClient(TorrentFConfig.GetConfig().trackerIp, TorrentFConfig.GetConfig().trackerHttpPort);
                // Sending the request
                string message = "GET /GetLocalInfos HTTP/1.1\n\n";
                NetworkStream nStream = tcpClient.GetStream();
                byte[] buffer = System.Text.Encoding.ASCII.GetBytes(message);
                nStream.Write(buffer, 0, buffer.Length);
                nStream.Flush();

                // Treating the answer
                byte[] receiveBuffer = new byte[3000];
                string receivedMessage = null;
                Int32 nRead = nStream.Read(receiveBuffer, 0, receiveBuffer.Length);
               
                receivedMessage = System.Text.Encoding.ASCII.GetString(receiveBuffer, 0, nRead);
                
                if (receivedMessage.Length > 0)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.Append(receivedMessage);
                    sb.Append("-----------------------------------------\r\n");
                    sb.Append("Number of published torrent files: ");
                    sb.Append(numberOfPublishedFiles.ToString());
                    sb.Append("\r\n");
                    sb.Append("-----------------------------------------\r\n");
                    sb.Append("Number of downloaded torrent files: ");
                    sb.Append(numberOfDownloadedFiles.ToString());
                    sb.Append("\r\n");
                    sb.Append("-----------------------------------------\r\n");
                    return sb.ToString();
                }
                else
                {
                    MessageBox.Show("Invalid received data from the local tracker.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                }
            }

            catch
            {
                // Problem occured while trying to request a file from the tracker
                MessageBox.Show("Problem occured while trying to request local infos from the tracker", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
            }

            finally
            {
                if (tcpClient != null)
                {
                    tcpClient.Close();
                    
                }
            }
            return null;

        }

        #region Managers Members

        public void StopRelatedServers()
        {
            filePublisher.StopRelatedServers();
            DownloadManager.StopRelatedServers();
            lookupManager.StopRelatedServers();
        }

        #endregion
    }
}

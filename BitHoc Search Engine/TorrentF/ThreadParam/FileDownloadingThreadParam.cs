using System;

using System.Collections.Generic;
using System.Text;
using TorrentF.FilesStatus;
using System.Net.Sockets;
using TorrentF.Managers;
using System.IO;
using System.Windows.Forms;
using System.Xml.Serialization;
using System.Diagnostics;

namespace TorrentF.ThreadParam
{
    class FileDownloadingThreadParam: ThreadParam
    {
        // The name of the file to be downloaded
        private string currentFileName;
        public string CurrentFileName
        {
            set
            {
                currentFileName = value;
            }
        }

        // a reference to the main manager
        private FilesManager filesManager = null;
        private DownloadManager downloadManager = null;

        public FileDownloadingThreadParam(FilesManager _fm, DownloadManager _downloadManager)
        {
            filesManager = _fm;
            downloadManager = _downloadManager;
        }

        // Main thread Method
        public void StartDownloading()
        {
            TcpClient client = null;
            NetworkStream stream = null;
            
            try
            {

                FileDownloadStatus fds = (FileDownloadStatus)filesManager.GetFileDetails(ref currentFileName);
                if (!fds.Downloaded)
                {
                    downloadManager.AddDownloadingFile(ref currentFileName);

                    client = new TcpClient(fds.RemoteHostIp, fds.RemoteHostPort);
                    stream = client.GetStream();
                    string message = fds.GetDownloadMessage();
                    byte[] buf = System.Text.Encoding.ASCII.GetBytes(message);
                    Trace.Assert(buf.Length > 0, "FileDownloadingThreadParam::StartDownloading, invalid request message.");
                    stream.Write(buf, 0, buf.Length);
                    stream.Flush();
                    

                    // Start receiving and storing the file
                    StringBuilder filePath = new StringBuilder();

                    if (!Directory.Exists(@".\My Documents"))
                    {
                        Directory.CreateDirectory(@".\My Documents");
                    }

                    if (!Directory.Exists(@".\My Documents\Expeshare"))
                    {
                        Directory.CreateDirectory(@".\My Documents\Expeshare");
                    }

                    if (!Directory.Exists(@".\My Documents\Expeshare\"))
                    {
                        Directory.CreateDirectory(@".\My Documents\Expeshare\");
                    }

                    filePath.Append(".\\My Documents\\Expeshare\\");
                    filePath.Append(fds.FileName);

                    // XmlSerializer xs = new XmlSerializer(typeof(DataFileBox));
                    
                    StringBuilder tmpSb = new StringBuilder();
                    byte[] tmpBuf = new byte[1024];
                    int r = 0;
                    FileStream fs = new FileStream(filePath.ToString(), FileMode.OpenOrCreate, FileAccess.Write);
                    
                    while ((r = stream.Read(tmpBuf, 0, 1024)) != 0)
                    {
                        tmpSb.Append(System.Text.Encoding.ASCII.GetString(tmpBuf, 0, r));
                        fs.Write(tmpBuf, 0, r);
                    }
                    fs.Close();

                    //TextReader stringReader = new StringReader(tmpSb.ToString());
                    //string tmpData = stringReader.ReadToEnd();
                    
                    //DataFileBox dataBox = (DataFileBox)xs.Deserialize(stringReader);
                    //string tmp2 = dataBox.GetData();
                    
                    byte [] tmpData = System.Text.Encoding.ASCII.GetBytes(tmpSb.ToString().ToCharArray());
                   
                    // update the downloaded bytes in the Dialog
                    filesManager.MainForm.UpdateDownloadingStatus(ref currentFileName, tmpData.Length);
                    
                    // Close all streams
                    stream.Close();
                    client.Close();

                    // Indicate that the file was already downloaded 
                    if (tmpData.Length == fds.FileSize)
                    {
                        fds.Downloaded = true;
                        fds.DownloadedBytes = tmpData.Length;
                        // Update the file description 
                        //fds.DataDescription = dataBox.GetDataFileDescription();
                        // Write the file to the disk
                        //dataBox.WriteDataFileTo(filePath.ToString());
                        
                        // Write the received data to the local device
                     /*   FileStream fs = new FileStream(filePath.ToString(), FileMode.OpenOrCreate, FileAccess.Write);
                        fs.Write(tmpData, 0, tmpData.Length);
                        fs.Flush();
                        fs.Close();
                        */
                        MessageBox.Show("File successefully downloaded.", currentFileName, MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);

                        // Message to add to the log dialog
                        StringBuilder sb = new StringBuilder();
                        sb.Append("The file: \"");
                        sb.Append(fds.FileName);
                        sb.Append("\" has been successefully downloaded.");
                        filesManager.MainForm.AppendToLogDialog(sb.ToString());
                        filesManager.numberOfDownloadedFiles++;
                        downloadManager.RemoveDownloadFile(ref currentFileName);
                        fds.LocalFilePath = filePath.ToString();
                    }
                    else
                    {
                        // Remove the incompleted files
                        File.Delete(fds.FileName);
                    }
                }
                else
                {
                    // Message to add to the log dialog
                    StringBuilder sb = new StringBuilder();
                    filesManager.MainForm.AppendToLogDialog("Trying to download an already downloaded file.");
                    MessageBox.Show("File already downloaded.", currentFileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                }
            }

            catch (SocketException)
            {
                downloadManager.RemoveDownloadFile(ref currentFileName);
                MessageBox.Show("Problem occured while trying to connect to the remote host.", "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                
            }

            finally
            {
                if (client != null)
                    client.Close();
                if (stream != null)
                    stream.Close();
            }
        }
    }
}

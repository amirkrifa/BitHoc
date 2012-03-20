using System;

using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using TorrentF.Managers;
using TorrentF.FilesStatus;
using TorrentF.RelatedForms;
using System.Net;
using System.Diagnostics;
using TorrentF.Utilities;
namespace TorrentF.Main
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            filesManager = FilesManager.GetFileManager(this);
            textBoxFileName.Enabled = true;

        }
        private FilesManager filesManager;

        
        private void tabMainControl_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void publishFile_Click(object sender, EventArgs e)
        {
            // Verify first thta the user has selected the publishing method
            switch (filesManager.FilePublisher.PublishingMethod)
            {
                case 0:
                    // No publishing method has been selected
                    MessageBox.Show("Please select the publishing method.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                    break;
                case 1:
                    if(TorrentFConfig.GetConfig().useLocalTracker)
                    {
                        // Publish the file via the local tracker.
                        DialogResult publishTorrent = openFileDialogPublish.ShowDialog();
                        string fileName = Path.GetFileName(openFileDialogPublish.FileName);

                        if (publishTorrent.Equals(DialogResult.OK))
                        {
                            // Create implicitly the torrent file associated to the 
                            // selected data file
                            string tracker = "http://"+TorrentFConfig.GetConfig().trackerIp+":"+TorrentFConfig.GetConfig().trackerHttpPort+"/announce";
                            string torrentFileName = openFileDialogPublish.FileName+".torrent";
                            CreateTorrentFile.CreateTFile(openFileDialogPublish.FileName, tracker, openFileDialogPublish.FileName);

                            FileDetails fd = null;
                            filesManager.PublishFile(torrentFileName, out fd);

                            ListViewItem item = new ListViewItem();
                            item.Text = fileName + @".torrent";

                            ListViewItem.ListViewSubItem subItem1 = new ListViewItem.ListViewSubItem();
                            subItem1.Text = fd.FileSize.ToString();

                            item.SubItems.Add(subItem1);
                            listViewPublished.Items.Add(item);

                            // Message to add to the log dialog
                            StringBuilder sb = new StringBuilder();
                            sb.Append("The file: \"");
                            sb.Append(fileName);
                            sb.Append("\" has been correctly published via the local tracker.");
                            AppendToLogDialog(sb.ToString());
                        }
                    }
                    break;
                case 2:
                    // Publish the file via the DHT
                    // The key = file name 
                    // The value = local node Ip @
                    if (!TorrentFConfig.GetConfig().useLocalTracker)
                    {
                        DialogResult publishTorrent = openFileDialogPublish.ShowDialog();
                        string fileName = Path.GetFileName(openFileDialogPublish.FileName);

                        if (publishTorrent.Equals(DialogResult.OK))
                        {
                            FileDetails fd = null;
                            if (filesManager.PublishFileViaDHT(openFileDialogPublish.FileName, out fd) == 1)
                            {

                                // Adding the item to the list
                                ListViewItem item = new ListViewItem();
                                item.Text = fileName;

                                ListViewItem.ListViewSubItem subItem1 = new ListViewItem.ListViewSubItem();
                                subItem1.Text = fd.FileSize.ToString();

                                item.SubItems.Add(subItem1);
                                listViewPublished.Items.Add(item);

                                // Message to add to the log dialog
                                StringBuilder sb = new StringBuilder();
                                sb.Append("The file: \"");
                                sb.Append(fileName);
                                sb.Append("\" has been correctly published via the local tracker.");
                                AppendToLogDialog(sb.ToString());
                            }
                        }
                    }
                    break;
                default:
                    // Invalid publishing Method
                    MessageBox.Show("Invalid publishing method.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                    break;
            }
           
        }

        private void comboBoxPublishingMethod_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Changing the publishing method
            filesManager.FilePublisher.PublishingMethod = comboBoxPublishMethodConfig.SelectedIndex + 1;
        }

        private void menuItemRetryPublishing_Click(object sender, EventArgs e)
        {
            //  Retry to publish the selected Item
            if (listViewPublished.SelectedIndices.Count > 0)
            {
                int selectedItemIndex = listViewPublished.SelectedIndices[0];
                FileDetails fd = null;
                filesManager.PublishFile(listViewPublished.Items[selectedItemIndex].Text, out fd);
            }
            else MessageBox.Show("Please select an item from the list first.!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);

        }

        private void menuItemDelete_Click(object sender, EventArgs e)
        {
            // Delete the selected item from the list
            if (listViewPublished.SelectedIndices.Count > 0)
            {
                // Just removing the item from the list view 
                int selectedItemIndex = listViewPublished.SelectedIndices[0];
                string fileName = listViewPublished.Items[selectedItemIndex].Text;
                listViewPublished.Items.RemoveAt(selectedItemIndex);
                // Clear the file from the main dictionary
                filesManager.RemoveFile(ref fileName);
                
            }
            else MessageBox.Show("Please select an item from the list!", "Error",MessageBoxButtons.OK,MessageBoxIcon.Exclamation,MessageBoxDefaultButton.Button1);
        }

        private void menuItemStatus_Click(object sender, EventArgs e)
        {
            if (listViewPublished.SelectedIndices.Count > 0)
            {
                int selectedItemIndex = listViewPublished.SelectedIndices[0];
                bool status = filesManager.GetFilePublishingStatus(listViewPublished.Items[selectedItemIndex].Text);
                if (status)
                {
                    MessageBox.Show("File already published.", listViewPublished.Items[selectedItemIndex].Text,MessageBoxButtons.OK,MessageBoxIcon.Asterisk,MessageBoxDefaultButton.Button1);
                }
                else
                {
                    MessageBox.Show("File not published.", listViewPublished.Items[selectedItemIndex].Text, MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                }

            }
            else MessageBox.Show("Please select an item from the list!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
        }

        private void buttonFind_Click(object sender, EventArgs e)
        {
            switch (filesManager.LookupManager.LookUpMethod)
            {
                case 0:
                    MessageBox.Show("Please select the lookup method.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                    break;
                case 1:
                    // 1: Ask for all available Torrent files, works just via the local tracker
                    if (TorrentFConfig.GetConfig().useLocalTracker)
                    {
                        List<FileDetails> res = filesManager.GetAllAvailableFiles();
                        buttonFind.Enabled = false;
                        if (res != null)
                        {
                            foreach (FileDetails fd in res)
                            {
                                // Add the file to the list
                                ListViewItem item = new ListViewItem();
                                item.Text = fd.FileName;

                                ListViewItem.ListViewSubItem subItem = new ListViewItem.ListViewSubItem();
                                subItem.Text = fd.RemoteHostIp;

                                item.SubItems.Add(subItem);
                                listViewFound.Items.Add(item);
                            }
                            res.Clear();

                            AppendToLogDialog("Asking the local tracker for all the available torrent files.(Done)");
                        }
                        else
                            MessageBox.Show("0 Torrent file(s) found.", "Infos", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                        buttonFind.Enabled = true;
                    }
                    else
                    {
                        MessageBox.Show("The selected lookup method is not supported by the remote DHT.", "Not supported", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                    }
                    break;
                case 2:
                    // 2: Ask for a specific Torrent file
                    if (TorrentFConfig.GetConfig().useLocalTracker)
                    {
                        if (textBoxFileName.Text.Length > 0)
                        {
                            buttonFind.Enabled = false;
                            StringBuilder sb = new StringBuilder();
                            sb.Append("Asking the local tracker for the file: \"");
                            sb.Append(textBoxFileName.Text);
                            string existingFileName = null;
                            List<FileDetails> listFd = filesManager.FindAndAddFile(textBoxFileName.Text, ref existingFileName);
                            if (listFd != null && listFd.Count > 0)
                            {
                                foreach (FileDetails fd in listFd)
                                {
                                    // Add the file to the list
                                    ListViewItem item = new ListViewItem();
                                    item.Text = fd.FileName;

                                    ListViewItem.ListViewSubItem subItem = new ListViewItem.ListViewSubItem();
                                    subItem.Text = fd.RemoteHostIp;

                                    item.SubItems.Add(subItem);
                                    listViewFound.Items.Add(item);

                                }
                                sb.Append("\".(Done)");
                            }
                            else
                            {
                                if (existingFileName == null)
                                {
                                    sb.Append("\".(File Not Found)");
                                    MessageBox.Show("File not found.", textBoxFileName.Text, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                                }
                                else MessageBox.Show("File already available.", existingFileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                            }

                            AppendToLogDialog(sb.ToString());
                            buttonFind.Enabled = true;
                        }
                        else
                        {
                            MessageBox.Show("Please specify file name.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                        }
                    }
                    else
                    {
                        // Send a request via the via the remote DHT without asking for the dada description
                        if (textBoxFileName.Text.Length > 0)
                        {
                            buttonFind.Enabled = false;
                            StringBuilder sb = new StringBuilder();
                            sb.Append("Asking the local tracker for the file: \"");
                            sb.Append(textBoxFileName.Text);
                            string existingFileName = null;
                            List<FileDetails> listFd = filesManager.FindAndAddFile(textBoxFileName.Text, ref existingFileName);
                            if (listFd != null && listFd.Count > 0)
                            {
                                foreach (FileDetails fd in listFd)
                                {
                                    // Add the file to the list
                                    ListViewItem item = new ListViewItem();
                                    item.Text = fd.FileName;

                                    ListViewItem.ListViewSubItem subItem = new ListViewItem.ListViewSubItem();
                                    subItem.Text = fd.RemoteHostIp;

                                    item.SubItems.Add(subItem);
                                    listViewFound.Items.Add(item);

                                }
                                sb.Append("\".(Done)");
                            }
                            else
                            {
                                if (existingFileName == null)
                                {
                                    sb.Append("\".(File Not Found)");
                                    MessageBox.Show("File not found.", textBoxFileName.Text, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                                }
                                else MessageBox.Show("File already available.", existingFileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                            }

                            AppendToLogDialog(sb.ToString());
                            buttonFind.Enabled = true;
                        }
                        else
                        {
                            MessageBox.Show("Please specify file name.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        private void comboBoxLookupMethods_SelectedIndexChanged(object sender, EventArgs e)
        {
           
        }

        private void buttonSelectFileForDownloading_Click(object sender, EventArgs e)
        {
            // Verify if there is an element already selected in listViewFound
            if (listViewFound.SelectedIndices.Count > 0)
            {
                // selected file name
                string fileName = listViewFound.Items[listViewFound.SelectedIndices[0]].Text;
                string hostIp = listViewFound.Items[listViewFound.SelectedIndices[0]].SubItems[1].Text;
                
                // Verify if it is a local file or not 
                IPAddress[] ips = Dns.GetHostByName(Dns.GetHostName()).AddressList;
                
                // The local Ip @ is ips[0]
                if(ips[0].ToString().CompareTo(hostIp) == 0)
                {
                    // It is a local file, we can just republish it
                    MessageBox.Show("That is a local file.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                }
                else
                {
                    // It is a remote file, we can download it
                    // remove the file from the found list 
                    listViewFound.Items.RemoveAt(listViewFound.SelectedIndices[0]);

                    FileDownloadStatus fds = filesManager.MoveFileToDownloadList(ref fileName);
                    if (fds != null)
                    {
                        // Adding the file to the download list
                        ListViewItem item = new ListViewItem();
                        item.Text = fds.FileName;

                        ListViewItem.ListViewSubItem subItem1 = new ListViewItem.ListViewSubItem();
                        subItem1.Text = fds.FileSize.ToString();

                        ListViewItem.ListViewSubItem subItem2 = new ListViewItem.ListViewSubItem();
                        subItem2.Text = fds.DownloadedBytes.ToString();

                        item.SubItems.Add(subItem1);
                        item.SubItems.Add(subItem2);
                        listViewDownload.Items.Add(item);
                    }
                }

            }
            else if(listViewFound.Items.Count == 0) 
                MessageBox.Show("The list of files is empty.","Error",MessageBoxButtons.OK,MessageBoxIcon.Asterisk,MessageBoxDefaultButton.Button1);
            else MessageBox.Show("Please select a file from the list.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
        }

        private void MainForm_Closing(object sender, CancelEventArgs e)
        {
            // Stopping the UploadServer
            filesManager.StopRelatedServers();
        }

        private void buttonDownloadFile_Click(object sender, EventArgs e)
        {
            // Download the selected file
            if (listViewDownload.Items.Count > 0)
            {
                if (listViewDownload.SelectedIndices.Count > 0)
                {

                    string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
                    // Verify if the file has been already downloaded or not 
                    if (fileName.Length > 0)
                    {
                        filesManager.DownloadManager.DownloadFile(ref fileName);
                    }
                }
                else MessageBox.Show("Please select a file from the list.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
            }
            else MessageBox.Show("The list of files is empty.","Error",MessageBoxButtons.OK,MessageBoxIcon.Asterisk,MessageBoxDefaultButton.Button1);
        }

        // Function used to update the download status of a given file in the list listViewDownload
        public void UpdateDownloadingStatus(ref string _fileName, int numberBytes)
        {
            this.Invoke(new UpdateDownloadingStatusDelegate(UpdateDownloadingS), new object[]{_fileName,numberBytes});
        }


        // a delegate method
        private delegate void UpdateDownloadingStatusDelegate(ref string _fileName, int numberBytes);
        public void UpdateDownloadingS(ref string _fileName, int numberBytes)
        {
            ListViewItem item = null; 
            // First Find the ListViewItem
            foreach (ListViewItem it in listViewDownload.Items)
            {
                if (it.Text.CompareTo(_fileName) == 0)
                {
                    item = it;
                    break;
                }
            }

            // Updating the number of bytes
            listViewDownload.Items[listViewDownload.Items.IndexOf(item)].SubItems[2].Text = numberBytes.ToString();

        }

        // function used while downloading a file
        private void buttonRemoveFile_Click(object sender, EventArgs e)
        {
            if (listViewDownload.Items.Count > 0)
            {
                if (listViewDownload.SelectedIndices.Count > 0)
                {
                    // Get the selected item fileName
                    string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
                    if (fileName.Length > 0)
                    {
                        FileDownloadStatus fds = (FileDownloadStatus)filesManager.GetFileDetails(ref fileName);
                        if (fds.Downloaded)
                        {
                            listViewDownload.Items.RemoveAt(listViewDownload.SelectedIndices[0]);
                            // Clear the file from the main dictionary
                            filesManager.RemoveFile(ref fileName);
                        }
                        else MessageBox.Show("File not already downloaded, please wait.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);

                    }
                }
                else MessageBox.Show("Please select a file from the list.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
            }
            else MessageBox.Show("The list of files is empty.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
        }

        private void buttonUpdateLocalInfos_Click(object sender, EventArgs e)
        {
            textBoxLocalInfos.Text = "";
            textBoxLocalInfos.Text = filesManager.AskForLocalInfos();
        }

        // Method used to add log entries 
        public void AppendToLogDialog(string text)
        {
            this.Invoke(new AppendToLogDelegate(AppendToLog), new object[] { text });
        }
        private delegate void AppendToLogDelegate(string text);
        private void AppendToLog(string text)
        {
            StringBuilder fullText = new StringBuilder();
            fullText.Append(DateTime.Now.ToString().ToCharArray());

            fullText.Append(" : ");
            fullText.Append(text.ToCharArray());
            fullText.Append("\r\n");
            fullText.Append(textBoxLog.Text);
            textBoxLog.Text = fullText.ToString();
  
        }

        private void menuItemExit_Click(object sender, EventArgs e)
        {
            filesManager.StopRelatedServers();
            Application.Exit();
        }

        private void menuItemDownloadRelatedData_Click(object sender, EventArgs e)
        {
            if (listViewDownload.Items.Count > 0)
            {
                // Get the selected item fileName
                string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
                if (fileName.Length > 0)
                {
                    FileDownloadStatus fds = (FileDownloadStatus)filesManager.GetFileDetails(ref fileName);
                    if (fds.Downloaded)
                    {
                        StringBuilder sb = new StringBuilder();
                        sb.Append(fds.LocalFilePath);
                        Process.Start(TorrentF.Utilities.TorrentFConfig.GetConfig().bitHocClientRelativePath, sb.ToString());
                    }
                    else MessageBox.Show("File not already downloaded.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                }
            }
            else MessageBox.Show("The list of files is empty.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
   
        }

        private void menuItemGetFullPath_Click(object sender, EventArgs e)
        {
            if (listViewDownload.Items.Count > 0)
            {
                // Get the selected item fileName
                string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
                if (fileName.Length > 0)
                {
                    FileDownloadStatus fds = (FileDownloadStatus)filesManager.GetFileDetails(ref fileName);
                    if (fds.Downloaded)
                    {
                        StringBuilder sb = new StringBuilder();
                        sb.Append("File path: ");
                        sb.Append(fds.LocalFilePath);
                        sb.Append(".");
                        MessageBox.Show(sb.ToString(),fds.FileName,MessageBoxButtons.OK,MessageBoxIcon.Asterisk,MessageBoxDefaultButton.Button1);
                    }
                    else MessageBox.Show("File not already downloaded.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                }
            }
            else MessageBox.Show("The list of files is empty.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
        }

        private void menuItemPublish_Click(object sender, EventArgs e)
        {
            if (listViewDownload.Items.Count > 0)
            {
                // Get the selected item fileName
                string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
                if (fileName.Length > 0)
                {
                    FileDownloadStatus fds = (FileDownloadStatus)filesManager.GetFileDetails(ref fileName);
                    if (fds.Downloaded)
                    {
                        FileDetails fd = null;
                   
                        filesManager.PublishFile(fds.FileName, out fd);
                        ListViewItem item = new ListViewItem();
                        item.Text = fds.FileName;

                        ListViewItem.ListViewSubItem subItem1 = new ListViewItem.ListViewSubItem();
                        subItem1.Text = fds.FileSize.ToString();

                        item.SubItems.Add(subItem1);
                        
                        listViewPublished.Items.Add(item);

                        // Message to add to the log dialog
                        StringBuilder sb = new StringBuilder();
                        sb.Append("The file: \"");
                        sb.Append(fds.FileName);
                        sb.Append("\" has been correctly published via the local tracker.");
                        AppendToLogDialog(sb.ToString());

                    }
                    else MessageBox.Show("File not already downloaded.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                }
            }
            else MessageBox.Show("The list of files is empty.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);

        }

        private void menuItemRePublish_Click(object sender, EventArgs e)
        {
            string fileName = listViewFound.Items[listViewFound.SelectedIndices[0]].Text;
            string hostIp = listViewFound.Items[listViewFound.SelectedIndices[0]].SubItems[1].Text;

            // Verify if it is a local file or not 
            IPAddress[] ips = Dns.GetHostByName(Dns.GetHostName()).AddressList;
            if (ips[0].ToString().CompareTo(hostIp) != 0)
            {
                MessageBox.Show("This is not a local file.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
            }
            else
            {
                // Get the selected item fileName
                if (fileName.Length > 0)
                {
                    FileDetails fd = null;

                    filesManager.PublishFile(fileName, out fd);

                    if (fd != null)
                    {
                        ListViewItem item = new ListViewItem();
                        item.Text = fd.FileName;

                        ListViewItem.ListViewSubItem subItem1 = new ListViewItem.ListViewSubItem();
                        subItem1.Text = fd.FileSize.ToString();

                        item.SubItems.Add(subItem1);

                        listViewPublished.Items.Add(item);

                        // Remove currently selected item from the search list
                        listViewFound.Items.RemoveAt(listViewFound.SelectedIndices[0]);
          
                        // Message to add to the log dialog
                        StringBuilder sb = new StringBuilder();
                        sb.Append("The file: \"");
                        sb.Append(fileName);
                        sb.Append("\" has been correctly published via the local tracker.");
                        AppendToLogDialog(sb.ToString());
                    }

                }
            }
        }

        private void menuItemSelectRemoteHost_Click(object sender, EventArgs e)
        {
            // Get the selected item fileName
            string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
            PossibleRemoteHostsForm hosts = null;

                
            FileDetails fd = filesManager.GetFileDetails(ref fileName);
            FileDownloadStatus fds = (FileDownloadStatus)fd;
            if (!filesManager.DownloadManager.Downloading(ref fileName) && !fds.Downloaded)
            {
                hosts = new PossibleRemoteHostsForm(ref fd);
                if (fd != null)
                {
                    TreeNode node;
                    foreach (RemoteHostCoordinates rhc in fd.HostList)
                    {
                        node = hosts.TreeViewHosts.Nodes.Add(rhc.hostIp);
                        StringBuilder sb = new StringBuilder();
                        sb.Append("Host' Remote Port: ");
                        sb.Append(rhc.hostPort.ToString());
                        node.Nodes.Add(sb.ToString());
                    }
                }
             
                // Show the list of possible remote host
                hosts.TreeViewHosts.ExpandAll();
                hosts.ShowDialog();

            }
            else if (fds.Downloaded)
            {
                MessageBox.Show("File already downloaded.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
            }
            else MessageBox.Show("Still Downloading the file, please wait.", fileName, MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
        }

        private void menuItemExit_Click_1(object sender, EventArgs e)
        {
            //Excluded for hopper
            filesManager.StopRelatedServers();
            Application.Exit();
        }

        private void menuItemRelatedTorrentDetails_Click(object sender, EventArgs e)
        {
            // selected file name
            string fileName = listViewFound.Items[listViewFound.SelectedIndices[0]].Text;
            FileDetails fd = filesManager.GetFileDetails(ref fileName);
            // Show the related torrent session details if we recive any thing from the local tracker
            RelatedTorrentDetails relatedTD = new RelatedTorrentDetails(ref fd);
            relatedTD.ShowDialog();
           
        }

        private void menuItemRelatedDataDescription_Click(object sender, EventArgs e)
        {
            // Show the description of the selected published item
            string fileName = listViewPublished.Items[listViewPublished.SelectedIndices[0]].Text;
            FileDetails fd = filesManager.GetFileDetails(ref fileName);
            // Show the related torrent session details if we recive any thing from the local tracker
            RelatedTorrentDetails relatedTD = new RelatedTorrentDetails(ref fd);
            relatedTD.ShowDialog();
        }

        private void menuItemDLGetTorrentDetails_Click(object sender, EventArgs e)
        {
            // selected file name
            string fileName = listViewDownload.Items[listViewDownload.SelectedIndices[0]].Text;
            FileDetails fd = filesManager.GetFileDetails(ref fileName);
            // Show the related torrent session details if we recive any thing from the local tracker
            RelatedTorrentDetails relatedTD = new RelatedTorrentDetails(ref fd);
            relatedTD.ShowDialog();
        }

        private void menuItemFDDelete_Click(object sender, EventArgs e)
        {
            // Delete the selected item from the list
            if (listViewFound.SelectedIndices.Count > 0)
            {
                // Just removing the item from the list view 
                int selectedItemIndex = listViewFound.SelectedIndices[0];
                string fileName = listViewFound.Items[selectedItemIndex].Text;
                listViewFound.Items.RemoveAt(selectedItemIndex);
                // Clear the file from the main dictionary
                filesManager.RemoveFile(ref fileName);

            }
            else MessageBox.Show("Please select an item from the list!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
        }

        private void comboBoxLookupMethods_Validating(object sender, CancelEventArgs e)
        {
            if (!TorrentFConfig.GetConfig().useLocalTracker)
            {
               /* if (comboBoxLookupMethodConfig.SelectedIndex == 0)
                {
                    // This method is not supported by the DHT Api.
                    MessageBox.Show("This lookup method is not supported by the undelying DHT.", "DHT API", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
                    e.Cancel = true;
                    comboBoxLookupMethodConfig.SelectedIndex = 1;
                    textBoxFileName.Enabled = true;
                }*/

                textBoxFileName.Enabled = true;
            }
        
        }

        private void comboBoxPublishMethodConfig_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Changing the publishing method
            filesManager.FilePublisher.PublishingMethod = comboBoxPublishMethodConfig.SelectedIndex + 1;
        }

        private void comboBoxLookupMethodConfig_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Changing the publishing method
          /*  if (TorrentFConfig.GetConfig().useLocalTracker)
            {
                filesManager.LookupManager.LookUpMethod = comboBoxLookupMethodConfig.SelectedIndex + 1;
                if (comboBoxLookupMethodConfig.SelectedIndex == 1)
                {
                    textBoxFileName.Enabled = true;
                }
                else if (comboBoxLookupMethodConfig.SelectedIndex == 0)
                {
                    textBoxFileName.Enabled = false;
                }
            }
           * */
        }

        
        // Automaticalyy resize the background image of the About tab function of the used device
        Bitmap aboutTabBackgroundImage = null;
        Rectangle aboutTabBackgroundImageRectangle;
      
        private void About_Paint(object sender, PaintEventArgs e)
        {
            if (aboutTabBackgroundImage == null)
                aboutTabBackgroundImage = new Bitmap(@"\Program Files\TorrentF\logo.JPG");

            aboutTabBackgroundImageRectangle = new Rectangle(0, 0, aboutTabBackgroundImage.Width, aboutTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(aboutTabBackgroundImage, this.ClientRectangle, aboutTabBackgroundImageRectangle, GraphicsUnit.Pixel);
            
        }

        Bitmap publishTabBackgroundImage = null;
        Rectangle publishTabBackgroundImageRectangle;

        private void Publish_Paint(object sender, PaintEventArgs e)
        {
            if (publishTabBackgroundImage == null)
                publishTabBackgroundImage = new Bitmap(@"\Program Files\BitHoc Search Engine\contents.JPG");

            publishTabBackgroundImageRectangle = new Rectangle(0, 0, publishTabBackgroundImage.Width, publishTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(publishTabBackgroundImage, this.ClientRectangle, publishTabBackgroundImageRectangle, GraphicsUnit.Pixel);
            // Drawing a transparent text
            using (Brush headingBrush = new SolidBrush(Color.Black), stringBrush = new SolidBrush(Color.Black))
            {
                Font font = new Font(FontFamily.GenericSerif, 8, FontStyle.Bold);
                int textWidth = (int)(g.MeasureString("+ List of Published Files:", font).Width);
                g.DrawString("+ List of Published Files:", font, headingBrush, new Rectangle(this.ClientRectangle.X + (int)(this.ClientRectangle.Width / 32), this.ClientRectangle.Y + (int)Math.Round(this.ClientRectangle.Height/6.5), textWidth, textWidth));
            }
        }

        Bitmap findTabBackgroundImage = null;
        Rectangle findTabBackgroundImageRectangle;

        private void Find_Paint(object sender, PaintEventArgs e)
        {
            if (findTabBackgroundImage == null)
                findTabBackgroundImage = new Bitmap(@"\Program Files\BitHoc Search Engine\contents.JPG");

            findTabBackgroundImageRectangle = new Rectangle(0, 0, findTabBackgroundImage.Width, findTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(findTabBackgroundImage, this.ClientRectangle, findTabBackgroundImageRectangle, GraphicsUnit.Pixel);
            // Drawing a transparent text
            using (Brush headingBrush = new SolidBrush(Color.Black), stringBrush = new SolidBrush(Color.Black))
            {
                Font font = new Font(FontFamily.GenericSerif, 8, FontStyle.Bold);
                int textWidth = (int)(g.MeasureString("+ Lookup Expression:", font).Width);
                g.DrawString("+ Lookup Expression:", font, headingBrush, new Rectangle(this.ClientRectangle.X + (int)(this.ClientRectangle.Width / 32), this.ClientRectangle.Y + (int)Math.Round(this.ClientRectangle.Height/53.6), textWidth, textWidth));
            }
        }

        Bitmap downloadTabBackgroundImage = null;
        Rectangle downloadTabBackgroundImageRectangle;

        private void download_Paint(object sender, PaintEventArgs e)
        {
            if (downloadTabBackgroundImage == null)
                downloadTabBackgroundImage = new Bitmap(@"\Program Files\BitHoc Search Engine\contents.JPG");

            downloadTabBackgroundImageRectangle = new Rectangle(0, 0, downloadTabBackgroundImage.Width, downloadTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(downloadTabBackgroundImage, this.ClientRectangle, downloadTabBackgroundImageRectangle, GraphicsUnit.Pixel);
            // Drawing a transparent text
            using (Brush headingBrush = new SolidBrush(Color.Black), stringBrush = new SolidBrush(Color.Black))
            {
                Font font = new Font(FontFamily.GenericSerif, 8, FontStyle.Bold);
                int textWidth = (int)(g.MeasureString("+ Files to Download:", font).Width);
                g.DrawString("+ Files to Download:", font, headingBrush, new Rectangle(this.ClientRectangle.X + (int)(this.ClientRectangle.Width / 32), this.ClientRectangle.Y + (int)Math.Round(this.ClientRectangle.Height / 53.6), textWidth, textWidth));
            }

        }

        Bitmap localInfosTabBackgroundImage = null;
        Rectangle localInfosTabBackgroundImageRectangle;

        private void LocalInfos_Paint(object sender, PaintEventArgs e)
        {
            if (localInfosTabBackgroundImage == null)
                localInfosTabBackgroundImage = new Bitmap(@"\Program Files\TorrentF\contents.JPG");

            localInfosTabBackgroundImageRectangle = new Rectangle(0, 0, localInfosTabBackgroundImage.Width, localInfosTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(localInfosTabBackgroundImage, this.ClientRectangle, localInfosTabBackgroundImageRectangle, GraphicsUnit.Pixel);

        }

        Bitmap logTabBackgroundImage = null;
        Rectangle logTabBackgroundImageRectangle;

        private void Log_Paint(object sender, PaintEventArgs e)
        {
            if (logTabBackgroundImage == null)
                logTabBackgroundImage = new Bitmap(@"\Program Files\TorrentF\contents.JPG");

            logTabBackgroundImageRectangle = new Rectangle(0, 0, logTabBackgroundImage.Width, logTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(logTabBackgroundImage, this.ClientRectangle, logTabBackgroundImageRectangle, GraphicsUnit.Pixel);
            // Drawing a transparent text
            using (Brush headingBrush = new SolidBrush(Color.Black), stringBrush = new SolidBrush(Color.Black))
            {
                Font font = new Font(FontFamily.GenericSerif, 8, FontStyle.Bold);
                int textWidth = (int)(g.MeasureString("+ Log Entries:", font).Width);
                g.DrawString("+ Log Entries:", font, headingBrush, new Rectangle(this.ClientRectangle.X + (int)(this.ClientRectangle.Width / 32), this.ClientRectangle.Y + (int)Math.Round(this.ClientRectangle.Height / 53.6), textWidth, textWidth));
            }
        }

        Bitmap configTabBackgroundImage = null;
        Rectangle configTabBackgroundImageRectangle;

        private void tabConfig_Paint(object sender, PaintEventArgs e)
        {
            comboBoxPublishMethodConfig.SelectedIndex = filesManager.FilePublisher.PublishingMethod - 1;
            comboBoxLookUpMethod.SelectedIndex = 0;
            if (configTabBackgroundImage == null)
                configTabBackgroundImage = new Bitmap(@"\Program Files\TorrentF\contents.JPG");

            configTabBackgroundImageRectangle = new Rectangle(0, 0, configTabBackgroundImage.Width, configTabBackgroundImage.Height);
            Graphics g = e.Graphics;
            g.DrawImage(configTabBackgroundImage, this.ClientRectangle, configTabBackgroundImageRectangle, GraphicsUnit.Pixel);
            
            using (Brush headingBrush = new SolidBrush(Color.Black), stringBrush = new SolidBrush(Color.Black))
            {
                Font font = new Font(FontFamily.GenericSerif, 8, FontStyle.Bold);
                int textWidth = (int)(g.MeasureString("+ LookUp Method:", font).Width);
                g.DrawString("+ LookUp Method:", font, headingBrush, new Rectangle(this.ClientRectangle.X + (int)(this.ClientRectangle.Width / 32), this.ClientRectangle.Y + (int)Math.Round(this.ClientRectangle.Height/10.72), textWidth, textWidth));
                textWidth = (int)(g.MeasureString("+ Publishing Method:", font).Width);
                g.DrawString("+ Publishing Method:", font, headingBrush, new Rectangle(this.ClientRectangle.X + (int)(this.ClientRectangle.Width / 32), this.ClientRectangle.Y + (int)Math.Round(this.ClientRectangle.Height/2.43), textWidth, textWidth));
            }
        }

        private void comboBoxLookUpMethod_SelectedIndexChanged(object sender, EventArgs e)
        {

            if (comboBoxLookUpMethod.SelectedIndex == 0)
            {
                TorrentFConfig.GetConfig().useLocalTracker = true;
            }
            else
            {
                TorrentFConfig.GetConfig().useLocalTracker = false;
                textBoxFileName.Enabled = true;
            }
        
        
        }

    }
}
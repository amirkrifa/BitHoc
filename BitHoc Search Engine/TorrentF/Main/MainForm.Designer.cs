namespace TorrentF.Main
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.MainMenu mainMenu;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.mainMenu = new System.Windows.Forms.MainMenu();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.menuItemExit = new System.Windows.Forms.MenuItem();
            this.tabMainControl = new System.Windows.Forms.TabControl();
            this.Publish = new System.Windows.Forms.TabPage();
            this.listViewPublished = new System.Windows.Forms.ListView();
            this.columnFileName = new System.Windows.Forms.ColumnHeader();
            this.columnFileSize = new System.Windows.Forms.ColumnHeader();
            this.contextMenuPublishList = new System.Windows.Forms.ContextMenu();
            this.menuItemRetryPublishing = new System.Windows.Forms.MenuItem();
            this.menuItemDelete = new System.Windows.Forms.MenuItem();
            this.menuItemStatus = new System.Windows.Forms.MenuItem();
            this.menuItemRelatedDataDescription = new System.Windows.Forms.MenuItem();
            this.publishFile = new System.Windows.Forms.Button();
            this.Find = new System.Windows.Forms.TabPage();
            this.listViewFound = new System.Windows.Forms.ListView();
            this.columnHeaderFileName = new System.Windows.Forms.ColumnHeader();
            this.columnHeaderRemoteHost = new System.Windows.Forms.ColumnHeader();
            this.contextMenuFindList = new System.Windows.Forms.ContextMenu();
            this.menuItemRelatedTorrentDetails = new System.Windows.Forms.MenuItem();
            this.menuItemFDDelete = new System.Windows.Forms.MenuItem();
            this.buttonSelectFileForDownloading = new System.Windows.Forms.Button();
            this.buttonFind = new System.Windows.Forms.Button();
            this.textBoxFileName = new System.Windows.Forms.TextBox();
            this.download = new System.Windows.Forms.TabPage();
            this.buttonRemoveFile = new System.Windows.Forms.Button();
            this.buttonDownloadFile = new System.Windows.Forms.Button();
            this.listViewDownload = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.contextMenuDownload = new System.Windows.Forms.ContextMenu();
            this.menuItemGetFullPath = new System.Windows.Forms.MenuItem();
            this.menuItemPublish = new System.Windows.Forms.MenuItem();
            this.menuItemSelectRemoteHost = new System.Windows.Forms.MenuItem();
            this.menuItemDLGetTorrentDetails = new System.Windows.Forms.MenuItem();
            this.LocalInfos = new System.Windows.Forms.TabPage();
            this.buttonUpdateLocalInfos = new System.Windows.Forms.Button();
            this.textBoxLocalInfos = new System.Windows.Forms.TextBox();
            this.Log = new System.Windows.Forms.TabPage();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.tabConfig = new System.Windows.Forms.TabPage();
            this.comboBoxLookUpMethod = new System.Windows.Forms.ComboBox();
            this.comboBoxPublishMethodConfig = new System.Windows.Forms.ComboBox();
            this.About = new System.Windows.Forms.TabPage();
            this.openFileDialogPublish = new System.Windows.Forms.OpenFileDialog();
            this.tabMainControl.SuspendLayout();
            this.Publish.SuspendLayout();
            this.Find.SuspendLayout();
            this.download.SuspendLayout();
            this.LocalInfos.SuspendLayout();
            this.Log.SuspendLayout();
            this.tabConfig.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.Add(this.menuItem1);
            // 
            // menuItem1
            // 
            this.menuItem1.MenuItems.Add(this.menuItemExit);
            this.menuItem1.Text = "Menu";
            // 
            // menuItemExit
            // 
            this.menuItemExit.Text = "Exit";
            this.menuItemExit.Click += new System.EventHandler(this.menuItemExit_Click_1);
            // 
            // tabMainControl
            // 
            this.tabMainControl.Controls.Add(this.Publish);
            this.tabMainControl.Controls.Add(this.Find);
            this.tabMainControl.Controls.Add(this.download);
            this.tabMainControl.Controls.Add(this.LocalInfos);
            this.tabMainControl.Controls.Add(this.Log);
            this.tabMainControl.Controls.Add(this.tabConfig);
            this.tabMainControl.Controls.Add(this.About);
            this.tabMainControl.Font = new System.Drawing.Font("Verdana", 8F, System.Drawing.FontStyle.Bold);
            this.tabMainControl.Location = new System.Drawing.Point(0, 0);
            this.tabMainControl.Name = "tabMainControl";
            this.tabMainControl.SelectedIndex = 0;
            this.tabMainControl.Size = new System.Drawing.Size(240, 265);
            this.tabMainControl.TabIndex = 0;
            this.tabMainControl.SelectedIndexChanged += new System.EventHandler(this.tabMainControl_SelectedIndexChanged);
            // 
            // Publish
            // 
            this.Publish.BackColor = System.Drawing.Color.LightCyan;
            this.Publish.Controls.Add(this.listViewPublished);
            this.Publish.Controls.Add(this.publishFile);
            this.Publish.Location = new System.Drawing.Point(0, 0);
            this.Publish.Name = "Publish";
            this.Publish.Size = new System.Drawing.Size(240, 242);
            this.Publish.Text = "Publish";
            this.Publish.Paint += new System.Windows.Forms.PaintEventHandler(this.Publish_Paint);
            // 
            // listViewPublished
            // 
            this.listViewPublished.Activation = System.Windows.Forms.ItemActivation.OneClick;
            this.listViewPublished.Columns.Add(this.columnFileName);
            this.listViewPublished.Columns.Add(this.columnFileSize);
            this.listViewPublished.ContextMenu = this.contextMenuPublishList;
            this.listViewPublished.Location = new System.Drawing.Point(7, 70);
            this.listViewPublished.Name = "listViewPublished";
            this.listViewPublished.Size = new System.Drawing.Size(226, 156);
            this.listViewPublished.TabIndex = 3;
            this.listViewPublished.View = System.Windows.Forms.View.Details;
            // 
            // columnFileName
            // 
            this.columnFileName.Text = "File Name";
            this.columnFileName.Width = 113;
            // 
            // columnFileSize
            // 
            this.columnFileSize.Text = "File Size(byte)";
            this.columnFileSize.Width = 110;
            // 
            // contextMenuPublishList
            // 
            this.contextMenuPublishList.MenuItems.Add(this.menuItemRetryPublishing);
            this.contextMenuPublishList.MenuItems.Add(this.menuItemDelete);
            this.contextMenuPublishList.MenuItems.Add(this.menuItemStatus);
            this.contextMenuPublishList.MenuItems.Add(this.menuItemRelatedDataDescription);
            // 
            // menuItemRetryPublishing
            // 
            this.menuItemRetryPublishing.Text = "Retry Publishing";
            this.menuItemRetryPublishing.Click += new System.EventHandler(this.menuItemRetryPublishing_Click);
            // 
            // menuItemDelete
            // 
            this.menuItemDelete.Text = "Delete";
            this.menuItemDelete.Click += new System.EventHandler(this.menuItemDelete_Click);
            // 
            // menuItemStatus
            // 
            this.menuItemStatus.Text = "Status";
            this.menuItemStatus.Click += new System.EventHandler(this.menuItemStatus_Click);
            // 
            // menuItemRelatedDataDescription
            // 
            this.menuItemRelatedDataDescription.Text = "Related Data Description";
            this.menuItemRelatedDataDescription.Click += new System.EventHandler(this.menuItemRelatedDataDescription_Click);
            // 
            // publishFile
            // 
            this.publishFile.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.publishFile.Location = new System.Drawing.Point(7, 13);
            this.publishFile.Name = "publishFile";
            this.publishFile.Size = new System.Drawing.Size(181, 20);
            this.publishFile.TabIndex = 2;
            this.publishFile.Text = "Select a new file for publishing";
            this.publishFile.Click += new System.EventHandler(this.publishFile_Click);
            // 
            // Find
            // 
            this.Find.BackColor = System.Drawing.Color.LightCyan;
            this.Find.Controls.Add(this.listViewFound);
            this.Find.Controls.Add(this.buttonSelectFileForDownloading);
            this.Find.Controls.Add(this.buttonFind);
            this.Find.Controls.Add(this.textBoxFileName);
            this.Find.Location = new System.Drawing.Point(0, 0);
            this.Find.Name = "Find";
            this.Find.Size = new System.Drawing.Size(232, 239);
            this.Find.Text = "Find";
            this.Find.Paint += new System.Windows.Forms.PaintEventHandler(this.Find_Paint);
            // 
            // listViewFound
            // 
            this.listViewFound.Columns.Add(this.columnHeaderFileName);
            this.listViewFound.Columns.Add(this.columnHeaderRemoteHost);
            this.listViewFound.ContextMenu = this.contextMenuFindList;
            this.listViewFound.Location = new System.Drawing.Point(7, 63);
            this.listViewFound.Name = "listViewFound";
            this.listViewFound.Size = new System.Drawing.Size(226, 126);
            this.listViewFound.TabIndex = 5;
            this.listViewFound.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderFileName
            // 
            this.columnHeaderFileName.Text = "FileName";
            this.columnHeaderFileName.Width = 100;
            // 
            // columnHeaderRemoteHost
            // 
            this.columnHeaderRemoteHost.Text = "Host Ip";
            this.columnHeaderRemoteHost.Width = 126;
            // 
            // contextMenuFindList
            // 
            this.contextMenuFindList.MenuItems.Add(this.menuItemRelatedTorrentDetails);
            this.contextMenuFindList.MenuItems.Add(this.menuItemFDDelete);
            // 
            // menuItemRelatedTorrentDetails
            // 
            this.menuItemRelatedTorrentDetails.Text = "Get Torrent Details";
            this.menuItemRelatedTorrentDetails.Click += new System.EventHandler(this.menuItemRelatedTorrentDetails_Click);
            // 
            // menuItemFDDelete
            // 
            this.menuItemFDDelete.Text = "Delete";
            this.menuItemFDDelete.Click += new System.EventHandler(this.menuItemFDDelete_Click);
            // 
            // buttonSelectFileForDownloading
            // 
            this.buttonSelectFileForDownloading.Location = new System.Drawing.Point(7, 216);
            this.buttonSelectFileForDownloading.Name = "buttonSelectFileForDownloading";
            this.buttonSelectFileForDownloading.Size = new System.Drawing.Size(226, 18);
            this.buttonSelectFileForDownloading.TabIndex = 4;
            this.buttonSelectFileForDownloading.Text = "Select File for Downloading";
            this.buttonSelectFileForDownloading.Click += new System.EventHandler(this.buttonSelectFileForDownloading_Click);
            // 
            // buttonFind
            // 
            this.buttonFind.Location = new System.Drawing.Point(7, 193);
            this.buttonFind.Name = "buttonFind";
            this.buttonFind.Size = new System.Drawing.Size(226, 19);
            this.buttonFind.TabIndex = 3;
            this.buttonFind.Text = "Find";
            this.buttonFind.Click += new System.EventHandler(this.buttonFind_Click);
            // 
            // textBoxFileName
            // 
            this.textBoxFileName.Location = new System.Drawing.Point(7, 32);
            this.textBoxFileName.Name = "textBoxFileName";
            this.textBoxFileName.Size = new System.Drawing.Size(226, 21);
            this.textBoxFileName.TabIndex = 1;
            // 
            // download
            // 
            this.download.BackColor = System.Drawing.Color.LightCyan;
            this.download.Controls.Add(this.buttonRemoveFile);
            this.download.Controls.Add(this.buttonDownloadFile);
            this.download.Controls.Add(this.listViewDownload);
            this.download.Location = new System.Drawing.Point(0, 0);
            this.download.Name = "download";
            this.download.Size = new System.Drawing.Size(240, 242);
            this.download.Text = "Download";
            this.download.Paint += new System.Windows.Forms.PaintEventHandler(this.download_Paint);
            // 
            // buttonRemoveFile
            // 
            this.buttonRemoveFile.Location = new System.Drawing.Point(137, 201);
            this.buttonRemoveFile.Name = "buttonRemoveFile";
            this.buttonRemoveFile.Size = new System.Drawing.Size(92, 20);
            this.buttonRemoveFile.TabIndex = 2;
            this.buttonRemoveFile.Text = "Remove";
            this.buttonRemoveFile.Click += new System.EventHandler(this.buttonRemoveFile_Click);
            // 
            // buttonDownloadFile
            // 
            this.buttonDownloadFile.Location = new System.Drawing.Point(13, 201);
            this.buttonDownloadFile.Name = "buttonDownloadFile";
            this.buttonDownloadFile.Size = new System.Drawing.Size(92, 20);
            this.buttonDownloadFile.TabIndex = 1;
            this.buttonDownloadFile.Text = "Download";
            this.buttonDownloadFile.Click += new System.EventHandler(this.buttonDownloadFile_Click);
            // 
            // listViewDownload
            // 
            this.listViewDownload.Columns.Add(this.columnHeader1);
            this.listViewDownload.Columns.Add(this.columnHeader2);
            this.listViewDownload.Columns.Add(this.columnHeader3);
            this.listViewDownload.ContextMenu = this.contextMenuDownload;
            this.listViewDownload.Location = new System.Drawing.Point(13, 30);
            this.listViewDownload.Name = "listViewDownload";
            this.listViewDownload.Size = new System.Drawing.Size(216, 151);
            this.listViewDownload.TabIndex = 0;
            this.listViewDownload.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "F. Name";
            this.columnHeader1.Width = 70;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "F. Size";
            this.columnHeader2.Width = 70;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Down Bytes";
            this.columnHeader3.Width = 80;
            // 
            // contextMenuDownload
            // 
            this.contextMenuDownload.MenuItems.Add(this.menuItemGetFullPath);
            this.contextMenuDownload.MenuItems.Add(this.menuItemPublish);
            this.contextMenuDownload.MenuItems.Add(this.menuItemSelectRemoteHost);
            this.contextMenuDownload.MenuItems.Add(this.menuItemDLGetTorrentDetails);
            // 
            // menuItemGetFullPath
            // 
            this.menuItemGetFullPath.Text = "Get Full Path";
            this.menuItemGetFullPath.Click += new System.EventHandler(this.menuItemGetFullPath_Click);
            // 
            // menuItemPublish
            // 
            this.menuItemPublish.Text = "Publish";
            this.menuItemPublish.Click += new System.EventHandler(this.menuItemPublish_Click);
            // 
            // menuItemSelectRemoteHost
            // 
            this.menuItemSelectRemoteHost.Text = "Select Remote Host";
            this.menuItemSelectRemoteHost.Click += new System.EventHandler(this.menuItemSelectRemoteHost_Click);
            // 
            // menuItemDLGetTorrentDetails
            // 
            this.menuItemDLGetTorrentDetails.Text = "Get Torrent Details";
            this.menuItemDLGetTorrentDetails.Click += new System.EventHandler(this.menuItemDLGetTorrentDetails_Click);
            // 
            // LocalInfos
            // 
            this.LocalInfos.BackColor = System.Drawing.Color.LightCyan;
            this.LocalInfos.Controls.Add(this.buttonUpdateLocalInfos);
            this.LocalInfos.Controls.Add(this.textBoxLocalInfos);
            this.LocalInfos.Location = new System.Drawing.Point(0, 0);
            this.LocalInfos.Name = "LocalInfos";
            this.LocalInfos.Size = new System.Drawing.Size(232, 239);
            this.LocalInfos.Text = "Local Infos";
            this.LocalInfos.Paint += new System.Windows.Forms.PaintEventHandler(this.LocalInfos_Paint);
            // 
            // buttonUpdateLocalInfos
            // 
            this.buttonUpdateLocalInfos.Location = new System.Drawing.Point(7, 208);
            this.buttonUpdateLocalInfos.Name = "buttonUpdateLocalInfos";
            this.buttonUpdateLocalInfos.Size = new System.Drawing.Size(223, 20);
            this.buttonUpdateLocalInfos.TabIndex = 1;
            this.buttonUpdateLocalInfos.Text = "Update";
            this.buttonUpdateLocalInfos.Click += new System.EventHandler(this.buttonUpdateLocalInfos_Click);
            // 
            // textBoxLocalInfos
            // 
            this.textBoxLocalInfos.Location = new System.Drawing.Point(8, 19);
            this.textBoxLocalInfos.Multiline = true;
            this.textBoxLocalInfos.Name = "textBoxLocalInfos";
            this.textBoxLocalInfos.ReadOnly = true;
            this.textBoxLocalInfos.Size = new System.Drawing.Size(222, 182);
            this.textBoxLocalInfos.TabIndex = 0;
            // 
            // Log
            // 
            this.Log.BackColor = System.Drawing.Color.LightCyan;
            this.Log.Controls.Add(this.textBoxLog);
            this.Log.Location = new System.Drawing.Point(0, 0);
            this.Log.Name = "Log";
            this.Log.Size = new System.Drawing.Size(232, 239);
            this.Log.Text = "Log";
            this.Log.Paint += new System.Windows.Forms.PaintEventHandler(this.Log_Paint);
            // 
            // textBoxLog
            // 
            this.textBoxLog.Location = new System.Drawing.Point(13, 37);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.Size = new System.Drawing.Size(189, 118);
            this.textBoxLog.TabIndex = 0;
            // 
            // tabConfig
            // 
            this.tabConfig.BackColor = System.Drawing.Color.LightCyan;
            this.tabConfig.Controls.Add(this.comboBoxLookUpMethod);
            this.tabConfig.Controls.Add(this.comboBoxPublishMethodConfig);
            this.tabConfig.Location = new System.Drawing.Point(0, 0);
            this.tabConfig.Name = "tabConfig";
            this.tabConfig.Size = new System.Drawing.Size(232, 239);
            this.tabConfig.Text = "Config";
            this.tabConfig.Paint += new System.Windows.Forms.PaintEventHandler(this.tabConfig_Paint);
            // 
            // comboBoxLookUpMethod
            // 
            this.comboBoxLookUpMethod.DisplayMember = "LookUp via BitHoc Tracker";
            this.comboBoxLookUpMethod.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.comboBoxLookUpMethod.Items.Add("LookUp via BitHoc Tracker");
            this.comboBoxLookUpMethod.Items.Add("LookUp via the DHT");
            this.comboBoxLookUpMethod.Location = new System.Drawing.Point(7, 130);
            this.comboBoxLookUpMethod.Name = "comboBoxLookUpMethod";
            this.comboBoxLookUpMethod.Size = new System.Drawing.Size(226, 20);
            this.comboBoxLookUpMethod.TabIndex = 15;
            this.comboBoxLookUpMethod.SelectedIndexChanged += new System.EventHandler(this.comboBoxLookUpMethod_SelectedIndexChanged);
            // 
            // comboBoxPublishMethodConfig
            // 
            this.comboBoxPublishMethodConfig.DisplayMember = "Publish file via BitHoc Tracker";
            this.comboBoxPublishMethodConfig.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.comboBoxPublishMethodConfig.Items.Add("Publish file via BitHoc Tracker");
            this.comboBoxPublishMethodConfig.Items.Add("Publish file via the DHT");
            this.comboBoxPublishMethodConfig.Location = new System.Drawing.Point(7, 50);
            this.comboBoxPublishMethodConfig.Name = "comboBoxPublishMethodConfig";
            this.comboBoxPublishMethodConfig.Size = new System.Drawing.Size(226, 20);
            this.comboBoxPublishMethodConfig.TabIndex = 0;
            this.comboBoxPublishMethodConfig.SelectedIndexChanged += new System.EventHandler(this.comboBoxPublishMethodConfig_SelectedIndexChanged);
            // 
            // About
            // 
            this.About.BackColor = System.Drawing.Color.Transparent;
            this.About.Location = new System.Drawing.Point(0, 0);
            this.About.Name = "About";
            this.About.Size = new System.Drawing.Size(232, 239);
            this.About.Text = "About";
            this.About.Paint += new System.Windows.Forms.PaintEventHandler(this.About_Paint);
            // 
            // openFileDialogPublish
            // 
            this.openFileDialogPublish.Filter = "Data Files|*.*";
            this.openFileDialogPublish.InitialDirectory = ".\\";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(240, 268);
            this.Controls.Add(this.tabMainControl);
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "BitHoc Search Engine";
            this.Closing += new System.ComponentModel.CancelEventHandler(this.MainForm_Closing);
            this.tabMainControl.ResumeLayout(false);
            this.Publish.ResumeLayout(false);
            this.Find.ResumeLayout(false);
            this.download.ResumeLayout(false);
            this.LocalInfos.ResumeLayout(false);
            this.Log.ResumeLayout(false);
            this.tabConfig.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabMainControl;
        private System.Windows.Forms.TabPage Publish;
        private System.Windows.Forms.TabPage Find;
        private System.Windows.Forms.TabPage Log;
        private System.Windows.Forms.TabPage About;
        private System.Windows.Forms.TabPage LocalInfos;
        private System.Windows.Forms.Button publishFile;
        private System.Windows.Forms.Button buttonSelectFileForDownloading;
        private System.Windows.Forms.Button buttonFind;
        private System.Windows.Forms.TextBox textBoxFileName;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.TabPage download;
        private System.Windows.Forms.OpenFileDialog openFileDialogPublish;
        private System.Windows.Forms.ListView listViewFound;
        private System.Windows.Forms.Button buttonDownloadFile;
        private System.Windows.Forms.ListView listViewDownload;
        private System.Windows.Forms.Button buttonRemoveFile;
        private System.Windows.Forms.ColumnHeader columnHeaderFileName;
        private System.Windows.Forms.ColumnHeader columnHeaderRemoteHost;
        private System.Windows.Forms.ColumnHeader columnFileName;
        private System.Windows.Forms.ColumnHeader columnFileSize;
        private System.Windows.Forms.ContextMenu contextMenuPublishList;
        private System.Windows.Forms.MenuItem menuItemRetryPublishing;
        private System.Windows.Forms.MenuItem menuItemDelete;
        private System.Windows.Forms.ListView listViewPublished;
        private System.Windows.Forms.MenuItem menuItemStatus;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Button buttonUpdateLocalInfos;
        private System.Windows.Forms.TextBox textBoxLocalInfos;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ContextMenu contextMenuDownload;
        private System.Windows.Forms.MenuItem menuItemGetFullPath;
        private System.Windows.Forms.MenuItem menuItemPublish;
        private System.Windows.Forms.ContextMenu contextMenuFindList;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItemExit;
        private System.Windows.Forms.MenuItem menuItemSelectRemoteHost;
        private System.Windows.Forms.MenuItem menuItemRelatedTorrentDetails;
        private System.Windows.Forms.MenuItem menuItemRelatedDataDescription;
        private System.Windows.Forms.MenuItem menuItemDLGetTorrentDetails;
        private System.Windows.Forms.MenuItem menuItemFDDelete;
        private System.Windows.Forms.TabPage tabConfig;
        private System.Windows.Forms.ComboBox comboBoxPublishMethodConfig;
        private System.Windows.Forms.ComboBox comboBoxLookUpMethod;
    }
}


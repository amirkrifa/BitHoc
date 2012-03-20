namespace TorrentF.RelatedForms
{
    partial class PossibleRemoteHostsForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.MainMenu mainMenu1;

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
            this.mainMenu1 = new System.Windows.Forms.MainMenu();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.treeViewHosts = new System.Windows.Forms.TreeView();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // mainMenu1
            // 
            this.mainMenu1.MenuItems.Add(this.menuItem1);
            // 
            // menuItem1
            // 
            this.menuItem1.MenuItems.Add(this.menuItem2);
            this.menuItem1.Text = "Menu";
            // 
            // menuItem2
            // 
            this.menuItem2.Text = "Select and Close";
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
            // 
            // treeViewHosts
            // 
            this.treeViewHosts.Indent = 20;
            this.treeViewHosts.Location = new System.Drawing.Point(18, 47);
            this.treeViewHosts.Name = "treeViewHosts";
            this.treeViewHosts.Size = new System.Drawing.Size(200, 200);
            this.treeViewHosts.TabIndex = 0;
            this.treeViewHosts.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewHosts_AfterSelect);
            this.treeViewHosts.AfterExpand += new System.Windows.Forms.TreeViewEventHandler(this.treeViewHosts_AfterExpand);
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.label1.Location = new System.Drawing.Point(8, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(229, 20);
            this.label1.Text = "- List of Possible Remote Hosts:";
            // 
            // PossibleRemoteHostsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.BackColor = System.Drawing.Color.LightCyan;
            this.ClientSize = new System.Drawing.Size(240, 268);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.treeViewHosts);
            this.KeyPreview = true;
            this.Menu = this.mainMenu1;
            this.Name = "PossibleRemoteHostsForm";
            this.Text = "Possible Remote Hosts";
            this.Closed += new System.EventHandler(this.PossibleRemoteHostsForm_Closed);
            this.Closing += new System.ComponentModel.CancelEventHandler(this.PossibleRemoteHostsForm_Closing);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.PossibleRemoteHostsForm_KeyDown);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TreeView treeViewHosts;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;
    }
}
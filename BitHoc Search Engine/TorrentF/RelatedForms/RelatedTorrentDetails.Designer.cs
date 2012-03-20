namespace TorrentF.RelatedForms
{
    partial class RelatedTorrentDetails
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
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxNumberOfSeeders = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxNumberOfLeechers = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxNumberOfInvolvedPeers = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.textBoxDataDescription = new System.Windows.Forms.TextBox();
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
            this.menuItem2.Text = "Close";
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.label1.Location = new System.Drawing.Point(2, 51);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(153, 20);
            this.label1.Text = "+Nbr. of seeders:";
            // 
            // textBoxNumberOfSeeders
            // 
            this.textBoxNumberOfSeeders.Location = new System.Drawing.Point(173, 47);
            this.textBoxNumberOfSeeders.Name = "textBoxNumberOfSeeders";
            this.textBoxNumberOfSeeders.ReadOnly = true;
            this.textBoxNumberOfSeeders.Size = new System.Drawing.Size(44, 25);
            this.textBoxNumberOfSeeders.TabIndex = 1;
            // 
            // label2
            // 
            this.label2.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.label2.Location = new System.Drawing.Point(2, 86);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(158, 20);
            this.label2.Text = "+Nbr. of leechers:";
            // 
            // textBoxNumberOfLeechers
            // 
            this.textBoxNumberOfLeechers.Location = new System.Drawing.Point(173, 83);
            this.textBoxNumberOfLeechers.Name = "textBoxNumberOfLeechers";
            this.textBoxNumberOfLeechers.ReadOnly = true;
            this.textBoxNumberOfLeechers.Size = new System.Drawing.Size(44, 25);
            this.textBoxNumberOfLeechers.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.label3.Location = new System.Drawing.Point(3, 16);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(206, 20);
            this.label3.Text = "+Nbr. of involved peers:";
            // 
            // textBoxNumberOfInvolvedPeers
            // 
            this.textBoxNumberOfInvolvedPeers.Location = new System.Drawing.Point(173, 11);
            this.textBoxNumberOfInvolvedPeers.Name = "textBoxNumberOfInvolvedPeers";
            this.textBoxNumberOfInvolvedPeers.ReadOnly = true;
            this.textBoxNumberOfInvolvedPeers.Size = new System.Drawing.Size(44, 25);
            this.textBoxNumberOfInvolvedPeers.TabIndex = 8;
            // 
            // label4
            // 
            this.label4.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.label4.Location = new System.Drawing.Point(3, 116);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(174, 20);
            this.label4.Text = "+Data description:";
            // 
            // textBoxDataDescription
            // 
            this.textBoxDataDescription.Location = new System.Drawing.Point(15, 139);
            this.textBoxDataDescription.Multiline = true;
            this.textBoxDataDescription.Name = "textBoxDataDescription";
            this.textBoxDataDescription.ReadOnly = true;
            this.textBoxDataDescription.Size = new System.Drawing.Size(202, 77);
            this.textBoxDataDescription.TabIndex = 13;
            this.textBoxDataDescription.TextChanged += new System.EventHandler(this.textBoxDataDescription_TextChanged);
            // 
            // RelatedTorrentDetails
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.BackColor = System.Drawing.Color.LightCyan;
            this.ClientSize = new System.Drawing.Size(220, 230);
            this.Controls.Add(this.textBoxDataDescription);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.textBoxNumberOfInvolvedPeers);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.textBoxNumberOfLeechers);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.textBoxNumberOfSeeders);
            this.Controls.Add(this.label1);
            this.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.Menu = this.mainMenu1;
            this.Name = "RelatedTorrentDetails";
            this.Text = "Related Torrent Details";
            this.Load += new System.EventHandler(this.RelatedTorrentDetails_Load);
            this.Closed += new System.EventHandler(this.RelatedTorrentDetails_Closed);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxNumberOfSeeders;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxNumberOfLeechers;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxNumberOfInvolvedPeers;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox textBoxDataDescription;
    }
}
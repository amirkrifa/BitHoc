namespace TorrentF.RelatedForms
{
    partial class FileDescription
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.MainMenu mainMenuFileDescription;

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
            this.mainMenuFileDescription = new System.Windows.Forms.MainMenu();
            this.menuItemMenuFileDescription = new System.Windows.Forms.MenuItem();
            this.menuItemFileDescriptionSave = new System.Windows.Forms.MenuItem();
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxDescription = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // mainMenuFileDescription
            // 
            this.mainMenuFileDescription.MenuItems.Add(this.menuItemMenuFileDescription);
            // 
            // menuItemMenuFileDescription
            // 
            this.menuItemMenuFileDescription.MenuItems.Add(this.menuItemFileDescriptionSave);
            this.menuItemMenuFileDescription.Text = "Menu";
            // 
            // menuItemFileDescriptionSave
            // 
            this.menuItemFileDescriptionSave.Text = "Save and Close";
            this.menuItemFileDescriptionSave.Click += new System.EventHandler(this.menuItemFileDescriptionSave_Click);
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.label1.Location = new System.Drawing.Point(3, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(166, 20);
            this.label1.Text = "+ File Description:";
            // 
            // textBoxDescription
            // 
            this.textBoxDescription.Location = new System.Drawing.Point(12, 38);
            this.textBoxDescription.Multiline = true;
            this.textBoxDescription.Name = "textBoxDescription";
            this.textBoxDescription.Size = new System.Drawing.Size(189, 155);
            this.textBoxDescription.TabIndex = 1;
            this.textBoxDescription.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBoxDescription_KeyPress);
            // 
            // FileDescription
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.BackColor = System.Drawing.Color.LightCyan;
            this.ClientSize = new System.Drawing.Size(220, 230);
            this.Controls.Add(this.textBoxDescription);
            this.Controls.Add(this.label1);
            this.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.Menu = this.mainMenuFileDescription;
            this.Name = "FileDescription";
            this.Text = "Data File Description";
            this.Closed += new System.EventHandler(this.FileDescription_Closed);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxDescription;
        private System.Windows.Forms.MenuItem menuItemMenuFileDescription;
        private System.Windows.Forms.MenuItem menuItemFileDescriptionSave;
    }
}
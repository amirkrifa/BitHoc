using System;

using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using TorrentF.Utilities;
using TorrentF.FilesStatus;
namespace TorrentF.RelatedForms
{
    public partial class FileDescription : Form
    {
        public FileDescription()
        {
            InitializeComponent();
        }

        public string description = null;
        private void menuItemFileDescriptionSave_Click(object sender, EventArgs e)
        {
            description = textBoxDescription.Text;
            this.Close();
        }

        // prevent the use of special characters while typing the related torrent file description
        private void textBoxDescription_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (TorrentFConfig.GetConfig().IsPrivateCharacter(e.KeyChar))
            {
                MessageBox.Show("Special reserved character.", e.KeyChar.ToString(), MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);
                e.Handled = true;
            }
        }

        private void FileDescription_Closed(object sender, EventArgs e)
        {
            description = textBoxDescription.Text;
        }
    }
}
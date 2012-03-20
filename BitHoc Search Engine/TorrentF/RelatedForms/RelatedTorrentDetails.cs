using System;

using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using TorrentF.FilesStatus;

namespace TorrentF.RelatedForms
{
    public partial class RelatedTorrentDetails : Form
    {
        private FileDetails fd = null;
        public RelatedTorrentDetails(ref FileDetails _fd)
        {
            InitializeComponent();
            fd = _fd;
        }

       
        private void menuItem2_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void textBoxDataDescription_TextChanged(object sender, EventArgs e)
        {

        }

        private void RelatedTorrentDetails_Load(object sender, EventArgs e)
        {
            if (fd.SessionDetails != null)
            {
                textBoxNumberOfInvolvedPeers.Text = fd.SessionDetails.NumberOfInvolvedPeers.ToString();
                textBoxNumberOfLeechers.Text = fd.SessionDetails.NumberOfLeechers.ToString();
                textBoxNumberOfSeeders.Text = fd.SessionDetails.NumberOfSeerders.ToString();
            }
            textBoxDataDescription.Text = fd.DataDescription;

        }

        private void RelatedTorrentDetails_Closed(object sender, EventArgs e)
        {

        }
    }
}
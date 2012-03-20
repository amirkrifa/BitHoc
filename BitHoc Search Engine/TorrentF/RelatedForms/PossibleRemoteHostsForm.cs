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
    public partial class PossibleRemoteHostsForm : Form
    {
        private FileDetails fd = null;
        public PossibleRemoteHostsForm(ref FileDetails _fd)
        {
            InitializeComponent();
            fd = _fd;
        }


        public System.Windows.Forms.TreeView TreeViewHosts
        {
            get
            {
                return treeViewHosts;
            }
        }
        private void treeViewHosts_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Action == TreeViewAction.ByMouse)
            {
                if (fd != null)
                {
                    int i = e.Node.Text.IndexOf(':');
                    if (i == -1)
                    {
                        fd.RemoteHostIp = e.Node.Text;
                    }
                }
                else
                {
                    MessageBox.Show("Related file not found.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);

                }
            }
        }

        private void PossibleRemoteHostsForm_Closed(object sender, EventArgs e)
        {

        }

        private void treeViewHosts_AfterExpand(object sender, TreeViewEventArgs e)
        {
            if (e.Action == TreeViewAction.ByMouse)
            {
                if (fd != null)
                {
                    int i = e.Node.Text.IndexOf(':');
                    if (i == -1)
                    {
                        fd.RemoteHostIp = e.Node.Text;
                    }
                }
                else
                {
                    MessageBox.Show("Related file not found.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Hand, MessageBoxDefaultButton.Button1);

                }
            }
        }

        private void PossibleRemoteHostsForm_Closing(object sender, CancelEventArgs e)
        {
       
        }

        private void PossibleRemoteHostsForm_KeyDown(object sender, KeyEventArgs e)
        {
            if ((e.KeyCode == System.Windows.Forms.Keys.Up))
            {
                // Rocker Up
                // Up
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Down))
            {
                // Rocker Down
                // Down
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Left))
            {
                // Left
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Right))
            {
                // Right
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Enter))
            {
                // Enter
            }

        }

        private void menuItem2_Click(object sender, EventArgs e)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("The selected remote host for downloading is: ");
            sb.Append(fd.RemoteHostIp);

            MessageBox.Show(sb.ToString(), "Infos", MessageBoxButtons.OK, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button1);
            this.Close();
        }


    }
}
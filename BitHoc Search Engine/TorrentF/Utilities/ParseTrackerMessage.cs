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
using System.Windows.Forms;
using TorrentF.Managers;
using TorrentF.FilesStatus;
using System.Diagnostics;
namespace TorrentF.Utilities
{
    
    class ParseTrackerMessage
    {
        private List<FileDetails> resultingFiles = null;
        public List<FileDetails> ResultingFiles
        {
            get
            {
                return resultingFiles;
            }
        }
        public ParseTrackerMessage()
        {
            resultingFiles = new List<FileDetails>();
        }

        public void ParseSingleFileMessage(string message, ref string existingFileName)
        {
            try
            {
                // Single file format: node1Ip#file1*file1.size*file1.description*file1.numberofleechers*file1.numberofseeders \r\n
                string nodeIp = null;
                string fileName = null;
                string fileDescription = null;
                long fileSize = 0;
                int numberOfLeechers = 0;
                int numberOfSeeders = 0;

                string tmp = null;
                Int32 dz = message.IndexOf('#');
                Trace.Assert(dz > 0, "ParseTrackerMessage::ParseSingleFileMessage, received message does not correspond to the template: " + message);
                nodeIp = message.Substring(0, dz );

                // Get the file name
                tmp = message.Substring(dz + 1);
                dz = tmp.IndexOf('*');
                Trace.Assert(dz > 0, "ParseTrackerMessage::ParseSingleFileMessage, received message does not correspond to the template: " + message);
                fileName = tmp.Substring(0, dz );

                // Get the file size
                tmp = tmp.Substring(dz + 1);
                dz = tmp.IndexOf('*');
                Trace.Assert(dz > 0, "ParseTrackerMessage::ParseSingleFileMessage, received message does not correspond to the template: " + message);
                fileSize = long.Parse(tmp.Substring(0, dz));

                // Get the file description
                tmp = tmp.Substring(dz + 1);
                dz = tmp.IndexOf('*');
                Trace.Assert(dz > 0, "ParseTrackerMessage::ParseSingleFileMessage, received message does not correspond to the template: " + message);
                fileDescription = tmp.Substring(0, dz);
                
                //Get the number of leechers
                tmp = tmp.Substring(dz + 1);
                dz = tmp.IndexOf('*');
                Trace.Assert(dz > 0, "ParseTrackerMessage::ParseSingleFileMessage, received message does not correspond to the template: " + message);
                numberOfLeechers = Int32.Parse(tmp.Substring(0, dz));

                //Get the number of seeders
                tmp = tmp.Substring(dz + 1);
                numberOfSeeders = Int32.Parse(tmp);

                int remotePort = TorrentFConfig.GetConfig().uploadingServerPort;
                FileDetails fd = FilesManager.GetFileManager().GetFileDetails(ref fileName);
                bool exists = false;

                if (fd == null)
                {
                    // Verify the local list

                    foreach (FileDetails x in resultingFiles)
                    {
                        if (x.FileName.CompareTo(fileName) == 0)
                        {
                            exists = true;
                            fd = x;
                            break;
                        }
                    }
                }
                else
                {
                    exists = true;
                }

                if(!exists)
                {
                    fd = new FileDetails(false, fileName, fileSize, nodeIp, remotePort, "");
                    
                    // Append the related data file description
                    string dfd = fileDescription;
                    fd.DataDescription = dfd;

                    // Append the related sharing session details
                    SharingSessionDetails ssd = new SharingSessionDetails();
                    ssd.NumberOfLeechers = numberOfLeechers;
                    ssd.NumberOfSeerders = numberOfSeeders;
                    fd.AppendRelatedSessionDetails(ref ssd);

                    resultingFiles.Add(fd);
                }
                else
                {
                    // File already available 
                    // Just add a possible Downloading remote host, the number of seeders and the number of leecherss
                    fd.AddHostCoordinates(ref nodeIp, ref remotePort);
                    fd.SessionDetails.NumberOfLeechers = numberOfLeechers;
                    fd.SessionDetails.NumberOfSeerders = numberOfSeeders;
                  
                    // Specify the existing fileName
                    existingFileName = fileName;
                }
                    
                
            }

            catch
            {
                MessageBox.Show("Error occured while trying to parse a single file tracker's response.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
            }
        }

        public void ParseMultiFileMessage(string message, ref string existingFileName)
        {
            // node1Ip#file1*file1.size*file1description-file2*file2.size*file2description-file3*file3.size*file3description \n
            string[] files = message.Split('\n');
            if (files.Length > 0)
            {
                foreach (string file in files)
                {
                    if(file.Length > 0)
                        ParseSingleFileMessage(file,ref existingFileName);
                }
            }
            else
            {
                MessageBox.Show("Invalid list of files received from the local tracker.","Error",MessageBoxButtons.OK,MessageBoxIcon.Exclamation,MessageBoxDefaultButton.Button1);                
            }
        }
    }
}

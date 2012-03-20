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

namespace TorrentF.FilesStatus
{
    class FilePublishingStatus: FileDetails
    {
        private bool published;
        private long numberOfUploadedBytes;
        // Indicates the number of times the file was uploaded
        private Int32 uploaded;

        public FilePublishingStatus(bool _strorageStatus, string _fileName, long _fileSize, string _remoteHostIp, Int32 _remoteHostPort, string _localFilePath)
            : base(_strorageStatus, _fileName, _fileSize, _remoteHostIp, _remoteHostPort, _localFilePath)
        {
            published = false;
            numberOfUploadedBytes = 0;
            uploaded = 0;
        }

        public bool Published
        {
            get
            {
                return published;
            }
            set
            {
                published = value;
            }
            
        }

        public long NumberOfUploadedBytes
        {
            get
            {
                return numberOfUploadedBytes;
            }
            set
            {
                numberOfUploadedBytes = value;
            }
        }

        public int Uploaded
        {
            get
            {
                return uploaded;
            }
            set
            {
                uploaded = value;
            }
        }
    }
}

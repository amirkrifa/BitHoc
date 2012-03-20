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
using System.Diagnostics;

namespace TorrentF.Utilities
{
    class ParseP2PMessage
    {
        // A node asking for a file send  a message following the format
        // fileName#fileSize
        static public string GetFileName(string message)
        {
            Int32 dz = message.IndexOf("#");
            Trace.Assert(dz > 0, "ParseP2PMessage::GetFileName, invalid message: " + message);
            string res = message.Substring(0, dz);
            return res;
        }

        static public long GetFileSize(string message)
        {
            Int32 dz = message.IndexOf("#");
            Trace.Assert(dz > 0, "ParseP2PMessage::GetFileSize, invalid message: " + message);
            string res = message.Substring(dz+1);
            return long.Parse(res);
        }
    }
}

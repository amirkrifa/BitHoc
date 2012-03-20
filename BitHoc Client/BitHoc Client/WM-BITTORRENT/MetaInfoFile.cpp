
/*
Copyright (C) 2005  Anders Hedstrom

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"


#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#include <windows.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <Utility.h>
#include "BString.h"
#include "Session.h"
#include "PeerHandler.h"
#include "MetaInfoFile.h"
#include "Exception.h"
#include "BTDictionary.h"



#define DEB(x) x

int MetaInfoFile::m_nr = 0;

MetaInfoFile::MetaInfoFile(void)
{
}

MetaInfoFile::~MetaInfoFile(void)
{
}

void MetaInfoFile::LoadMetaInfoFile()
{
try
	{
		FILE *fil = fopen(m_filename.c_str(), "rb");
		if (fil)
		{
			PeerHandler& ref = static_cast<PeerHandler&>(Handler());
			BString meta;
			meta.read_file(fil);
			fclose(fil);
			std::string info_hash = meta.GetHashAsString("info");
			// copy metainfo file
			std::string copy_to = ref.GetTorrentDirectory() + "/" + info_hash + "/.metainfo";
			ref.mkpath(copy_to);
			fil = fopen(m_filename.c_str(), "rb");
			if (fil)
			{
				FILE *fil2 = fopen(copy_to.c_str(), "wb");
				char buf[1000];
				size_t n = fread(buf, 1, 1000, fil);
				while (n > 0)
				{
					fwrite(buf, 1, n, fil2);
					n = fread(buf, 1, 1000, fil);
				}
				fclose(fil2);
				fclose(fil);
			}
			if (ref.SessionExists(info_hash))
			{
			}
			else
			{
				Session *sess = new Session(dynamic_cast<SocketHandler&>(Handler()), info_hash);
				ref.RegSession( sess );
				sess -> SetHash(meta.GetHash("info"));
				BTString *p;
				if ((p = meta.GetString("announce")) != NULL)
				{
					sess -> SetAnnounce(p -> GetValue());
				}
				std::string name;
				if ((p = meta.GetString("info.name")) != NULL)
				{
					sess -> SetName(name = p -> GetValue());
				}
				BTInteger *piecelength = meta.GetInteger("info.piece length");
				if (piecelength)
				{
					sess -> SetPieceLength(piecelength -> GetVal());
				}
				BTInteger *length = meta.GetInteger("info.length");
				if (length)
				{
					sess -> AddFile(length -> GetVal());
				}
				else // info.files
				{
					BTObject *p = meta.GetBTObject("info.files");
					BTList *files = dynamic_cast<BTList *>(p);
					if (files)
					{
						btobject_v& ref = files -> GetList();
						for (btobject_v::iterator it = ref.begin(); it != ref.end(); it++)
						{
							BTDictionary *p = dynamic_cast<BTDictionary *>(*it);
							if (p)
							{
								BTInteger *length = dynamic_cast<BTInteger *>(p -> Find("length"));
								BTList *path = dynamic_cast<BTList *>(p -> Find("path"));
								if (path && length)
								{
									btobject_v& ref = path -> GetList();
									std::string pathname = name;
									for (btobject_v::iterator it = ref.begin(); it != ref.end(); it++)
									{
										BTString *p = dynamic_cast<BTString *>(*it);
										if (p)
											pathname += "/" + p -> GetValue();
									}
									sess -> AddFile(pathname, length -> GetVal());
								}
							}
						}
					}
				}
				// pieces checksum
				BTString *pieces = meta.GetString("info.pieces");
				if (pieces)
				{
					sess -> SetPieces( pieces -> GetValue() );
				}
				sess -> CreateFileManager();
				// restore session
				sess -> Load();
				//
			}
			//
			//unlink(m_filename.c_str());
		}
	}
	catch (const Exception& e)
	{
		Handler().LogError(this, "InitSession", 0, e.GetText(), LOG_LEVEL_ERROR);
	}
	
}
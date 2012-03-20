/*

  Copyright (C) 2008  INRIA, Planète Team

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


/*
Copyright (C) 2005  Anders Hedstrom

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


/*!
* \file MetaInfoSocket.cpp
* \brief Implementation of the functions used to load the MetaInfo File and to load the new Download session.
**/


#include "stdafx.h"

#ifdef _WIN32
#pragma warning(disable:4786)
#endif
//! Modified by Amir Krifa.
//#include <sys/types.h>
//#include <sys/stat.h>
#include <windows.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <Utility.h>
#include "BString.h"
#include "Session.h"
#include "PeerHandler.h"
#include "MetainfoSocket.h"
#include "Exception.h"
#include "BTDictionary.h"
#include "Peer.h"
#include "pSocket.h"
extern pSocket * GetpSocket();
int MetainfoSocket::m_nr = 0;
//! Added by Amir Krifa.
extern LPWSTR ConvertLPCSTRToLPWSTR (char* pCstring);
extern CRITICAL_SECTION csBITTORENT;

MetainfoSocket::MetainfoSocket(SocketHandler& h)
:TcpSocket(h)
,m_filenr(++m_nr)
,m_fil(NULL)
,m_state(STATE_GET_LENGTH)
,m_sz(0)
,m_sz_read(0)
,pLocalSocket(NULL)
{
}


MetainfoSocket::MetainfoSocket(SocketHandler& h,const std::string& filename)
:TcpSocket(h)
,m_filenr(0)
,m_filename(filename)
,m_fil(NULL)
,m_state(STATE_GET_LENGTH)
,m_sz(0)
,m_sz_read(0)
{
}

//! Modified by Amir Krifa.
MetainfoSocket::~MetainfoSocket()
{
	if (m_fil)
		fclose(m_fil);
}


void MetainfoSocket::OnConnect()
{
	/*
	struct stat st;
	if (stat(m_filename.c_str(), &st) == -1)
	{
		SetCloseAndDelete();
		return;
	}
	SendBuf( (char *)&st.st_size, sizeof(off_t));
	FILE *fil = fopen(m_filename.c_str(), "rb");
	if (fil)
	{
		char buf[5000];
		int n = fread(buf,1,5000,fil);
		while (n > 0)
		{
			SendBuf(buf,n);
			n = fread(buf,1,5000,fil);
		}
		fclose(fil);
	}
	SetCloseAndDelete();
	*/
}


void MetainfoSocket::OnAccept()
{
	m_filename = "metafile." + Utility::l2string(m_filenr);
	m_fil = fopen(m_filename.c_str(), "wb");
	m_state = STATE_GET_LENGTH;
}


/*
void MetainfoSocket::OnRawData(const char *p,size_t l)
{
	if (m_fil)
		fwrite(p,1,l,m_fil);
}
*/


void MetainfoSocket::OnRead()
{
	TcpSocket::OnRead();
	while (ibuf.GetLength())
	{
		size_t l = ibuf.GetLength();
	
		switch (m_state)
		{
		case STATE_GET_LENGTH:
			if (l < sizeof(off_t))
				return;
			ibuf.Read( (char *)&m_sz, sizeof(off_t));
			m_state = STATE_GET_FILE;
			break;
		case STATE_GET_FILE:
			{
				char slask[TCP_BUFSIZE_READ];
				ibuf.Read( slask, l);
				if (m_fil)
					fwrite(slask,1,l,m_fil);
				m_sz_read += l;
			}
			break;
		}
	}
}


void MetainfoSocket::OnDelete()
{
	if (m_fil)
	{
		fclose(m_fil);
		m_fil = NULL;
		if (m_sz_read == m_sz)
		{
			InitSession();
		}
		else
		{
			
		}
	}
}


void MetainfoSocket::InitSession()
{

}

//! Added by Amir Krifa.
//! Used to load a Meta Info File, not in a network manner.
void MetainfoSocket::LoadMetaInfoFile(bool bSeeding,string MetaInfoFilePath)
{
	this->SetMetaInfoFileToLoad(MetaInfoFilePath);
	try
	{
		FILE *fil = fopen(m_filename.c_str(), "rb");
		m_fil=fil;
		if (fil)
		{
			PeerHandler& ref = static_cast<PeerHandler&>(Handler());
			BString meta;
			meta.read_file(fil,m_filename);
			fclose(fil);
			std::string info_hash = meta.GetHashAsString("info");
			// copy metainfo file
			std::string copy_to = ref.GetTorrentDirectory() + "\\" + info_hash + "\\.metainfo";
			
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
				Session *sess = new Session(dynamic_cast<SocketHandler&>(Handler()), info_hash, bSeeding,string(ref.GetTorrentDirectory() + "\\" + info_hash + "\\" ));
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
											pathname += "\\" + p -> GetValue();
									}
									sess -> AddFile(pathname, length -> GetVal());
								}
								path=NULL;
								length=NULL;
							}
							p=NULL;
						}
					}
					files=NULL;
				}
				//! pieces checksum
				BTString *pieces = meta.GetString("info.pieces");
				if (pieces)
				{
					sess -> SetPieces( pieces -> GetValue() );
				}
				pieces =NULL;
				sess->SetFileToSeedPath(MetaInfoFilePath.substr(0,MetaInfoFilePath.find_last_of(".")));
				if(sess->isSeeding())
				{
					
					//! Copy the file to seed to the Torrents Directory
					string SourceFile(sess->GetFileToSeedPath());
					string DestFile(ref.GetTorrentDirectory() + "\\" + info_hash + "\\"+sess->GetName());
					if(CopyFile(ConvertLPCSTRToLPWSTR((char*)SourceFile.c_str()),ConvertLPCSTRToLPWSTR((char *)DestFile.c_str()),FALSE)==0)
					{
						
						//! Source File Not Found
						MessageBox( NULL,TEXT("Error encountred while trying to copy the file to seed."),
                            TEXT("Error while copying the source file."), MB_OK|MB_ICONSTOP );
               
					}
						
				}
				sess -> CreateFileManager();
				//! restore session
				sess -> Load();
				sess->Verify();
				sess=NULL;

			}
			
		}
	}
	catch (string)
	{
	}
	
}


	
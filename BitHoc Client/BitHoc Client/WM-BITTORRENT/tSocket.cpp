
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
* \file tSocket.cpp
**/

#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <Utility.h>

#include "PeerHandler.h"
#include "Session.h"
#include "BString.h"
#include "BTDictionary.h"
#include "Exception.h"
#include "Peer.h"
#include "tSocket.h"

#define DEB(x) 
extern LPWSTR ConvertLPCSTRToLPWSTR (char* pCstring);
extern int WM_BITTORRENT_VERSION;
int tSocket::m_next_id = 0;
extern size_t GetFileSizeWin32(const TCHAR *fileName);

tSocket::tSocket(SocketHandler& h,const std::string& hash, bool Seeding, bool SyncRequest)
:HttpGetSocket(h)
,m_hash(hash)
,m_gzip(false)
,bSeeding(Seeding)
{
	if(!SyncRequest)
	{
		Session *p = static_cast<PeerHandler&>(Handler()).GetSession(hash);
		if (p)
		{
			
				std::string announce = p -> GetAnnounceUrl("started");
				std::string host;
				port_t port;
				Url(announce, host, port);
				if(host.size() && port)
				{
					PeerHandler& ref = static_cast<PeerHandler&>(Handler());
					m_filename = ref.GetTorrentDirectory() + "\\" + hash + "\\tracker." + Utility::l2string(++m_next_id);
					
					ref.mkpath( m_filename );
					SetFilename(m_filename);
					SetConnectTimeout(10);
					Open(host, port);
				}
				else
					SetCloseAndDelete();
			
		}
		else
			SetCloseAndDelete();
	}else
		{
			// Sending a synchronisarion request.
			std::string announce("http://127.0.0.1:2617/synchronize?info_hash="+hash);
			std::string host;
			port_t port;
			Url(announce, host, port);
			if(host.size() && port)
			{
				PeerHandler& ref = static_cast<PeerHandler&>(Handler());
				m_filename = ref.GetTorrentDirectory()  + "\\tracker." + Utility::l2string(++m_next_id);
				
				ref.mkpath( m_filename );
				SetFilename(m_filename);
				SetConnectTimeout(10);
				Open(host, port);
			}
			else
				SetCloseAndDelete();
		}
}


tSocket::~tSocket()
{
	DeleteFile(ConvertLPCSTRToLPWSTR((char*)m_filename.c_str()));
}


void tSocket::OnDelete()
{
	HttpGetSocket::OnDelete();
	if (!Complete())
	{
		return;
	}
	Session *sess = static_cast<PeerHandler&>(Handler()).GetSession(m_hash);
	if (!sess)
	{
		return;
	}
	if (m_gzip)
	{
		//char cmd[1000];
		//unlink(m_filename.substr(0, m_filename.size() - 3).c_str());
		//sprintf(cmd, "/usr/bin/gzip -d %s", m_filename.c_str());
		//system( cmd );
		//m_filename = m_filename.substr(0, m_filename.size() - 3); // remove .gz
	}
	try
	{
	size_t uTest = 0;
	if((uTest=GetFileSizeWin32(ConvertLPCSTRToLPWSTR((char*)m_filename.c_str())))>1)
	{
		FILE *fil;
		if ((fil = fopen(m_filename.c_str(),"rb")) != NULL)
		{
			BString bstr;
			bstr.read_file(fil,m_filename);
			fclose(fil);
			int interval;
			BTInteger *p = dynamic_cast<BTInteger *>(bstr.GetBTObject("interval"));
			if (p)
			{
				interval = atoi(p -> GetValue().c_str());
				sess -> SetInterval( interval );
			}
			BTList *l = dynamic_cast<BTList *>(bstr.GetBTObject("peers"));
			if (l)
			{
				for (btobject_v::iterator it = l -> GetList().begin(); it != l -> GetList().end(); it++)
				{
					BTDictionary *p = dynamic_cast<BTDictionary *>(*it);
					std::string ip;
					std::string id;
					int port = 0;
					int iNumberOfHops=0;
					if (p)
					{
						for (btmap_t::iterator it = p -> GetMap().begin(); it != p -> GetMap().end(); it++)
						{
							std::string key = (*it).first;
							BTObject *p0 = (*it).second;
							if (key == "ip")
							{
								BTString *p = dynamic_cast<BTString *>(p0);
								if (p)
									ip = p -> GetValue();
							}
							else
							if (key == "peer id")
							{
								BTString *p = dynamic_cast<BTString *>(p0);
								if (p)
									id = p -> GetValue();
							}
							else
							if (key == "port")
							{
								BTInteger *p = dynamic_cast<BTInteger *>(p0);
								if (p)
									port = atoi(p -> GetValue().c_str());
							}
							else
							if (key == "hops" && WM_BITTORRENT_VERSION==2)
							{
								BTInteger *p = dynamic_cast<BTInteger *>(p0);
								if (p)
									iNumberOfHops = atoi(p -> GetValue().c_str());
							}
						}
					}
					if (ip.size() && id.size() == 20 && port)
					{
						Peer *p = sess -> GetPeer(ip);
						if (!p)
						{
							Peer *p;
							
							if(WM_BITTORRENT_VERSION==1)
								p = new Peer(dynamic_cast<SocketHandler&>(Handler()),m_hash,ip,id,port);
							else if(WM_BITTORRENT_VERSION==2)
								p = new Peer(dynamic_cast<SocketHandler&>(Handler()),m_hash,ip,id,port,iNumberOfHops);
							
							sess -> AddPeer(p);
							p=NULL;
						}
						else
						{
							p->SetPort(port);
							p->SetId(id);
							if(WM_BITTORRENT_VERSION ==2 )
								p->SetNumberOfHops(iNumberOfHops);
							p=NULL;
						}
						this->SetCloseAndDelete();
						
						
					}
				}
			}
		}
	}
	}
		catch (string)
		{
			this->SetCloseAndDelete();
		}
}	


void tSocket::OnFirst()
{
	HttpGetSocket::OnFirst();

	if (IsRequest())
	{
		//printf(" Method: %s\n",GetMethod().c_str());
		//printf(" URL: %s\n",GetUrl().c_str());
		//printf(" Http version: %s\n",GetHttpVersion().c_str());
	}

	if (IsResponse())
	{
		//printf(" Http version: %s\n",GetHttpVersion().c_str());
		//printf(" Status: %s\n",GetStatus().c_str());
		//printf(" Status text: %s\n",GetStatusText().c_str());
	}
}


void tSocket::OnHeader(const std::string& key,const std::string& value)
{
	HttpGetSocket::OnHeader(key,value);
	if (!strcasecmp(key.c_str(), "content-encoding"))
	{
		if (!strcasecmp(value.c_str(), "gzip"))
		{
			m_filename += ".gz";
			SetFilename(m_filename);
			m_gzip = true;
		}
	}
}



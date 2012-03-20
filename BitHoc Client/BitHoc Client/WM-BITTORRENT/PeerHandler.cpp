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
as published by the Free Software Foundation

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
* \file PeerHandler.cpp
* \brief Custom sockethandler for bittorrent client
**/

#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
//#include <direct.h>
#endif

#include <windows.h>

#include <stdarg.h>

#include "Session.h"
#include "tSocket.h"
#include "cstring.h"
#include "pSocket.h"
#include "Peer.h"
//#include "StatusSocket.h"
#include "PeerHandler.h"
#include <string.h>

#define DEB(x) x
extern LPWSTR ConvertLPCSTRToLPWSTR(char* pCstring);
//extern int gettimeofday(struct timeval *tv, struct timezone_s *tz);
extern std::map<std::string,Session *> mSuspendedSession;


PeerHandler::PeerHandler()
:SocketHandler()
,m_listen_port(0)
,m_choke_timer(5)
,m_min_peers(10)
,m_max_peers(20)
,m_local_choke_time(5)
,m_ignore_choke(false)
,m_max_request_age(5)
,m_quit(false)
,m_debug(0)
,m_downloaders(4)
,m_optimistic(1)
,pLastTSocket(NULL)
{
	InitializeCriticalSection(&appStop);
	srand(time(NULL) * (time_t)this);
	
}


PeerHandler::PeerHandler(StdLog *p)
:SocketHandler(p)
,m_listen_port(0)
,m_choke_timer(5)
,m_min_peers(10)
,m_max_peers(20)
,m_local_choke_time(5)
,m_ignore_choke(false)
,m_max_request_age(5)
,m_quit(false)
,m_debug(0)
,m_downloaders(4)
,m_optimistic(1)
,pLastTSocket(NULL)
{
	InitializeCriticalSection(&appStop);
}


PeerHandler::~PeerHandler()
{
	
	while (!m_sessions.empty())
	{
		session_m::iterator it = m_sessions.begin();
		Session *p = (*it).second;
		delete p;
		m_sessions.erase(it);
	}
	DeleteCriticalSection(&appStop);
}


void PeerHandler::RegSession(Session *p)
{
	m_sessions[p -> GetInfoHash()] = p;
}


bool PeerHandler::SessionExists(const std::string& hash)
{
	for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		if ((*it).first == hash && dynamic_cast<Session *>((*it).second))
			return true;
	}
	return false;
}

//! Modified by Amir Krifa.

void PeerHandler::mkpath(const std::string& path)
{
	for (size_t i = 0; i < path.size(); i++)
	{
		if (path[i] == '\\')
		{
#ifdef _WIN32
		std::string sTempDirectory(path.substr(0, i).c_str());
		CreateDirectory( ConvertLPCSTRToLPWSTR((char*)sTempDirectory.c_str()),NULL);
#else
		mkdir(path.substr(0, i).c_str(), 0750);
#endif		
		}
	}
}


//! Modified by Amir Krifa.
Session *PeerHandler::GetSession(const std::string& hash)
{
	session_m::iterator it;
	for ( it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		if ((*it).first == hash && dynamic_cast<Session *>((*it).second))
			return (*it).second;
	}
	if(it == m_sessions.end())
	{
		// My be the session is paused
		if(!mSuspendedSession.empty())
		{
			std::map<std::string, Session *>::iterator iter=mSuspendedSession.begin();
			while(iter!=mSuspendedSession.end())
			{
				if (dynamic_cast<Session *>((*iter).second)->GetInfoHash() == hash && dynamic_cast<Session *>((*iter).second))
					return (*iter).second;
				iter++;
			}
		}
	}
	return NULL;
}


//! Modified by Amir Krifa.
void PeerHandler::Tick(time_t now)
{	
	if(!m_sessions.empty())
	{
		bool sessionsToRemove = false;
		for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end() ; it++)
		{
			Session *sess = (*it).second;
			if (sess)
			{
				if ((now > sess -> GetTimeTracker() + sess -> GetInterval()) || sess->iPaused==1)
				{
					//if(pLastTSocket!=NULL)
					//{
					//	this->pLastTSocket->CloseAndDelete();
					//}
					tSocket *p = new tSocket(*this, sess -> GetInfoHash(),sess->isSeeding(),false);
					p->SetDeleteByHandler();
				
					Add(p);
					//pLastTSocket=p;
					p=NULL;
					sess -> SetTimeTracker();
				}

				sess -> Update();
				if(sess->bToDelete)
					sessionsToRemove = true;
			}
			sess = NULL;
		}
		if(sessionsToRemove)
			Clean();
	}
}

//! Added by Amir Krifa.
void PeerHandler::Clean()
{
	if(!m_sessions.empty())
	{
		session_m::iterator it = m_sessions.begin();
		while(it != m_sessions.end())
		{
			Session *sess = (*it).second;
			if (sess->bToDelete)
			{	
				std::string sInfoHash=sess->GetInfoHash();
				std::string sTorrentDirectory=this->GetTorrentDirectory();
				bool bDeleteFiles=sess->bDeleteRelatedFiles;
				// This session should be deleted
				delete sess;
				m_sessions.erase(it);
			
				if(bDeleteFiles)
				{
					std::string sDirToDelete=sTorrentDirectory;
					sDirToDelete.append("\\"+sInfoHash);
					std::string sDataFile=sDirToDelete+"\\";
					sDataFile.append(sess->GetName());
					std::string sSessionLog=sDirToDelete+"\\"+std::string("session_log.txt");
					std::string sMetaInfo=sDirToDelete+"\\"+std::string(".metainfo");
					std::string sTempFile=sDirToDelete+"\\."+sInfoHash;
					DeleteFile(ConvertLPCSTRToLPWSTR((char*)sDataFile.c_str()));
					DeleteFile(ConvertLPCSTRToLPWSTR((char*)sSessionLog.c_str()));
					DeleteFile(ConvertLPCSTRToLPWSTR((char*)sMetaInfo.c_str()));
					DeleteFile(ConvertLPCSTRToLPWSTR((char*)sTempFile.c_str()));
					RemoveDirectory(ConvertLPCSTRToLPWSTR((char*)sDirToDelete.c_str()));
				}
				if(!m_sessions.empty())
					{it++;continue;}
				else break;
			}
			sess=NULL;	
			it++;
		}
	}		
}

void PeerHandler::dprintf(int id, char *format, ...)
{
//	char *colors = "rgybmcwlRGYBMCWL";
	char *colors = "rgymcRGYBMCWL";
	va_list ap;
	va_start(ap, format);
	char tmp[5000];
#ifdef _WIN32
	vsprintf(tmp, format, ap);
	va_end(ap);
	printf("%s\n", tmp);
#else
	vsnprintf(tmp, 5000, format, ap);
	va_end(ap);
	cstring str;
	char slask[10];
	sprintf(slask, "&%c", colors[id % strlen(colors)]);
	str = slask;
	str += tmp;
	str += "&n";
	printf("%s\n", str.c_str());
	if (0)
	{
		FILE *fil = fopen("btlog","at");
		if (!fil)
			fil = fopen("btlog","wt");
		fprintf(fil,"%s\n", str.c_str());
		fclose(fil);
	}
#endif
}


size_t PeerHandler::PeerCount(const std::string& hash)
{
	size_t q = 0;
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		pSocket *p = dynamic_cast<pSocket *>((*it).second);
		if (p && p -> GetHash() == hash)
		{
			q++;
		}
	}
	return q;
}


bool PeerHandler::Connected(Peer *peer)
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		pSocket *p = dynamic_cast<pSocket *>((*it).second);
		if (p && p -> GetHash() == peer -> GetHash() && p -> GetRemoteAddress() == peer -> GetIP())
		{
			return true;
		}
	}
	return false;
}


pSocket *PeerHandler::PeerSocket(Peer *peer)
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		pSocket *p = dynamic_cast<pSocket *>((*it).second);
		if (p && p -> GetHash() == peer -> GetHash() && p -> GetRemoteAddress() == peer -> GetIP())
		{
			return p;
		}
	}
	return NULL;
}


void PeerHandler::SendHave(const std::string& hash,size_t piece)
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		pSocket *p = dynamic_cast<pSocket *>((*it).second);
		if (p && p -> GetHash() == hash && p -> CTS() )
		{
			p -> SendHave(piece);
		}
	}
}


//! Modified by Amir Krifa.
void PeerHandler::Save()
{
	for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		Session *sess = (*it).second;
		//sess->InCriticalSection();
		sess -> Save();
		//sess->OutCriticalSection();
	}
}


void PeerHandler::CheckDownloadRate()
{
	std::map<pSocket *,size_t> mmap;
	size_t max = 0;
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		pSocket *p = dynamic_cast<pSocket *>((*it).second);
		if (p)
		{
			Peer *peer = p -> GetPeer();
			if (p && peer && p -> CTS() && !peer -> IsChoked())
			{
				size_t sz;
				if (p -> GetDownloadRate(sz))
				{
					mmap[p] = sz;
					max = MAX(max,sz);
				}
			}
		}
	}
	if (mmap.size() == GetDownloaders() + GetOptimistic() )
	{
		pSocket *least = NULL;
		for (std::map<pSocket *,size_t>::iterator it = mmap.begin(); it != mmap.end(); it++)
		{
			pSocket *sock = (*it).first;
			size_t sz = (*it).second;
			if (sz < max)
			{
				least = sock;
				max = sz;
			}
		}
		if (least)
		{
			least -> SendChoke(true);
		}
	}
}

/*
std::string PeerHandler::GetTime()
{
	char slask[100];

	struct timeval tv;
	timezone_s tz;
	gettimeofday(&tv, &tz);
	sprintf(slask, "%ld.%06ld", tv.tv_sec, tv.tv_usec);
	return slask;
}
*/

//! Added by Amir Krifa.
//! Deletes the session given the info_hash.
bool PeerHandler::DeleteSession(const std::string& hash,bool bPaused)
{
	if(!bPaused)
	{
		for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
		{
			if ((*it).first == hash && dynamic_cast<Session *>((*it).second))
			{
				//! deleting the session
				Session* temp= (*it).second;

				delete temp;
				this->m_sessions.erase(it);
				return true;
			}
		}
		return false;
	}
	return false;
}


//! Added by Amir Krifa.
//! Retrives the Hash based in the File Name.
std::string PeerHandler::GetHashFronFileName(const std::string & sFileName)
{
	
	if( !m_sessions.empty() )
	{
		session_m::iterator it;
		for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
		{
			if (dynamic_cast<Session *>((*it).second))
			{
				if(dynamic_cast<Session *>((*it).second)->GetName().compare(sFileName)==0)
					return (*it).first;
			}
			
		}
		//! Look in the saved sessions.
		//! My be the session is paused.
		if(it==m_sessions.end())
		{
			if(!mSuspendedSession.empty())
			{
				std::map<std::string, Session *>::iterator iter=mSuspendedSession.begin();
				while(iter!=mSuspendedSession.end())
				{
					if (dynamic_cast<Session *>((*iter).second)->GetName() ==sFileName && dynamic_cast<Session *>((*iter).second))
						return dynamic_cast<Session *>((*iter).second)->GetInfoHash();
					iter++;
				}
			}
		}
	}
	else 
	{
		//! Look in the saved sessions.
		//! My be the session is paused.
		if(!mSuspendedSession.empty())
		{
			std::map<std::string, Session *>::iterator iter=mSuspendedSession.begin();
			while(iter!=mSuspendedSession.end())
			{
				if (dynamic_cast<Session *>((*iter).second)->GetName() ==sFileName && dynamic_cast<Session *>((*iter).second))
					return dynamic_cast<Session *>((*iter).second)->GetInfoHash();
				iter++;
			}
		}
	}
	
	return std::string("");
}


//! Added by Amir Krifa.
//! Suspends session given the info_hash.
Session * PeerHandler::SuspendSession(const std::string& hash)
{
	for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		if ((*it).first == hash && dynamic_cast<Session *>((*it).second))
		{
			//! deleting the session.
			Session *pSession=(*it).second;
			(*it).second=NULL;
			//pSession->InCriticalSection();
			pSession->Save();
			pSession->SuspendDownloading(false);
			this->m_sessions.erase(it);
			//pSession->OutCriticalSection();
			return pSession;
		}
	}
	return NULL;
}

//! Added by Amir Krifa.

void PeerHandler::SendSynchronisationRequest(std::string session_hash)
{
	tSocket *p = new tSocket(*this, session_hash,false,true);
	p->SetDeleteByHandler();
	Add(p);
	p=NULL;
}

void PeerHandler::SynchronizeSession(std::string session_hash)
{
	for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		if((*it).first == session_hash)
		{
			// Session founded 
			(*it).second->Synchronize();
		}
	}
}
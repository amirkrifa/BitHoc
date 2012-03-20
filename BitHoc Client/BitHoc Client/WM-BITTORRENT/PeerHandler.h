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
* \file PeerHandler.h
* \brief Custom sockethandler for bittorrent client.
**/


#include "stdafx.h"

#ifndef _PEERHANDLER_H
#define _PEERHANDLER_H

#include <SocketHandler.h>
#include <StdLog.h>
#include "Session.h"
#ifdef _WIN32
#define MAX(a,b) (a>b)?a:b
#endif

class Peer;
class pSocket;
extern HANDLE hMutex;
class tSocket;
//! Custom sockethandler for bittorrent client

class PeerHandler : public SocketHandler
{
	typedef std::map<std::string,Session *> session_m;
public:
	PeerHandler();
	PeerHandler(StdLog *);
	~PeerHandler();
	
	void RegSession(Session *);
	bool SessionExists(const std::string& hash);
	Session *GetSession(const std::string& hash);
	
	const std::string& GetTorrentDirectory() { return m_torrent_dir; }
	void SetTorrentDirectory(const std::string& x) { m_torrent_dir = x; }

	void mkpath(const std::string& );

	void Tick(time_t);
	port_t GetListenPort() { return m_listen_port; }
	void SetListenPort(port_t x) { m_listen_port = x; }
	const std::string& GetExternIP() { return m_extern_ip; }
	void SetExternIP(const std::string& x) { m_extern_ip = x; }

	void dprintf(int id,char *format,...);

	int GetChokeTimer() { return m_choke_timer; }
	void SetChokeTimer(int x) { m_choke_timer = x; }
	/** outgoing connections stop at this limit */
	size_t GetMinPeers() { return m_min_peers; }
	void SetMinPeers(size_t x) { m_min_peers = x; }
	/** no more incoming connections will be accepted above this limit */
	size_t GetMaxPeers() { return m_max_peers; }
	void SetMaxPeers(size_t x) { m_max_peers = x; }

	int LocalChokeTime() { return m_local_choke_time; }
	void SetLocalChokeTime(int x) { m_local_choke_time = x; }

	size_t PeerCount(const std::string& hash);
	bool Connected(Peer *);
	pSocket *PeerSocket(Peer *);
	void SendHave(const std::string& have,size_t piece);

	bool IgnoreChoke() { return m_ignore_choke; }
	void SetIgnoreChoke(bool x = true) { m_ignore_choke = x; }

	int MaxRequestAge() { return m_max_request_age; }
	void SetMaxRequestAge(int x) { m_max_request_age = x; }

	void Save();
	//! Modified by Amir Krifa.
	void Show() {
		if(!this->m_sessions.empty())
		{
			for (session_m::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
			{
				Session *sess = (*it).second;
				if (sess)
				{	//sess->InCriticalSection();
					sess -> PeerStatus();
					//sess->OutCriticalSection();
				}
			}
		}
	}

	//void Show(StatusSocket *);
	bool Quit() 
	{
		bool tmp;
		EnterCriticalSection(&appStop);
		tmp = m_quit;
		LeaveCriticalSection(&appStop);
		return tmp;
	}
	void SetQuit(bool x = true) 
	{ 
		EnterCriticalSection(&appStop);
		m_quit = x; 
		LeaveCriticalSection(&appStop);
	}
	void SetDebug(int x) { m_debug = x; }
	int GetDebug() { return m_debug; }
	size_t GetDownloaders() { return m_downloaders; }
	void SetDownloaders(size_t x) { m_downloaders = x; }
	size_t GetOptimistic() { return m_optimistic; }
	void SetOptimistic(size_t x) { m_optimistic = x; }

	void CheckDownloadRate();
	//std::string GetTime();
	
	// Added by Amir Krifa

	/*!
	* \fn void SetPaused()
	* \brief Wait for the aquisition of a Mutex. 
	*/
	void SetPaused()
	{
		DWORD test = WaitForSingleObject(hMutex,60000);
		if(test == WAIT_OBJECT_0)
		{
			int i=0;
		}
	};
	
	/*!
	* \fn void SetNotPaused()
	* \brief Release a Mutex. 
	*/
	void SetNotPaused()
	{
		ReleaseMutex(hMutex);
	};
	
	/*!
	* \fn int GetTheNumberOfActiveSession()
	* \brief Returns the number of active sessions. 
	*/
	int GetTheNumberOfActiveSession()
	{
		return this->m_sessions.size();
	};
	
	/*!
	* \fn bool DeleteSession(const std::string& hash,bool )
	* \brief Delete a Session. 
	*/	
	bool DeleteSession(const std::string& hash,bool );
	
	/*!
	* \fn std::string GetHashFronFileName(const std::string & sFileName)
	* \brief Returns the hash given the file name. 
	*/
	std::string GetHashFronFileName(const std::string & sFileName);

	/*!
	* \fn Session * SuspendSession(const std::string& hash)
	* \brief Suspends a session. 
	*/	
	Session * SuspendSession(const std::string& hash);

	/*!
	* \fn void Clean()
	* \brief Cleans the already deleted sessions. 
	*/	
	void Clean();
	/*!
	* \fn void SendSynchronisationRequest()
	* \brief Sends a synchronisation request. 
	*/
	void SendSynchronisationRequest(std::string);
	/*!
	* \fn void SynchronizeSession(std::string session_hash)
	* \brief Synchronizes a given session. 
	*/
	void SynchronizeSession(std::string session_hash);

private:
	PeerHandler(const PeerHandler& ) {} // copy constructor
	PeerHandler& operator=(const PeerHandler& ) { return *this; } // assignment operator
	session_m m_sessions;
	std::string m_torrent_dir;
	port_t m_listen_port;
	std::string m_extern_ip;
	int m_choke_timer;
	size_t m_min_peers;
	size_t m_max_peers;
	int m_local_choke_time;
	bool m_ignore_choke;
	int m_max_request_age;
	bool m_quit;
	int m_debug;
	size_t m_downloaders;
	size_t m_optimistic;
	unsigned char m_peer_id[20];
	tSocket *pLastTSocket;
	CRITICAL_SECTION appStop;
};

#endif // _PEERHANDLER_H

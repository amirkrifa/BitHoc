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
* \file pSocket.h
* \brief BitTorrent peer to peer connection 
**/
#include "stdafx.h"
#ifndef _PSOCKET_H
#define _PSOCKET_H

#include <TcpSocket.h>
#include <SocketHandler.h>
class LogFile;
#define DLCHECKSIZE 30 // seconds


//! BitTorrent peer to peer connection 

class pSocket : public TcpSocket
{	enum state_t {
		ACCEPT_LENGTH = 0,
		ACCEPT_PROTOCOL,
		ACCEPT_NULLBYTES,
		ACCEPT_HASH,
		ACCEPT_PEER_ID,
		STATE_COMMAND,
		STATE_COMMAND2,
		STATE_GET_INTEGERS,
		STATE_GET_BITMAP,
		STATE_GET_PIECE,
		STATE_GET_COMMAND,
	};
public:
	pSocket(SocketHandler&);
	pSocket(SocketHandler&,const std::string& hash,unsigned char *remote_peer_id);
	~pSocket();

	void OnAccept(); // hash still unknown
	void OnConnect();
	void OnRead();
	void OnConnectFailed();
	void OnDelete();

	void SendHello();
	void SendBitmap();
	void SendInterest(bool);
	void SendRequest(size_t piece,size_t offset,size_t length);
	void SendCancel(size_t piece,size_t offset,size_t length);
	void SendChoke(bool);
	void SendHave(size_t piece);
	void SendPiece(size_t piece,size_t offset,size_t length);

	const std::string& GetHash() { return m_hash; }
	Peer *GetPeer();

	bool Interested() { return m_interest; }
	bool Choked() { return m_choke; }
	time_t ChokeTime() { return time(NULL) - m_t_choke; }

	bool CTS() { return m_cts; }
	int GetSockId() { return m_id; }

	uint64_t GetBytesR(); // GetBytesReceived
	uint64_t GetBytesW(); // GetBytesSent
	bool GetDownloadRate(size_t&);
	uint64_t GetBytesRecv(){
		return this->GetBytesReceived();
	};
	uint64_t GetBytesSen(){
		return this->GetBytesSent();
	};

	void ShowStatus(size_t);
	void InitLogFile(std::string ld,bool);
	LogFile * pSocket_log;
	std::string sLogDirectory;
	
	
private:
	pSocket(const pSocket& s) : TcpSocket(s) {} // copy constructor
	pSocket& operator=(const pSocket& ) { return *this; } // assignment operator

	void cmdChoke();
	void cmdUnchoke();
	void cmdInterested();
	void cmdNotinterested();
	void cmdHave(size_t);
	void cmdBitfield();
	void cmdRequest(size_t, size_t, size_t);
	void cmdCancel(size_t, size_t, size_t);
	void cmdPiece(size_t, size_t, unsigned char *piece);

	std::string m_hash;
	state_t m_state;
	bool m_server;
	char m_cmd;
	unsigned char m_remote_peer_id[20];
	unsigned long m_int[3];
	size_t m_integers;
	size_t m_ptr;
	unsigned char *m_slice;
static	int m_next_id;
	int m_id;
	bool m_interest;
	bool m_choke;
	time_t m_t_choke;
	bool m_cts;
	Session *m_sess;
	Peer *m_peer;
	int64_t m_last_r;
	int64_t m_last_w;
	int64_t m_last_r_buf[DLCHECKSIZE];
	int m_last_r_ptr;
	size_t m_length_cmd;
	
};




#endif // _PSOCKET_H

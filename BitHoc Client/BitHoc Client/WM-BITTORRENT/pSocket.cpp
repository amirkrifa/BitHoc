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
* \file pSocket.cpp
* \brief BitTorrent peer to peer connection 
**/


#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

#include "PeerHandler.h"
#include "Session.h"
#include "Peer.h"
#include "Piece.h"
#include "pSocket.h"
#include "LogFile.h"
#define DEB(x) x
#include <sstream>
extern void IntToString(uint32_t i, std::string & o);
extern std::string iClientTorrentsDirectory;
int pSocket::m_next_id = 0;
extern bool bRecoverFromError;
extern size_t iSliceSize;
extern bool ACTIVATE_LOG;

pSocket::pSocket(SocketHandler& h)
:TcpSocket(h,131072,131072)
,m_state(ACCEPT_LENGTH)
,m_server(false)
,m_cmd(0)
,m_slice(new unsigned char[iSliceSize])
,m_id(++m_next_id)
,m_interest(false)
,m_choke(false)
,m_t_choke(time(NULL))
,m_cts(false)
,m_sess(NULL)
,m_peer(NULL)
,m_last_r(0)
,m_last_w(0)
,m_last_r_ptr(0)
,pSocket_log(NULL)
{
	if(ACTIVATE_LOG)
		this->InitLogFile(string(string(".\\")+iClientTorrentsDirectory)+"\\",false);
	SetConnectionRetry(-1); // retry forever
}


pSocket::pSocket(SocketHandler& h,const std::string& hash,unsigned char *p)
:TcpSocket(h,131072,131072)
,m_hash(hash)
,m_state(ACCEPT_LENGTH)
,m_server(false)
,m_cmd(0)
,m_slice(new unsigned char[iSliceSize])
,m_id(++m_next_id)
,m_interest(false)
,m_choke(false)
,m_t_choke(time(NULL))
,m_cts(false)
,m_sess(NULL)
,m_peer(NULL)
,m_last_r(0)
,m_last_w(0)
,pSocket_log(NULL)
{
	m_sess = static_cast<PeerHandler&>(h).GetSession(hash);
	if (!m_sess)
	{
		SetCloseAndDelete();
		return;
	}
	memcpy(m_remote_peer_id, p, 20);
	
}


pSocket::~pSocket()
{
	delete[] m_slice;
	if(pSocket_log)
		delete this->pSocket_log;
}

void pSocket::InitLogFile(std::string ld,bool b)
{
	if(b)
	{
		this->sLogDirectory.assign(ld);
		string pid((char*)m_remote_peer_id);
		if(ACTIVATE_LOG)
			pSocket_log = new LogFile(string(this->sLogDirectory+pid.substr(0,20)+".txt"));
	}else{
		
		this->sLogDirectory.assign(ld);
		if(ACTIVATE_LOG)
			pSocket_log = new LogFile(string(this->sLogDirectory+"listen_socket.txt"));
	}
};
void pSocket::OnAccept()
{	
	static_cast<PeerHandler&>(Handler()).SetPaused();
	char sMessage[100];
	sprintf(sMessage,"(%d)OnAccept", m_id);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	// 0x13
	// 'BitTorrent protocol'
	// 8 x 0x00
	// 20 byte hash
	// 20 byte peer id
	//  (If the receiving side's peer id doesn't match 
	//   the one the initiating side expects, it severs
	//   the connection.)
	m_state = ACCEPT_LENGTH;
	m_server = true;
	static_cast<PeerHandler&>(Handler()).SetNotPaused();
}


void pSocket::OnConnect()
{
	static_cast<PeerHandler&>(Handler()).SetPaused();
	m_peer = m_sess -> GetPeer(GetRemoteAddress());
	if (m_peer)
	{
		m_peer -> SetChoked(false);
		m_peer -> SetInterested(true);
	}
	
		char sMessage[100];
		sprintf(sMessage,"(%d)OnConnect", m_id);
		if(ACTIVATE_LOG)
			this->pSocket_log->add_line(sMessage,true);
	
	SendHello();
	SendBitmap();
	m_state = ACCEPT_LENGTH;
	static_cast<PeerHandler&>(Handler()).SetNotPaused();
}


void pSocket::OnDelete()
{
	static_cast<PeerHandler&>(Handler()).SetPaused();
		char sMessage[100];
		sprintf(sMessage,"(%d)OnDelete", m_id);
		if(ACTIVATE_LOG)
			this->pSocket_log->add_line(sMessage,true);
	static_cast<PeerHandler&>(Handler()).SetNotPaused();
	
}


void pSocket::SendHello()
{
	

	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess)
	{

	SetCloseAndDelete();
	return;
	}
	char buf[68];
	*buf = 0x13;
	memcpy(buf + 1, "BitTorrent protocol", 19);
	memset(buf + 20, 0, 8);
	memcpy(buf + 28, sess -> GetHashptr(), 20);
	memcpy(buf + 48, sess -> GetPeerId(), 20);
	SendBuf(buf, 68);
}


void pSocket::SendBitmap()
{
	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess)
	{
		SetCloseAndDelete();
		return;
	}
	piece_v& pcs = sess -> Complete();
	// TODO: uncomment..
//	if (pcs.size())
	{
		bitmap_t bitmap(sess -> GetNumberOfPieces());
		for (piece_v::iterator it = pcs.begin(); it != pcs.end(); it++)
		{
			Piece *p = *it;
			bitmap.set(p -> GetNumber());
		}
		uint32_t l = htonl(bitmap.GetBitmapSize() + 1);
		
		SendBuf( (char *)&l, 4);
		
		Send("\05");
		SendBuf( (char *)bitmap.GetBitmap(),bitmap.GetBitmapSize());
	}
}


void pSocket::OnRead()
{
	
	static_cast<PeerHandler&>(Handler()).SetPaused();

	if (m_state != STATE_GET_PIECE)
	{	

	}
	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess && m_hash.size() )
	{

		SetCloseAndDelete();
		static_cast<PeerHandler&>(Handler()).SetNotPaused();
		return;
	}
	Peer *peer = m_peer; //sess ? sess -> GetPeer(GetRemoteAddress()) : NULL;
	char slask[100];
	TcpSocket::OnRead();
	while (ibuf.GetLength() && !CloseAndDelete() )
	{
		size_t l = ibuf.GetLength();
		switch (m_state)
		{
		case ACCEPT_LENGTH:
			{
				char c;
				ibuf.Read(&c, 1);
				if (c != 0x13)
				{
	
					SetCloseAndDelete();
					static_cast<PeerHandler&>(Handler()).SetNotPaused();
					return;
				}
				m_state = ACCEPT_PROTOCOL;
			}
			break;
		case ACCEPT_PROTOCOL:
			if (l < 19)
			{
				static_cast<PeerHandler&>(Handler()).SetNotPaused();
				return;
			}
			ibuf.Read(slask, 19);
			slask[19] = 0;
			if (strcmp(slask, "BitTorrent protocol"))
			{

				SetCloseAndDelete();
				static_cast<PeerHandler&>(Handler()).SetNotPaused();
				return;
			}
			m_state = ACCEPT_NULLBYTES;
			break;
		case ACCEPT_NULLBYTES:
			if (l < 8)
			{static_cast<PeerHandler&>(Handler()).SetNotPaused();return;}
			ibuf.Read(slask, 8);
			m_state = ACCEPT_HASH;
			break;
		case ACCEPT_HASH:
			if (l < 20)
			{static_cast<PeerHandler&>(Handler()).SetNotPaused();return;}
			ibuf.Read(slask, 20);
			{
				std::string hash;
				for (size_t i = 0; i < 20; i++)
				{
					unsigned char c;
					char tmp[10];
					memcpy(&c, slask + i, 1);
					sprintf(tmp, "%02x", c);
					hash += tmp;
				}

				if (m_server) // incoming
				{
					// resolve session for incoming connection (m_sess is NULL)
					Session *s = static_cast<PeerHandler&>(Handler()).GetSession(hash);
					if (!s)
					{
						SetCloseAndDelete();
						static_cast<PeerHandler&>(Handler()).SetNotPaused();
						return;
					}
					m_hash = hash;
					m_sess = s;
					m_peer = s -> GetPeer(GetRemoteAddress()); // can be NULL
					sess = s;
					peer = m_peer;
				}
				if (hash != m_hash)
				{

					SetCloseAndDelete();
					static_cast<PeerHandler&>(Handler()).SetNotPaused();
					return;
				}
				if (m_server)
				{
					SendHello();
					SendBitmap();
				}
				m_state = ACCEPT_PEER_ID;
			}
			break;
		case ACCEPT_PEER_ID:
			if (l < 20)
			{static_cast<PeerHandler&>(Handler()).SetNotPaused();return;}
			ibuf.Read(slask, 20);
			slask[20]='\0';
			//  (If the receiving side's peer id doesn't match 
			//   the one the initiating side expects, it severs
			//   the connection.)
			if (!m_server)
			{
				bool ok = true;
				for (size_t i = 0; i < 20 && ok; i++)
					if (m_remote_peer_id[i] != (unsigned char)(slask[i]) )
						ok = false;
				if (!ok)
				{
					{
						if(ACTIVATE_LOG)
							this->pSocket_log->add_line(string("m_remote_peer_id != current_peer_id"),true);
						SetCloseAndDelete();
						static_cast<PeerHandler&>(Handler()).SetNotPaused();
						return;
					}
				}
			}
			else // if (m_server)
			{
				std::string ip = GetRemoteAddress();
				std::string id = static_cast<std::string>(slask).substr(0,20);
				if (sess && !sess -> GetPeer(ip))
				{
					Peer *p = new Peer(dynamic_cast<SocketHandler&>(Handler()),m_hash,ip,id,0);
					sess -> AddPeer(p);
					m_peer = p;
					peer = p;
				}
			}
			m_state = STATE_COMMAND;
			m_cts = true;
			break;
		case STATE_COMMAND:
			if (l < 4)
			{static_cast<PeerHandler&>(Handler()).SetNotPaused();return;}
			{
				uint32_t len;
				ibuf.Read( (char *)&len, 4);
				m_length_cmd = ntohl(len);
				if (len > 0)
				{
					m_state = STATE_COMMAND2;
				}

			}
			break;
		case STATE_COMMAND2:
			{
				ibuf.Read(&m_cmd, 1);
				//while(m_cmd<0 || m_cmd>8)
				//	ibuf.Read(&m_cmd, 1);
				char sMessage[100];
				
				//     * 0 - choke
				//     * 1 - unchoke
				//     * 2 - interested
				//     * 3 - not interested
				//     * 4 - have [piece(integer)]
				//     * 5 - bitfield [bitmap]
				//     * 6 - request [index begin length]
				//     * 7 - piece [index begin piece(byte[])]
				//     * 8 - cancel [index begin length]
				switch (m_cmd)
				{
				case 0:
					sprintf(sMessage, " Choke Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					cmdChoke();
					m_state = STATE_COMMAND;
					break;
				case 1:
					sprintf(sMessage, " Unchoke Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					cmdUnchoke();
					m_state = STATE_COMMAND;
					break;
				case 2:
					sprintf(sMessage, " Interested Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					cmdInterested();
					m_state = STATE_COMMAND;
					break;
				case 3:
					sprintf(sMessage, " Not interested Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					cmdNotinterested();
					m_state = STATE_COMMAND;
					break;
				case 4:
					sprintf(sMessage, " Have [piece(integer)] Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					m_integers = 1;
					m_ptr = 0;
					m_state = STATE_GET_INTEGERS;
					break;
				case 5:
					sprintf(sMessage, " Bitfield [bitmap] Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					m_ptr = 0;
					m_state = STATE_GET_BITMAP;
					break;
				case 6:
					sprintf(sMessage, " Request [index begin length] Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					m_integers = 3;
					m_ptr = 0;
					m_state = STATE_GET_INTEGERS;
					break;
				case 7:
					{
						sprintf(sMessage, " Piece [index begin piece(byte[])] Received: %i", m_cmd);
						if(ACTIVATE_LOG)
							this->pSocket_log->add_line(string(sMessage),true);
						size_t test=ibuf.Space();
						m_integers = 2;
						m_ptr = 0;
						m_state = STATE_GET_INTEGERS;
					}
					break;
				case 8:
					sprintf(sMessage, " Cancel [index begin length] Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					m_integers = 3;
					m_ptr = 0;
					m_state = STATE_GET_INTEGERS;
					break;
				default:
			
					sprintf(sMessage, " Unknown Command Received: %i", m_cmd);
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string(sMessage),true);
					m_ptr = 1;
					m_state = STATE_GET_COMMAND;
					bRecoverFromError = true;
					SetCloseAndDelete();
				}
			}
			break;
		case STATE_GET_INTEGERS:
			if (l < 4)
			{static_cast<PeerHandler&>(Handler()).SetNotPaused();return;}
			uint32_t ll;
			ibuf.Read( (char *)&ll, 4);
			m_int[m_ptr++] = ntohl(ll);
			if (m_ptr >= m_integers)
			{
				switch (m_cmd)
				{
				case 4:
					if (m_int[0] < sess -> GetNumberOfPieces())
					{
						cmdHave(m_int[0]);
						m_state = STATE_COMMAND;
					}
					else
					{
					
						SetCloseAndDelete();
					}
					break;
				case 6:
					if (m_int[0] < sess -> GetNumberOfPieces() &&
						m_int[1] < sess -> GetPieceLength() &&
						m_int[2] < 131072 &&
						m_int[1] + m_int[2] <= sess -> GetPieceLength() &&
						m_int[2] > 0)
					{
						cmdRequest(m_int[0], m_int[1], m_int[2]);
						m_state = STATE_COMMAND;
					}
					else
					{
					
						SetCloseAndDelete();
					}
					break;
				case 7:
					m_ptr = 0;
					m_state = STATE_GET_PIECE;
					break;
				case 8:
					if (m_int[0] < sess -> GetNumberOfPieces() &&
						m_int[1] < sess -> GetPieceLength() &&
						m_int[2] < 131072 &&
						m_int[1] + m_int[2] <= sess -> GetPieceLength() &&
						m_int[2] > 0)
					{
						cmdCancel(m_int[0], m_int[1], m_int[2]);
						m_state = STATE_COMMAND;
					}
					else
					{
						if(ACTIVATE_LOG)
							this->pSocket_log->add_line(string("cmdCancel Error."),true);
						bRecoverFromError=true;
						SetCloseAndDelete();
					}
					break;
				}
			}
			break;
		case STATE_GET_BITMAP:
			if (l < peer -> GetBitmapSize())
			{static_cast<PeerHandler&>(Handler()).SetNotPaused();return;}
			ibuf.Read((char *)peer -> GetBitmap(), peer -> GetBitmapSize());
			cmdBitfield();
			m_state = STATE_COMMAND;
			break;
		case STATE_GET_PIECE:
			if(m_ptr+l<iSliceSize)
			{
				ibuf.Read( (char *)&m_slice[m_ptr], l);
				m_ptr+=l;
			}
			else
			{
				while(m_ptr<iSliceSize)
				{
					ibuf.Read( (char *)&m_slice[m_ptr], 1);
					m_ptr++;
				}
			}
			if (m_ptr == iSliceSize)
			{
				if (m_int[0] < sess -> GetNumberOfPieces() &&
					m_int[1] < sess -> GetPieceLength())
				{
					cmdPiece(m_int[0], m_int[1], m_slice);
					if (peer)
					{
						peer -> RefreshRequests();
					}
					m_state = STATE_COMMAND;
				}
				else
				{

					SetCloseAndDelete();
				}
			}
			break;
		case STATE_GET_COMMAND:
			{
				std::string hex;
				std::string txt;
				char slask[10];
				while (m_ptr < m_length_cmd && l && m_length_cmd<ibuf.GetLength())
				{
					unsigned char c;
					ibuf.Read( (char *)&c,1);
					sprintf(slask,"%02x",c);
					hex += slask;
					sprintf(slask,"%c",isprint((char)c) ? (char)c : '.');
					txt += slask;
					l--;
					m_ptr++;
				}
				
				if (m_ptr == m_length_cmd)
				{
					m_state = STATE_COMMAND;
				}else 
				{
					if(ACTIVATE_LOG)
						this->pSocket_log->add_line(string("STATE_GET_COMMAND Error m_ptr != m_length_cmd"),true);
					SetCloseAndDelete();
				}
			}
			break;
		}
	}
	static_cast<PeerHandler&>(Handler()).SetNotPaused();
}


			//     * 0 - choke 1
			//     * 1 - unchoke 2
			//     * 2 - interested 4
			//     * 3 - not interested 8
			//     * 4 - have [piece(integer)] 16
			//     * 5 - bitfield [bitmap] 32
			//     * 6 - request [index begin length] 64
			//     * 7 - piece [index begin piece(byte[])] 128
			//     * 8 - cancel [index begin length] 256
void pSocket::cmdChoke()
{
	Peer *peer = GetPeer();
	
	char sMessage[100];
	sprintf(sMessage, ">(%d)Choke", m_id);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	if (peer)
	{
		peer -> SetChoked(true);
		peer -> RemoveRequests();
	}
}


void pSocket::cmdUnchoke()
{
	Peer *peer = GetPeer();
	char sMessage[100];
	sprintf(sMessage, ">(%d)Unchoke", m_id);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	if (peer)
	{
		peer -> SetChoked(false);
		peer -> RequestAvailable();
	}
}


void pSocket::cmdInterested()
{
	char sMessage[100];
	sprintf(sMessage, ">(%d)Interested", m_id);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	Peer *peer = GetPeer();
	if (peer)
		peer -> SetInterested(true);
}


void pSocket::cmdNotinterested()
{
	char sMessage[100];
	sprintf(sMessage, ">(%d)Notinterested", m_id);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);

	Peer *peer = GetPeer();
	if (peer)
		peer -> SetInterested(false);
	// We have modified the choking algorithm 
	//if (!m_choke)
	//	SendChoke(true);
}


void pSocket::cmdHave(size_t piece)
{
	char sMessage[100];
	sprintf(sMessage, ">(%d)Have(%d)", m_id, piece);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess)
	{
	
		SetCloseAndDelete();
		static_cast<PeerHandler&>(Handler()).SetNotPaused();
		return;
	}
	Peer *peer = GetPeer();
	if (peer)
	{
		peer -> set(piece);
	}
	sess -> SetUpdateInterested();
}


void pSocket::cmdBitfield()
{

	char sMessage[100];
	sprintf(sMessage,  ">(%d)Bitfield", m_id);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess)
	{
		SetCloseAndDelete();
		static_cast<PeerHandler&>(Handler()).SetNotPaused();
		return;
	}
	sess -> SetUpdateInterested();
	SendChoke(false);
}


void pSocket::cmdRequest(size_t piece, size_t offset, size_t length)
{
	char sMessage[100];
	sprintf(sMessage,  ">(%d)Request(%d,%d,%d)", m_id, piece, offset, length);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);

	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess)
	{
		
		SetCloseAndDelete();
		return;
	}
	SendPiece(piece, offset, length);
}


void pSocket::cmdCancel(size_t piece, size_t offset, size_t length)
{
	char sMessage[100];
	sprintf(sMessage,  ">(%d)Cancel(%d,%d,%d)", m_id, piece, offset, length);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
}


void pSocket::cmdPiece(size_t piece, size_t offset, unsigned char *data)
{
	Session *sess = m_sess; 
	if (!sess)
	{
		SetCloseAndDelete();
		return;
	}
	
	Peer *peer = GetPeer();
	
	size_t length = peer ? peer -> GotSlice(piece, offset) : 0;
	
	if (length)
	{
		// Trafic Log
		std::string sMl;
		sMl.append("R ");
		// Local ip @
		sMl.append(m_sess->GetLocalIpAdr());
		sMl.append(" ");
		// Remote ip @
		sMl.append(this->GetRemoteAddress());
		sMl.append(" ");
		// Number of hops to the remote ip @
		std::ostringstream oss;
		oss<<this->m_peer->GetNumberOfHops();
		sMl.append(oss.str());
		sMl.append(" ");
		 // Number of bytes received from the remote peer
		std::ostringstream oss2;
		oss2<<length;
		sMl.append(oss2.str());
		sMl.append("\n");
		if(ACTIVATE_LOG)
		{	
			sess->AddDBytesToTrafficMatrix(this->GetRemoteAddress(), length);
			sess ->matrix_log->add_line(sMl,false);
		}
		sess -> SaveSlice(piece, offset, length, data);
		sess -> SetCheckComplete();
		//TOVERIFY
		sess -> GenerateRequest(peer);
	}
	else
	{
		char sMessage[100];
		sprintf(sMessage,  "(%d)cmdPiece: no length for slice write", m_id);
		if(ACTIVATE_LOG)
			this->pSocket_log->add_line(sMessage,true);
		
	}
}


Peer *pSocket::GetPeer()
{
	return m_peer;
}


void pSocket::SendInterest(bool interested)
{
	char sMessage[100];
	sprintf(sMessage,  "<(%d)SendInterest(%s)", m_id, interested ? "true" : "false");
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);	
	
	uint32_t l = htonl(1);
	SendBuf( (char *)&l,4);
		
	if (interested)
		Send("\02");
	else
		Send("\03");
	m_interest = interested;
}


void pSocket::SendRequest(size_t piece,size_t offset,size_t length)
{
	char sMessage[100];
	sprintf(sMessage,  "<(%d)SendRequest(%d,%d,%d)", m_id, piece, offset, length);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	uint32_t tmp = htonl(piece);
	char buf[13];
	*buf = 6; // request
	
	memcpy(buf + 1, (char*)&tmp,4);
	tmp = htonl(offset);
	memcpy(buf + 5, (char*)&tmp, 4);
	tmp = htonl(length);
	memcpy(buf + 9,(char*)&tmp, 4);
	uint32_t l = htonl(13);
	SendBuf( (char *)&l,4);
	SendBuf(buf, 13);

}


void pSocket::SendCancel(size_t piece,size_t offset,size_t length)
{
	char sMessage[100];
	sprintf(sMessage,  "<(%d)SendCancel(%d,%d,%d)", m_id, piece, offset, length);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);

	uint32_t tmp = htonl(piece);
	char buf[13];
	*buf = 8; // cancel
	memcpy(buf + 1, (char*)&tmp, 4);
	tmp = htonl(offset);
	memcpy(buf + 5, (char*)&tmp, 4);
	tmp = htonl(length);
	memcpy(buf + 9,(char*)&tmp, 4);
	uint32_t l = htonl(13);
	SendBuf( (char *)&l,4);
	SendBuf(buf, 13);
}


void pSocket::SendChoke(bool choke)
{
	char sMessage[100];
	sprintf(sMessage,   "<(%d)SendChoke(%s)", m_id, choke ? "true" : "false" );
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	std::string sl;
	uint32_t l = htonl(1);
	
	SendBuf( (char *)&l, 4);
	if (choke)
		Send("\00");
	else
		Send("\01");
	m_choke = choke;
	if (choke)
		m_t_choke = time(NULL);

}


void pSocket::SendHave(size_t piece)
{
	char sMessage[100];
	sprintf(sMessage,   "<(%d)SendHave(%d)", m_id, piece );
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);

	uint32_t l = htonl(piece);
	
	uint32_t sz = htonl(5);
	
	SendBuf( (char *)&sz, 4);
	Send("\04");
	SendBuf( (char *)&l, 4);
}

//! Modified by Amir Krifa
void pSocket::SendPiece(size_t piece,size_t offset,size_t length)
{
	char sMessage[100];
	sprintf(sMessage,  "<(%d)SendPiece(%d,%d,%d)", m_id, piece, offset, length);
	if(ACTIVATE_LOG)
		this->pSocket_log->add_line(sMessage,true);
	
	// Trafic Log
	std::string sMl;
	sMl.append("S ");
	// Local ip @
	sMl.append(m_sess->GetLocalIpAdr());
	sMl.append(" ");
	// Remote ip @
	sMl.append(this->GetRemoteAddress());
	sMl.append(" ");
	// Number of hops to the remote ip @
	std::ostringstream oss;
	oss<<this->m_peer->GetNumberOfHops();
	sMl.append(oss.str());
	sMl.append(" ");
    // Number of bytes sent to the remote peer
	std::ostringstream oss2;
	oss2<<length;
	sMl.append(oss2.str());
	sMl.append("\n");
	if(ACTIVATE_LOG)
	{	
		m_sess->matrix_log->add_line(sMl,false);
		m_sess->AddUBytesToTrafficMatrix(this->GetRemoteAddress(), length);
	}

	Session *sess = m_sess; //ref.GetSession(m_hash);
	if (!sess)
	{
		SetCloseAndDelete();
		return;
	}
	sess -> SendSlice(this, piece, offset, length);
}


void pSocket::OnConnectFailed()
{
	static_cast<PeerHandler&>(Handler()).SetPaused();
	Peer *peer = GetPeer();
	if (peer)
		peer -> SetFailed();
	static_cast<PeerHandler&>(Handler()).SetNotPaused();
}


uint64_t pSocket::GetBytesR()
{
	int64_t sz = GetBytesReceived() - m_last_r;
	m_last_r = GetBytesReceived();
	m_last_r_buf[m_last_r_ptr++ % DLCHECKSIZE] = m_last_r;
	return sz;
}


uint64_t pSocket::GetBytesW()
{
	int64_t sz = GetBytesSent() - m_last_w;
	m_last_w = GetBytesSent();
	return sz;
}


bool pSocket::GetDownloadRate(size_t& sz)
{
	if (m_last_r_ptr >= DLCHECKSIZE)
	{
		int ptr = m_last_r_ptr - 1;
		sz = m_last_r_buf[ptr % DLCHECKSIZE] - m_last_r_buf[m_last_r_ptr % 60];
		sz /= DLCHECKSIZE; // bytes / sec
		return true;
	}
	return false;
}


void pSocket::ShowStatus(size_t max)
{
	if (CTS())
	{
		std::string client = "unknown";
		std::string version = "-";
		std::string id = GetPeer() -> GetID();
		switch (id[0])
		{
		case '-':
			if (id.substr(1,2) == "AZ")
				client = "Azureus";
			if (id.substr(1,2) == "BB")
				client = "BitBuddy";
			if (id.substr(1,2) == "CT")
				client = "CTorrent";
			if (id.substr(1,2) == "MT")
				client = "MoonlightTorrent";
			if (id.substr(1,2) == "LT")
				client = "libtorrent";
			if (id.substr(1,2) == "BX")
				client = "Bittorrent X";
			if (id.substr(1,2) == "TS")
				client = "Torrentstorm";
			if (id.substr(1,2) == "TN")
				client = "TorrentDotNET";
			if (id.substr(1,2) == "SS")
				client = "SwarmScope";
			if (id.substr(1,2) == "XT")
				client = "XanTorrent";
			if (id.substr(1,2) == "BS")
				client = "BTSlave";
			if (id.substr(1,2) == "ZT")
				client = "ZipTorrent";
			if (id.substr(1,2) == "AR")
				client = "Arctic";
			if (id.substr(1,2) == "SB")
				client = "Swiftbit";
			if (id.substr(1,2) == "++")
				client = "C++";
			version = id.substr(3,4);
			break;
		case 'S':
			client = "Shadow";
			version = id.substr(1,3);
			break;
		case 'U':
			client = "UPnP NAT Bit Torrent";
			version = id.substr(1,3);
			break;
		case 'T':
			client = "BitTornado";
			version = id.substr(1,3);
			break;
		case 'A':
			client = "ABC";
			version = id.substr(1,3);
			break;
		case 'M':
			client = "Brams";
			version = id.substr(1,7);
			break;
		case 'e':
			client = "BitComet";
			break;
		}
		char up[100];
		char dn[100];
		if (GetBytesSent() / 1024 > 999)
			sprintf(up, "%4d MB", GetBytesSent() / (1024 * 1024));
		else
			sprintf(up, "%4d kB", GetBytesSent() / 1024);
		if (GetBytesReceived() / 1024 > 999)
			sprintf(dn, "%4d MB", GetBytesReceived() / (1024 * 1024));
		else
			sprintf(dn, "%4d kB", GetBytesReceived() / 1024);
		{
			std::string tmp;
			for (size_t i = 0; i < version.size(); i++)
				tmp += isprint(version[i]) ? version[i] : '.';
			version = tmp;
		}
	}
}



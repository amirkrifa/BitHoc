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
* \file Peer.h
* \brief Peer represents a peer status and the main functions used to manage it.
**/

#include "stdafx.h"
#ifndef _PEER_H
#define _PEER_H

#include <string>
#include <socket_include.h>
#include <list>
#include "bitmap_t.h"
#include "Request.h"


class pSocket;

typedef std::list<Request *> request_v;


//! Status of an on- or offline peer

class Peer
{
public:
	Peer(SocketHandler&,const std::string& hash,const std::string& ip,const std::string& id,port_t port);
	Peer(SocketHandler&,const std::string& hash,const std::string& ip,const std::string& id,port_t port, int iNbrHops);
	
	~Peer();

	const std::string& GetIP() { return m_ip; }
	const std::string& GetID() { return m_id; }
	port_t GetPort() { return m_port; }
	void SetPort(port_t x) { m_port = x; }

	SocketHandler& Handler() const { return m_handler; }
	unsigned char *GetBitmap() { return m_bitmap ? m_bitmap -> GetBitmap() : NULL; }
	size_t GetBitmapSize() { return m_bitmap_size; }

	/** choke status from remote end */
	bool IsChoked() { return m_choke; }
	void SetChoked(bool x = true) { m_choke = x; if (x) m_t_choke = time(NULL); }
	/** interest status from remote end */
	bool IsInterested() { return m_interested; }
	void SetInterested(bool x = true) { m_interested = x; }
	time_t GetChokeTime() { return m_t_choke; }

	const std::string& GetHash() { return m_hash; }

	bool Connected();
	pSocket *PeerSocket();

	bool IsSet(size_t nr) { return m_bitmap ? m_bitmap -> IsSet(nr) : false; }
	void set(size_t nr);

	request_v& Requests() { return m_requests; }
	size_t GotSlice(size_t piece,size_t offset);

	// reset age of all request on incoming piece
	void RefreshRequests();

	bool Tried() { return m_tried; }
	void SetTried(bool x = true) { m_tried = x; }
	bool Failed() { return m_failed; }
	void SetFailed(bool x = true) { m_failed = x; }

	/** number of set bits */
	size_t GetSet() { return m_bitmap ? m_bitmap -> GetSet() : 0; }

	/** Peer score. Calculate for each piece:
	 * Peer have: YES  We have: NO   Score: +1
	 * Peer have: NO   We have: YES  Score: +1
	 * Peer have: YES  We have: YES  Score:  0
	 * Peer have: NO   We have: NO   Score:  0
	*/
	size_t Score(bitmap_t *);

	// new
	void RequestAvailable();
	void RemoveRequests();
	
	//! Added by Amir Krifa
	/*!
	* \fn int GetNumberOfHops()
	* \brief Returns the number of hops to this peer.
	*/
	int GetNumberOfHops()
	{
		return iNbrHops;
	};
	/*!
	* \fn void SetNumberOfHops(int i)
	* \brief Sets the number of hops to this peer.
	*/
	void SetNumberOfHops(int i)
	{
		this->iNbrHops=i;
	};
	/*!
	* \fn void SetId(std::string & id)
	* \brief Sets the peer id.
	*/
	void SetId(std::string & id)
	{
		this->m_id.assign(id);
	};
private:
	Peer(const Peer& s) : m_handler(s.Handler()) {} // copy constructor
	Peer& operator=(const Peer& ) { return *this; } // assignment operator

	SocketHandler& m_handler;
	std::string m_hash;
	std::string m_ip;
	std::string m_id;
	port_t m_port;
	bitmap_t *m_bitmap;
	size_t m_bitmap_size;
	bool m_choke;
	bool m_interested;
	time_t m_t_choke;
	request_v m_requests;
	bool m_tried;
	bool m_failed;
	//! Number of hops, introduced to adapt Bittorrent to AdHoc network.
	int iNbrHops;
	
};




#endif // _PEER_H

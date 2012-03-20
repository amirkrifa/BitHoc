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
* \file Peer.cpp
* \brief Implementation of the main functions related to the peer management.
**/

#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <SocketHandler.h>

#include "Session.h"
#include "PeerHandler.h"
#include "pSocket.h"
#include "Piece.h"
#include "Peer.h"

#define DEB(x) x

//! Modified by Amir Krifa.
Peer::Peer(SocketHandler& h,const std::string& hash,const std::string& ip,const std::string& id,port_t port)
:m_handler(h)
,m_hash(hash)
,m_ip(ip)
,m_id(id)
,m_port(port)
,m_bitmap(NULL)
,m_bitmap_size(0)
,m_choke(false)
,m_interested(false)
,m_t_choke(0)
,m_tried(false)
,m_failed(false)
,iNbrHops(-1)
{
	Session *sess = static_cast<PeerHandler&>(Handler()).GetSession(hash);
	srand(time(NULL));
	if (sess)
	{
		size_t n = sess -> GetNumberOfPieces();
		m_bitmap = new bitmap_t(n);
		m_bitmap_size = m_bitmap -> GetBitmapSize();
	}
}

//! Modified by Amir Krifa.
Peer::Peer(SocketHandler& h,const std::string& hash,const std::string& ip,const std::string& id,port_t port, int nbrh)
:m_handler(h)
,m_hash(hash)
,m_ip(ip)
,m_id(id)
,m_port(port)
,m_bitmap(NULL)
,m_bitmap_size(0)
,m_choke(false)
,m_interested(false)
,m_t_choke(0)
,m_tried(false)
,m_failed(false)
,iNbrHops(nbrh)
{
	Session *sess = static_cast<PeerHandler&>(Handler()).GetSession(hash);
	srand(time(NULL));
	if (sess)
	{
		size_t n = sess -> GetNumberOfPieces();
		m_bitmap = new bitmap_t(n);
		m_bitmap_size = m_bitmap -> GetBitmapSize();
	}
}

//! Modified by Amir Krifa.
Peer::~Peer()
{
	if (m_bitmap)
		delete m_bitmap;
	RemoveRequests();
	while (m_requests.size())
	{
		request_v::iterator it = m_requests.begin();
		Request *r = *it;
		delete r;
		m_requests.erase(it);
	}
	if(PeerSocket())
		PeerSocket()->SetCloseAndDelete();
}


bool Peer::Connected()
{
	pSocket *p = PeerSocket();
	return p ? static_cast<PeerHandler&>(Handler()).Connected(this) && p -> CTS() : false;
}


pSocket *Peer::PeerSocket()
{
	return static_cast<PeerHandler&>(Handler()).PeerSocket(this);
}


void Peer::set(size_t piece)
{
	if (m_bitmap)
		m_bitmap -> set(piece);
}


size_t Peer::GotSlice(size_t piece,size_t offset)
{
	for (request_v::iterator it = m_requests.begin(); it != m_requests.end(); it++)
	{
		Request *r = *it;
		if (r -> GetPiece() == piece && r -> GetOffset() == offset)
		{
			size_t l = r -> GetLength();
			delete r;
			m_requests.erase(it);
			// TODO: reset request timeout here
			return l;
		}
	}

	// slice was not in request queue, get slice length from incomplete piece list
	Session *sess = static_cast<PeerHandler&>(Handler()).GetSession(m_hash);
	if (!sess)
		return 0;
	Piece *p = sess -> GetIncomplete(piece);
	if (p)
		return MIN(iSliceSize, p -> PieceLength() - offset);
	return 0;
}


void Peer::RefreshRequests()
{
	for (request_v::iterator it = m_requests.begin(); it != m_requests.end(); it++)
	{
		Request *r = *it;
		r -> ResetAge();
	}
}


size_t Peer::Score(bitmap_t *bitmap)
{
	if (!bitmap || !m_bitmap)
		return 0;
	size_t q = 0;
	for (size_t i = 0; i < bitmap -> GetNumberOfPieces(); i++)
	{
		if (bitmap -> IsSet(i) != m_bitmap -> IsSet(i))
			q++;
	}
	return q;
}


void Peer::RequestAvailable()
{
	Session *sess = static_cast<PeerHandler&>(Handler()).GetSession(m_hash);
	if (!sess)
		return;
	sess -> RequestAvailable(this);
}


void Peer::RemoveRequests()
{
	Session *sess = static_cast<PeerHandler&>(Handler()).GetSession(m_hash);
	if (!sess)
		return;
	sess -> RemoveRequests(this);
}



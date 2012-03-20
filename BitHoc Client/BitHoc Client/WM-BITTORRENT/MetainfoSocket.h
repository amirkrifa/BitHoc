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
* \file MetaInfoSocket.h
* \brief Class used to load the MetaInfo File and to load the new Download session.
**/

#include "stdafx.h"
#ifndef _METAINFOSOCKET_H
#define _METAINFOSOCKET_H
#include <TcpSocket.h>
#include <SocketHandler.h>


//! Sending and receiving end of metainfo file transfer
class pSocket;
class MetainfoSocket : public TcpSocket
{
	enum state_t {
		STATE_GET_LENGTH = 0,
		STATE_GET_FILE,
	};
public:
	MetainfoSocket(SocketHandler& );
	MetainfoSocket(SocketHandler& ,const std::string& filename);
	~MetainfoSocket();
	void SetLocalSocket(pSocket *ps)
	{
		this->pLocalSocket=ps;
	};
	void OnAccept();
	void OnConnect();
//	void OnRawData(const char *,size_t);
	void OnDelete();
	void OnRead();
	
	//! Added by Amir Krifa.
	/*!
	* \fn void LoadMetaInfoFile(bool bSeeding,string MetaInfoFilePath)
	* \brief Used to load a given meta info file and to start the downloading process. 
	*/
	void LoadMetaInfoFile(bool bSeeding,string MetaInfoFilePath);
	/*!
	* \fn void SetMetaInfoFileToLoad(string lf)
	* \brief Sets the Meta info file name. 
	*/
	void SetMetaInfoFileToLoad(string lf){this->m_filename=lf;};
	
private:
	MetainfoSocket(const MetainfoSocket& s) : TcpSocket(s) {} // copy constructor
	MetainfoSocket& operator=(const MetainfoSocket& ) { return *this; } // assignment operator
	void InitSession();
	static	int m_nr;
	int m_filenr;
	string m_filename;
	FILE *m_fil;
	state_t m_state;
	off_t m_sz;
	off_t m_sz_read;
	pSocket * pLocalSocket;
};




#endif // _METAINFOSOCKET_H

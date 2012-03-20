
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
#include "stdafx.h"
#ifndef _STATUSSOCKET_H
#define _STATUSSOCKET_H

#include <HttpdSocket.h>
#include <SocketHandler.h>
#include "PeerHandler.h"

#define COOKIE_DOMAIN ""
#define COOKIE_PATH "/"

//! Small web server interface

class StatusSocket : public HttpdSocket
{
public:
	StatusSocket(SocketHandler&);
	~StatusSocket();

	void Exec();

	void GetEnvironment();
	void ReadCookies();
	void FormInput();
	void CreateHeader();
	void GenerateDocument();

private:
	StatusSocket(const StatusSocket& s) : HttpdSocket(s) {} // copy constructor
	StatusSocket& operator=(const StatusSocket& ) { return *this; } // assignment operator
	void SetTheCookie();
	void menyval(int val,int sheet,const std::string& text);

	std::string m_tmp;
	int m_sheet;
};




#endif // _STATUSSOCKET_H

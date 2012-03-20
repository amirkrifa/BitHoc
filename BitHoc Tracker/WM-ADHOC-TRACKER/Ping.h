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

/*!
* \file Ping.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _PING_H
#define _PING_H

#pragma once

#include <stdio.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
/*!
* \class Ping 
* \brief Class used to send a ping request to a given device and to return the result.
*/
class Ping
{
public:
	Ping(void);
	/*!
	* \fn char * send_a_ping(char * ip_adr)
	* \brief Function used to send a ping request to a given device and to return the result.
	*/
	char * send_a_ping(char * ip_adr);
public:
	~Ping(void);
};


#endif _PING_H
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
* \file SpyThreadParam.cpp
* \brief Implementation of the class SpyThreadParam used to transmit the parameters needed for the thread we used to catch any change in the device routing table. 
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "StdAfx.h"
#include "SpyThreadParam.h"
#include "WMRoutingTable.h"
SpyThreadParam::SpyThreadParam(WMRoutingTable * rt)
{
	this->wmrt=rt;
}

SpyThreadParam::~SpyThreadParam(void)
{
	this->wmrt=NULL;
	this->ptr_nm=NULL;
}
void SpyThreadParam::rt_is_modified()
{
	rt_was_modified=true;
	this->wmrt->routing_table_was_modified=true;
	wmrt->update_rt_map();
}


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
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include "Request.h"




Request::Request(size_t p,size_t o,size_t l)
:m_piece(p)
,m_offset(o)
,m_length(l)
{

	m_time = time(NULL);
}


Request::~Request()
{
}





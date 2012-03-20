
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
#ifndef _REQUEST_H
#define _REQUEST_H

#include <string>


//! outstanding/filled requests for incomplete pieces 

class Request
{
public:
	Request(size_t piece,size_t offset,size_t length);
	~Request();
	
	time_t Age() { return time(NULL) - m_time; }
	bool Equals(size_t piece,size_t offset,size_t length) {
		return piece == m_piece &&
			offset == m_offset &&
			length == m_length;
	}

	void ResetAge() { m_time = time(NULL); }

	size_t GetPiece() { return m_piece; }
	size_t GetOffset() { return m_offset; }
	size_t GetLength() { return m_length; }

private:
	Request(const Request& ) {} // copy constructor
	Request& operator=(const Request& ) { return *this; } // assignment operator

	size_t m_piece;
	size_t m_offset;
	size_t m_length;
	time_t m_time;
};




#endif // _REQUEST_H

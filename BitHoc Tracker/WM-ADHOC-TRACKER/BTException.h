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
#ifndef _BTEXCEPTION_H
#define _BTEXCEPTION_H

#include "Exception.h"


//! Exception handling for metainfo file

class BTException : public Exception
{
public:
	BTException(const std::string& e) : Exception(e) {}

	BTException(const BTException& e) : Exception(e.GetText()) {} // copy constructor
	BTException& operator=(const BTException& ) { return *this; } // assignment operator

};




#endif _BTEXCEPTION_H

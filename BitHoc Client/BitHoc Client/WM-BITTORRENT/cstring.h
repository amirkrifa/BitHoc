
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

#ifndef _CSTRING_H
#define _CSTRING_H

#include <string>


//! STL string with color functionality (ANSI)

class cstring : public std::string
{
public:
	cstring();
	cstring(const std::string&);
	~cstring();

//	const cstring& 
	void operator=(const char *);
	void operator+=(const char *);
	void operator+=(const std::string&);

	bool operator==(const std::string&);
	bool operator==(class cstring&);
	bool operator==(const char *);

	const char *c_str();
	const char *uc_str();

private:
	char *m_cstr;
	char *m_ucstr;
};




#endif // _CSTRING_H

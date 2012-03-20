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


#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <string>


//! Exception handling base class

class Exception
{
public:
	Exception(const std::string& t) : m_t(t) {}

	const std::string& GetText() const { return m_t; }

protected:
	Exception(const Exception& e) : m_t(e.GetText()) {} // copy constructor
	Exception& operator=(const Exception& ) { return *this; } // assignment operator

private:
	std::string m_t;
};




#endif // _EXCEPTION_H



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
#ifndef _CLOCK_H
#define _CLOCK_H


#ifdef _WIN32
#pragma warning(disable:4786)
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <string>


//! millisecond timer for debug printing

class Clock
{
public:
	Clock();
	~Clock();

	void PrintDiff(const std::string& = "");
	void Reset() {
#ifdef _WIN32
		::GetLocalTime(&m_time);
#else
		gettimeofday(&m_tv, &m_tz);
#endif
	}

private:
	Clock(const Clock& ) {} // copy constructor
	Clock& operator=(const Clock& ) { return *this; } // assignment operator

#ifdef _WIN32
	SYSTEMTIME m_time;
	SYSTEMTIME m_time0;
#else
	struct timeval m_tv;
	struct timeval m_tv0;
	struct timezone m_tz;
	struct timezone m_tz0;
#endif
};




#endif // _CLOCK_H

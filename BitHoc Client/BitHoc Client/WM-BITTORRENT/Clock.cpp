
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
#include <stdio.h>

#include "Clock.h"




Clock::Clock()
{
#ifdef _WIN32
	::GetLocalTime(&m_time);
	::GetLocalTime(&m_time0);
#else
	gettimeofday(&m_tv, &m_tz);
	gettimeofday(&m_tv0, &m_tz0);
#endif
}


Clock::~Clock()
{
}


void Clock::PrintDiff(const std::string& str)
{
#ifdef _WIN32
	SYSTEMTIME time;
	SYSTEMTIME time0;
	::GetLocalTime(&time);
	::GetLocalTime(&time0);
	time.wSecond -= m_time.wSecond;
	if (time.wSecond < 0)
		time.wSecond += 60;
	{
		time_t t = time.wSecond * 1000 + time.wMilliseconds - m_time.wMilliseconds;
		time_t s = t / 1000;
		time_t m = t % 1000;

	}
	time0.wSecond -= m_time0.wSecond;
	if (time0.wSecond < 0)
		time0.wSecond += 60;
	{
		time_t t = time0.wSecond * 1000 + time0.wMilliseconds - m_time0.wMilliseconds;
		time_t s = t / 1000;
		time_t m = t % 1000;

	}
#else
	struct timeval tv;
	struct timezone tz;
	struct timeval tv0;
	struct timezone tz0;
	gettimeofday(&tv, &tz);
	gettimeofday(&tv0, &tz0);
	tv.tv_sec -= m_tv.tv_sec;
	{
		time_t t = tv.tv_sec * 1000000 + tv.tv_usec - m_tv.tv_usec;
		time_t s = t / 1000000;
		time_t u = t % 1000000;
		if (!str.size())
			printf("Time: %d.%06d", s, u);
		else
			printf("Time %s: %d.%06d", str.c_str(), s, u);
	}
	tv0.tv_sec -= m_tv0.tv_sec;
	{
		time_t t = tv0.tv_sec * 1000000 + tv0.tv_usec - m_tv0.tv_usec;
		time_t s = t / 1000000;
		time_t u = t % 1000000;
		if (!str.size())
			printf(" / %d.%06d", s, u);
		else
			printf(" / %d.%06d", s, u);
	}
	printf("\n");
#endif
	Reset();
}



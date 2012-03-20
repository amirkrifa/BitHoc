
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
* \file UsefulFunctions.h
* \brief Includes the implementation of some useful functions which will be used in many parts of the project.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _USEFUL_FUNCTIONS_H
#define _USEFUL_FUNCTIONS_H

#include <sstream>
#include <winbase.h>
#include <windows.h>
#include <time.h>
#pragma once

#if defined(_MSC_VER) || defined(_MSC_EXTENTIONS)
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64
#else 
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#endif

/*!

*/
size_t GetFileSizeWin32(const TCHAR *fileName)
{
    BOOL                        fOk;
    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

    if (NULL == fileName)
        return 0;

    fOk = GetFileAttributesEx(fileName, GetFileExInfoStandard, (void*)&fileInfo);
    if (!fOk)
        return 0;
    
    return (long)fileInfo.nFileSizeLow;
}


LPWSTR ConvertLPCSTRToLPWSTR (char* pCstring)

{

    LPWSTR pszOut = NULL;

    if (pCstring != NULL)

    {

        int nInputStrLen = strlen (pCstring);

        // Double NULL Termination

        int nOutputStrLen = MultiByteToWideChar(CP_ACP, 0, pCstring, nInputStrLen, NULL, 0) + 2;

        pszOut = new WCHAR [nOutputStrLen];

        if (pszOut)

        {

            memset (pszOut, 0x00, sizeof (WCHAR)*nOutputStrLen);

            MultiByteToWideChar (CP_ACP, 0, pCstring, nInputStrLen, pszOut, nInputStrLen);

        }

    }

    return pszOut;

}


void IntToString(uint32_t i, std::string & o)
{
	std::ostringstream oss;
    oss << i;
    o = oss.str();

}

template< typename T> T GetMax( T & a, T & b)
{
	return a>b?a:b;
}

template< typename T> T GetMin( T & a, T & b)
{
	return a<b?a:b;
}

/*
int gettimeofday(struct timeval *tv, struct timezone_s *tz)
{
	SYSTEMTIME time;
	FILETIME ft;
	
	
	unsigned _int64 tmpres=0;
	static int tzflag;

	if(tv!=NULL)
	{
		GetLocalTime(&time);
		SystemTimeToFileTime(&time,&ft);
		tmpres|= ft.dwHighDateTime;
		tmpres<<=32;
		tmpres|=ft.dwLowDateTime;

		// Converting file time to unix Epoch
		tmpres/=10; //convert into milliseconds
		tmpres-=DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec=(long)(tmpres/1000000UL);
		tv->tv_usec=(long)(tmpres%1000000UL);
	}
	if(tz!=NULL)
	{
		if(!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest=_timezone/60;
		tz->tz_dsttime=_daylight;
	}
	return 0;
}
*/
#endif _USEFUL_FUNCTIONS_H

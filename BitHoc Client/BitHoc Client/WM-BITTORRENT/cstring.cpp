

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
#undef cstring
#include "color.h"
#include "cstring.h"

// vs won't compile when I write out 'std::string' everywhere - blah
using namespace std;


cstring::cstring()
:string()
,m_cstr(NULL)
,m_ucstr(NULL)
{
}


cstring::cstring(const std::string& p)
:string(p)
,m_cstr(NULL)
,m_ucstr(NULL)
{
}


cstring::~cstring()
{
	if (m_cstr)
		delete m_cstr;
	if (m_ucstr)
		delete m_ucstr;
}


const char *cstring::c_str()
{
	size_t n = size();
	std::string tmp;

	for (size_t i = 0; i < n; i++)
	{
		if ((*this)[i] == '&')
		{
			i++;
			switch ((*this)[i])
			{
			case 'n':
				tmp += CNRM;
				break;
			case 'r':
				tmp += CRED;
				break;
			case 'g':
				tmp += CGRN;
				break;
			case 'y':
				tmp += CYEL;
				break;
			case 'b':
				tmp += CBLU;
				break;
			case 'm':
				tmp += CMAG;
				break;
			case 'c':
				tmp += CCYN;
				break;
			case 'w':
				tmp += CWHT;
				break;
			case 'l':
				tmp += CBLK;
				break;
			case 'R':
				tmp += BRED;
				break;
			case 'G':
				tmp += BGRN;
				break;
			case 'Y':
				tmp += BYEL;
				break;
			case 'B':
				tmp += BBLU;
				break;
			case 'M':
				tmp += BMAG;
				break;
			case 'C':
				tmp += BCYN;
				break;
			case 'W':
				tmp += BWHT;
				break;
			case 'L':
				tmp += BBLK;
				break;
			case 'e':
				tmp += BKRED;
				break;
			case 'k':
				tmp += BKGRN;
				break;
			case 'p':
				tmp += BKYEL;
				break;
			case 'u':
				tmp += BKBLU;
				break;
			case 'a':
				tmp += BKMAG;
				break;
			case 'f':
				tmp += BKCYN;
				break;
			case 'h':
				tmp += BKWHT;
				break;
			case 'i':
				tmp += BKBLK;
				break;
			case 'U':
				tmp += UNDER;
				break;
			case 'F':
				tmp += FLASH;
				break;
			case '&':
				tmp += (*this)[i];
				break;
			default:
				i--;
				tmp += (*this)[i];
				break;
			} // switch ((*this)[i])
		}
		else
		if ((*this)[i] == '\n')
		{
			tmp += '\r';
			tmp += (*this)[i];
		}
		else
		{
			tmp += (*this)[i];
		}
	}
	if (m_cstr)
		delete m_cstr;
	m_cstr = new char[tmp.size() + 1];
	memcpy(m_cstr,tmp.c_str(),tmp.size());
	m_cstr[tmp.size()] = 0;

	return m_cstr;
}


const char *cstring::uc_str()
{
	size_t n = size();
	std::string tmp;

	for (size_t i = 0; i < n; i++)
	{
		if ((*this)[i] == '&')
		{
			i++;
			switch ((*this)[i])
			{
			case 'n':
				break;
			case 'r':
				break;
			case 'g':
				break;
			case 'y':
				break;
			case 'b':
				break;
			case 'm':
				break;
			case 'c':
				break;
			case 'w':
				break;
			case 'l':
				break;
			case 'R':
				break;
			case 'G':
				break;
			case 'Y':
				break;
			case 'B':
				break;
			case 'M':
				break;
			case 'C':
				break;
			case 'W':
				break;
			case 'L':
				break;
			case 'e':
				break;
			case 'k':
				break;
			case 'p':
				break;
			case 'u':
				break;
			case 'a':
				break;
			case 'f':
				break;
			case 'h':
				break;
			case 'i':
				break;
			case 'U':
				break;
			case 'F':
				break;
			case '&':
				tmp += (*this)[i];
				break;
			default:
				i--;
				tmp += (*this)[i];
				break;
			} // switch ((*this)[i])
		}
		else
		{
			tmp += (*this)[i];
		}
	}
	if (m_ucstr)
		delete m_ucstr;
	m_ucstr = new char[tmp.size() + 1];
	memcpy(m_ucstr,tmp.c_str(),tmp.size());
	m_ucstr[tmp.size()] = 0;

	return m_ucstr;
}


bool cstring::operator==(const std::string& str)
{
	return !strcmp(uc_str(),str.c_str());
}


bool cstring::operator==(cstring& str)
{
	return !strcmp(uc_str(),str.uc_str());
}


bool cstring::operator==(const char *p)
{
	return !strcmp(uc_str(),p);
}


//const cstring& 
void cstring::operator=(const char *p)
{
//	return 
	string::operator=(p);
}


void cstring::operator+=(const char *p)
{
	string::operator+=(p);
}


void cstring::operator+=(const std::string& p)
{
	string::operator+=(p);
}



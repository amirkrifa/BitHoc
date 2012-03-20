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

/*!
* \file BString.cpp
**/

#include "stdafx.h"
#pragma warning(disable:4786)



#include <stdio.h>
#include <ctype.h>
//#include <assert.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <openssl/sha.h>
#include "BTObject.h"
#include "BTDictionary.h"
#include "BTList.h"
#include "BTInteger.h"
#include "BTString.h"
#include "BString.h"
#include "ParseBT.h"
#include <sstream>

#define DEB(x) 
extern size_t GetFileSizeWin32(const TCHAR *fileName);
extern LPWSTR ConvertLPCSTRToLPWSTR (char* pCstring);

BString::BString()
:m_bindex(0)
,m_root(0)
,m_current(NULL)
,m_state(0)
,m_strlen(0)
,m_str("")
,m_name("")
,m_bDictionary(false) // key-value pairs
,m_bList(false) // values
,m_bKey(false) // used when m_bDictionary is true
{
	m_current = &m_root;
}


BString::~BString()
{
}
void BString::read_file_2(FILE *fil)
{
	char c;

	fread(&c,1,1,fil);
	while (!feof(fil))
	{
		if (!reg_char(c))
		{
			break;
		}
		//
		fread(&c,1,1,fil);
	}
}

void BString::read_file(FILE *fil, string &sFileFullPath)
{
	// Added by Amir Krifa
	// Get the size of the file
	size_t uFileSize=GetFileSizeWin32(ConvertLPCSTRToLPWSTR ((char*)sFileFullPath.c_str()));

	char * szFileBuffer=(char * ) malloc(uFileSize);

	char x;
	int iNumberOfCaracters=0;
	while (!feof(fil)&&iNumberOfCaracters< abs(uFileSize))
	{
				
		x=fgetc(fil);
		
		szFileBuffer[iNumberOfCaracters]=x;
		
		iNumberOfCaracters++;
	}
	
	this->read_buf(szFileBuffer,uFileSize);
	free(szFileBuffer);
}


void BString::read_buf(const char *p,size_t l)
{
	for (size_t i = 0; i < l; i++)
	{
		if (!reg_char(p[i]))
		{
			break;
		}
	}
}


DEB(
int lvl = 0;
char lt[100];

void plvl(int x)
{

}
)


bool BString::reg_char(char c)
{
	m_bstring += c;
	switch (m_state)
	{
	case 0:  
			switch (c)
			{
				case 'd': // dictionary
					AddDictionary();
					break;

				case 'l': // list
					AddList();
					break;

				case 'i': // integer
					m_state = 1;
					m_str = "";
					break;

				case 'e': // lvl end
					End();
					break;

				default:
					if (!isdigit(c))
						{
							//assert(!"no good");
							//throw(BTException("Digit expected"));
						}
					// string
					m_state = 2;
					m_str = c;
			}
			break;
	
	case 1: // read integer
			if (c == 'e')
				{
					m_state = 0;
					AddInteger( m_str );
				}
				else
					{
						m_str += c;
					}
			break;
	
	case 2: // read string length
			if (isdigit(c))
				{
					m_str += c;
				}
				else
					{
						m_strlen = atoi(m_str.c_str());
						m_str = "";
	
						//assert(c == ':');
						if (c != ':')
							{
								//throw(BTException("':' expected"));
							}
			if (m_strlen)
				{
					m_state = 3;
				}
				else
					{
					// add empty string
					m_state = 0;
					AddString( m_str );
					}
			}
			break;
	
	case 3: // read string
			m_str += c;
			m_strlen--;
			if (m_strlen <= 0)
				{
					m_state = 0;
					AddString( m_str );
				}
			break;
	}
	m_current -> Increase();
	m_bindex++;
	return true;
}


void BString::AddDictionary()
{
	BTDictionary *tmp = new BTDictionary(m_bindex);
	
	tmp -> SetParent(m_current);

	switch (m_current -> Type())
	{
	case 'd': // dictionary
			m_current -> AddObject(m_name, tmp);
			m_current = tmp;
			m_name = "";
			break;
	case 'l': // list
			m_current -> AddObject(tmp);
			m_current = tmp;
			break;
	default:
			//assert(!"no good");
			//throw(BTException("AddDictionary"));
			break;
	}
}


void BString::AddList()
{
	BTList *tmp = new BTList(m_bindex);
	
	tmp -> SetParent(m_current);

	switch (m_current -> Type())
	{
	case 'd': // dictionary
			m_current -> AddObject(m_name, tmp);
			m_current = tmp;
			m_name = "";
			break;
	case 'l': // list
			m_current -> AddObject(tmp);
			m_current = tmp;
			break;
	default:
			//assert(!"no good");
			//throw(BTException("AddList"));
			break;
	}
}


void BString::AddInteger(const std::string &str)
{
	BTInteger *tmp = new BTInteger;
	
	tmp -> SetParent(m_current);
	tmp -> SetValue(str);

	switch (m_current -> Type())
	{
	case 'd': // dictionary
			m_current -> AddObject(m_name, tmp);
			m_name = "";
			break;
	case 'l': // list
			m_current -> AddObject(tmp);
			break;
	default:
			//throw(BTException("AddInteger"));
			//assert(!"no good");
			break;
	}
}


void BString::AddString(const std::string &str)
{
	if (m_current -> Type() == 'd' && m_name.size() == 0)
	{
		m_name = str;
		return;
	}
	BTString *tmp = new BTString;

	tmp -> SetParent(m_current);
	tmp -> SetValue(str);

	switch (m_current -> Type())
	{
	case 'd': // dictionary
			m_current -> AddObject(m_name, tmp);
			m_name = "";
			break;
	case 'l': // list
			m_current -> AddObject(tmp);
			break;
	default:
		//	throw(BTException("AddString"));
			//assert(!"no good");
			break;
	}
}


void BString::End()
{
	m_current = m_current -> GetParent();
}


void BString::Show()
{
	//m_root.Show();

}


BTObject *BString::GetBTObject(const std::string &path)
{
	ParseBT pa( (char *)path.c_str(), ".", 1);
	char slask[200];
	BTObject *p = &m_root;
	BTList *l = dynamic_cast<BTList *>(p);
	//	assert(l && l -> GetSize() == 1);
	if (!l || l -> GetSize() != 1)
	{
		throw(string("GetBTObject"));
	}
	p = l -> GetFirst();
	pa.getword(slask);
	while (*slask)
	{
		BTDictionary *d = dynamic_cast<BTDictionary *>(p);
		//		assert(d);
		if (!d)
		{
			throw(string("Not Dictionary"));
		}
		p = d -> Find(slask);
		if (!p)
			return NULL;
		pa.getword(slask);
	}
	return p;
}


BTInteger *BString::GetInteger(const std::string &str)
{
	BTObject *p = GetBTObject(str);
	return dynamic_cast<BTInteger *>(p);
}


BTString *BString::GetString(const std::string &str)
{
	BTObject *p = GetBTObject(str);
	return dynamic_cast<BTString *>(p);
}


std::string BString::GetBString(const std::string& path)
{
	BTObject *p = GetBTObject(path);
	size_t begin = p -> GetIndex();
	size_t length = p -> GetLength() + 1;
	std::string str = m_bstring.substr(begin,length);
	//printf("%s\n",printable(str).c_str());
	return str;
}


unsigned char *BString::GetHash(const std::string& node)
{
	std::string tmp = GetBString( node );
	SHA1( (unsigned char *)tmp.c_str(), tmp.size(), m_hash);

	return m_hash;
}

// Modified by Amir Krifa
const std::string& BString::GetHashAsString(const std::string& node)
{
	
	std::string tmp = GetBString( node );
	char slask[10];
	SHA1( (unsigned char *)tmp.c_str(), tmp.size(), m_hash);
	
	//istringstream iss((char*)m_hash);
	//string sHash;
	//std::getline(iss,sHash);
	
	// Correct the GetLine Bug
	//if(sHash.length()>20)
	//	sHash=sHash.substr(0,sHash.length()-1);
	

	string res;
	*slask = 0;
	for (size_t i = 0; i < 20; i++)
	{	
		strcpy(slask,"\0");
		sprintf(slask, "%02x", m_hash[i]);
	res.append(slask);
	}
	m_hash_str = res;
	
	return m_hash_str;
}



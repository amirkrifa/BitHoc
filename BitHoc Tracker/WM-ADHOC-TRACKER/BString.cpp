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
//! Header deleted by Amir Krifa
//#include <assert.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "BTObject.h"
#include "BTDictionary.h"
#include "BTList.h"
#include "BTInteger.h"
#include "BTString.h"
#include "BString.h"
#include "ParseBT.h"
#include "BTException.h"

#define DEB(x) 


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


//! Function added by Amir Krifa
void BString::read_file(FILE *fil)
{
	
	char c[1];
	char test;

	string buffer;
	c[0]=fgetc(fil);
	c[1]='\0';
	test=c[0];
	buffer.append(c);
	while (!(c[0]==EOF && test=='e') )
	{
		test=c[0];
		c[0]=fgetc(fil);
		c[1]='\0';
		if(!(c[0]==EOF && test=='e'))
			buffer.append(c);
	}
	this->read_buf(buffer.c_str(),buffer.length());
}


//! Procedure modified by Amir Krifa
void BString::read_buf(const char *p,size_t l)
{
	for (size_t i = 0; i < l; i++)
	{
		if (!reg_char(p[i]))
		{
			break;
		}
	}
	std::string stop;
}


DEB(
int lvl = 0;
char lt[100];

void plvl(int x)
{

}
)


//! Function modified by Amir Krifa
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
							throw(BTException("Digit expected"));
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
	
						if (c != ':')
							{
								throw(BTException("':' expected"));
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


//! Procedure modified by Amir Krifa
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
			throw(BTException("AddDictionary"));
			break;
	}
}

//! Procedure modified by Amir Krifa
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
			throw(BTException("AddList"));
			break;
	}
}


//! Procedure modified by Amir Krifa
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
			throw(BTException("AddInteger"));
			break;
	}
}

//! Procedure modified by Amir Krifa
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
			throw(BTException("AddString"));
			break;
	}
}


void BString::End()
{
	m_current = m_current -> GetParent();
}


// Function modified by Amir Krifa
void BString::Show()
{
	//m_root.Show();
	
}


// Function modified by Amir Krifa
BTObject *BString::GetBTObject(const std::string &path)
{
	ParseBT pa( (char *)path.c_str(), ".", 1);
	char slask[200];
	BTObject *p = &m_root;
	BTList *l = dynamic_cast<BTList *>(p);
	if (!l || l -> GetSize() != 1)
	{
		throw(BTException("GetBTObject"));
	}
	p = l -> GetFirst();
	pa.getword(slask);
	while (*slask)
	{
		BTDictionary *d = dynamic_cast<BTDictionary *>(p);
		if (!d)
		{
			throw(BTException("Not Dictionary"));
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
	
	return str;
}


unsigned char *BString::GetHash(const std::string& node)
{
	return m_hash;
}


const std::string& BString::GetHashAsString(const std::string& node)
{

	return m_hash_str;
}



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
* \file BString.h
**/

#ifndef _BSTRING_H
#define _BSTRING_H

#include "BTObject.h"
#include "BTList.h"
#include "BTInteger.h"
#include "BTString.h"

#define MAXL 100


//! metainfo file reader

class BString
{
public:
	BString();
	~BString();

	void read_file(FILE *fil,std::string &x);
	void read_buf(const char *,size_t);

	void Show();

	BTObject *GetBTObject(const std::string &);
	BTInteger *GetInteger(const std::string &);
	BTString *GetString(const std::string &);
	std::string GetBString(const std::string& path);

	std::string printable(const std::string& s) {
		std::string str;
		for (size_t i = 0; i < s.size(); i++)
		{
			if (isprint(s[i]))
				str += s[i];
			else
				str += ".";
		}
		return str;
	}
	size_t GetIndex() { return m_bindex; }
	unsigned char *GetHash(const std::string& node);
	const std::string& GetHashAsString(const std::string& node);
	void read_file_2(FILE *fil);
private:
	bool reg_char(char);
	void AddDictionary();
	void AddList();
	void AddInteger(const std::string &);
	void AddString(const std::string &);
	void End();
	//
	std::string m_bstring;
	size_t m_bindex;
	BTList m_root;
	BTObject *m_current;
	int m_state;
	int m_strlen;
	std::string m_str;
	std::string m_name;
	bool m_bDictionary;
	bool m_bList;
	bool m_bKey;
	//
	unsigned char m_hash[20];
	std::string m_hash_str;
};


#endif _BSTRING_H


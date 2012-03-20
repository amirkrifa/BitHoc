
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

#ifndef _BTDICTIONARY_H
#define _BTDICTIONARY_H

#include "BTObject.h"
#include <map>

typedef std::map<std::string,BTObject *> btmap_t;


//! map in metainfo file

class BTDictionary : public BTObject
{
public:
	BTDictionary(size_t i) : BTObject(i) {}
	~BTDictionary() {
		for (btmap_t::iterator it = m_map.begin(); it != m_map.end(); it++)
		{
			delete (*it).second;
		}
	}
	char Type() { return 'd'; }
	void Show() {
		bool first = true;
		BTObject *p = GetParent();
		printf("\n");
		printf("Show Dictionary, length %d\n",GetLength());
		while (p)
		{
			printf("  ");
			p = p -> GetParent();
		}
		printf("( ");
		for (btmap_t::iterator it = m_map.begin(); it != m_map.end(); it++)
		{
			if (!first)
				printf(", ");
			first = false;
			printf("%s: ",(*it).first.c_str());
			(*it).second -> Show();
		}
		printf(")");
	}
	BTObject *Find(const std::string &str) {
		for (btmap_t::iterator it = m_map.begin(); it != m_map.end(); it++)
		{
			if ((*it).first == str)
				return (*it).second;
		}
		return NULL;
	}

	void AddObject(const std::string &str,BTObject *p) {
		m_map[str] = p;
	}
	btmap_t& GetMap() { return m_map; }

private:
	btmap_t m_map;
};


#endif _BTDICTIONARY_H

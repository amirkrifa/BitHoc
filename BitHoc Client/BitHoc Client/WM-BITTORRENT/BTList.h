


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
#ifndef _BTLIST_H
#define _BTLIST_H

#include "BTObject.h"


//! list in metainfo file
class BTList : public BTObject
{
public:
	BTList(size_t i) : BTObject(i) {}
	~BTList() {
		for (btobject_v::iterator it = m_objects.begin(); it != m_objects.end(); it++)
		{
			delete (*it);
		}
	}
	char Type() { return 'l'; }
	void Show() {
		BTObject *p = GetParent();
		printf("\n");
		while (p)
		{
			printf("  ");
			p = p -> GetParent();
		}
		printf("[ ");
		for (btobject_v::iterator it = m_objects.begin(); it != m_objects.end(); it++)
			(*it) -> Show();
		printf("] ");
	}
	size_t GetSize() {
		return m_objects.size();
	}
	BTObject *GetFirst() {
		btobject_v::iterator it = m_objects.begin();
		return (*it);
	}

	void AddObject(BTObject *p) {
		m_objects.push_back(p);
	}
	btobject_v& GetList() { return m_objects; }

private:
	btobject_v m_objects;
};


#endif _BTLIST_H

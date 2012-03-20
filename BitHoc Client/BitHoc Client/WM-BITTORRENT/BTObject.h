


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
#ifndef _BTOBJECT_H
#define _BTOBJECT_H
using namespace std;
#include <string>
#include <vector>


//! Base class for an object in a bencoded string.
class BTObject
{
public:
	BTObject(size_t i) : m_parent(NULL),m_index(i),m_len(0) {}
	virtual ~BTObject() {}

	virtual char Type() = 0;
	virtual void Show() {
	}

	virtual void AddObject(BTObject *) {}
	virtual void AddObject(const std::string &,BTObject *) {}

	BTObject *GetParent() {
		return m_parent;
	}
	void SetParent(BTObject *p) {
		m_parent = p;
	}
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
	void Increase() {
		m_len++;
		if (m_parent)
			m_parent -> Increase();
	}
	size_t GetLength() { return m_len; }
	size_t GetIndex() { return m_index; }

private:
	BTObject *m_parent;
	size_t m_index;
	size_t m_len;
};
typedef std::vector<BTObject *> btobject_v;


#endif _BTOBJECT_H


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

#ifndef _BTSTRING_H
#define _BTSTRING_H

#include "BTObject.h"


//! string object in metainfo file

class BTString : public BTObject
{
public:
	BTString() : BTObject(0) {}
	~BTString() {}
	char Type() { return 's'; }
	void Show() {
		BTObject::Show();
		printf("'%s' ",printable(GetValue()).c_str());
	}

	std::string GetValue() {
		return m_value;
	}
	void SetValue(const std::string &str) {
		m_value = str;
	}

private:
	std::string m_value;
};


#endif  _BTSTRING_H



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

#ifndef _BTINTEGER_H
#define _BTINTEGER_H

#ifdef _WIN32
#pragma warning(disable:4786)
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif
#include "BTObject.h"


//! Integer value in metainfo file

class BTInteger : public BTObject
{
public:
	BTInteger() : BTObject(0),m_val(0) {}
	~BTInteger() {}
	char Type() { return 'i'; }
	void Show() {
		BTObject::Show();
		printf("%s ",GetValue().c_str());
	}

	std::string GetValue() {
		return m_value;
	}
	void SetValue(const std::string &str) {
		m_value = str;
		m_val = 0;
		for (size_t i = 0; i < str.size(); i++)
		{
			m_val = m_val * 10 + (str[i] - 48);
		}
	}
	int64_t GetVal() { return m_val; }

private:
	std::string m_value;
	int64_t m_val;
};


#endif _BTINTEGER_H

/*
Copyright (C) 2005  Anders Hedstrom

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _BENCODE_H
#define _BENCODE_H
#pragma once

typedef enum {
	BE_STR,
	BE_INT,
	BE_LIST,
	BE_DICT,
} be_type;

struct be_dict;
struct be_node;

/*
 * XXX: the "val" field of be_dict and be_node can be confusing ...
 */

typedef struct be_dict {
	char *key;
	struct be_node *val;
} be_dict;

typedef struct be_node {
	be_type type;
	union {
		char *s;
		long long i;
		struct be_node **l;
		struct be_dict *d;
	} val;
} be_node;



class Bencode
{
public:
	Bencode(void);
	be_node *be_decode(const char *bencode);
	void be_free(be_node *node);
	void be_dump(be_node *node);

public:
	~Bencode(void);
};




#endif
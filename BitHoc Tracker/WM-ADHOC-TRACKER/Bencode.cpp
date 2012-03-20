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
#include "StdAfx.h"
#include "Bencode.h"
#include <stdlib.h> /* malloc() realloc() free() strtoll() */
#include <string.h> /* memset() */

Bencode::Bencode(void)
{
}


Bencode::~Bencode(void)
{
}


static be_node *be_alloc(be_type type)
{
	be_node *ret = (be_node *)malloc(sizeof(*ret));
	if (ret) {
		memset(ret, 0x00, sizeof(*ret));
		ret->type = type;
	}
	return ret;
}

static long long _be_decode_int(const char **data)
{
	char *endp;
	long long ret = strtoll(*data, &endp, 10);
	*data = endp;
	return ret;
}

static char *_be_decode_str(const char **data)
{
	/* XXX: should check for overflow/negative ... */
	long slen = _be_decode_int(data);
	char *ret = NULL;
	if (**data == ':') {
	 	ret = (char *)malloc(slen + 1);
		memcpy(ret, *data + 1, slen);
		ret[slen] = '\0';
		*data += slen + 1;
	}
	return ret;
}

static be_node *_be_decode(const char **data)
{
	be_node *ret = NULL;

	switch (**data) {
		/* lists */
		case 'l': {
			unsigned int i = 0;

			ret = be_alloc(BE_LIST);

			++(*data);
			while (**data != 'e') {
				ret->val.l = (be_node **)realloc(ret->val.l, (i + 2) * sizeof(*ret->val.l));
				ret->val.l[i] = _be_decode(data);
				++i;
			}
			++(*data);

			ret->val.l[i] = NULL;

			return ret;
		}

		/* dictionaries */
		case 'd': {
			unsigned int i = 0;

			ret = be_alloc(BE_DICT);

			++(*data);
			while (**data != 'e') {
				ret->val.d = (be_dict *)realloc(ret->val.d, (i + 2) * sizeof(*ret->val.d));
				ret->val.d[i].key = _be_decode_str(data);
				ret->val.d[i].val = _be_decode(data);
				++i;
			}
			++(*data);

			ret->val.d[i].val = NULL;

			return ret;
		}

		/* integers */
		case 'i': {
			ret = be_alloc(BE_INT);

			++(*data);
			ret->val.i = _be_decode_int(data);
			if (**data != 'e')
				return NULL;
			++(*data);

			return ret;
		}

		/* byte strings */
		case '0'...'9': {
			ret = be_alloc(BE_STR);

			ret->val.s = _be_decode_str(data);

			return ret;
		}

		/* invalid */
		default:
			break;
	}

	return ret;
}

be_node Bencode::*be_decode(const char *data)
{
	return _be_decode(&data);
}

void Bencode::be_free(be_node *node)
{
	switch (node->type) {
		case BE_STR:
			free(node->val.s);
			break;

		case BE_INT:
			break;

		case BE_LIST: {
			unsigned int i;
			for (i = 0; node->val.l[i]; ++i)
				be_free(node->val.l[i]);
			free(node->val.l);
			break;
		}

		case BE_DICT: {
			unsigned int i;
			for (i = 0; node->val.d[i].val; ++i) {
				free(node->val.d[i].key);
				be_free(node->val.d[i].val);
			}
			free(node->val.d);
			break;
		}
	}
	free(node);
}

#ifdef BE_DEBUG
#include <stdio.h>
#include <stdint.h>

static void _be_dump_indent(ssize_t indent)
{
	while (indent-- > 0)
		printf("    ");
}
static void _be_dump(be_node *node, ssize_t indent)
{
	size_t i;

	_be_dump_indent(indent);
	indent = abs(indent);

	switch (node->type) {
		case BE_STR:
			printf("str = %s\n", node->val.s);
			break;

		case BE_INT:
			printf("int = %lli\n", node->val.i);
			break;

		case BE_LIST:
			puts("list [");

			for (i = 0; node->val.l[i]; ++i)
				_be_dump(node->val.l[i], indent + 1);

			_be_dump_indent(indent);
			puts("]");
			break;

		case BE_DICT:
			puts("dict {");

			for (i = 0; node->val.d[i].val; ++i) {
				_be_dump_indent(indent + 1);
				printf("%s => ", node->val.d[i].key);
				_be_dump(node->val.d[i].val, -(indent + 1));
			}

			_be_dump_indent(indent);
			puts("}");
			break;
	}
}
void Bencode::be_dump(be_node *node)
{
	_be_dump(node, 0);
}
#endif
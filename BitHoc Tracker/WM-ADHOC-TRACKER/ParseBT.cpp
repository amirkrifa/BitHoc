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
#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ParseBT.h"

/* implementation of class ParseBT */

ParseBT::ParseBT()
{
	pa_the_str = NULL;
	pa_splits = NULL;
	pa_ord = NULL;
	pa_the_ptr = 0;
	pa_breakchar = 0;
	pa_enable = pa_disable = 0;
	pa_nospace = 0;
	pa_quote = false;
}

ParseBT::ParseBT(char *s)
{
	pa_the_str = NULL;
	pa_splits = NULL;
	pa_ord = NULL;
	pa_the_ptr = 0;
	pa_breakchar = 0;
	pa_enable = pa_disable = 0;
	pa_nospace = 0;
	pa_quote = false;

	pa_the_str = new char[strlen(s) + 1];
	strcpy(pa_the_str,s);
	pa_ord = new char[strlen(s) + 1];
	*pa_ord = 0;
}

ParseBT::ParseBT(char *s,char *sp)
{
	pa_the_str = NULL;
	pa_splits = NULL;
	pa_ord = NULL;
	pa_the_ptr = 0;
	pa_breakchar = 0;
	pa_enable = pa_disable = 0;
	pa_nospace = 0;
	pa_quote = false;

	pa_the_str = new char[strlen(s) + 1];
	strcpy(pa_the_str,s);
	pa_splits = new char[strlen(sp) + 1];
	strcpy(pa_splits,sp);
	pa_ord = new char[strlen(s) + 1];
	*pa_ord = 0;
}

ParseBT::ParseBT(char *s,char *sp,short nospace)
{
	pa_the_str = NULL;
	pa_splits = NULL;
	pa_ord = NULL;
	pa_the_ptr = 0;
	pa_breakchar = 0;
	pa_enable = pa_disable = 0;
	pa_nospace = 0;
	pa_quote = false;

	pa_the_str = new char[strlen(s) + 1];
	strcpy(pa_the_str,s);
	pa_splits = new char[strlen(sp) + 1];
	strcpy(pa_splits,sp);
	pa_ord = new char[strlen(s) + 1];
	*pa_ord = 0;
	pa_nospace = 1;
}

#define FREE(x) if (x) delete x;

ParseBT::~ParseBT()
{
	FREE(pa_the_str);
	FREE(pa_splits);
	FREE(pa_ord);
}

#define C pa_the_str[pa_the_ptr]

short ParseBT::issplit(char c)
{
	short i;

	if (pa_splits)
		for (i = 0; (size_t)i < strlen(pa_splits); i++)
			if (pa_splits[i] == c)
				return 1;
	return 0;
}

void ParseBT::getsplit(void)
{
	int x;

	if (C == '=')
	{
		x = pa_the_ptr++;
	} else
	{
		while (C && (issplit(C)))
			pa_the_ptr++;
		x = pa_the_ptr;
		while (C && !issplit(C) && C != '=')
			pa_the_ptr++;
	}
	if (x == pa_the_ptr && C == '=')
		pa_the_ptr++;
	strncpy(pa_ord,pa_the_str + x,pa_the_ptr - x);
	pa_ord[pa_the_ptr - x] = 0;
}

void ParseBT::getword(void)
{
	int x;
	int disabled = 0;
	int quote = 0;

	if (pa_nospace)
	{
		while (C && issplit(C))
			pa_the_ptr++;
		x = pa_the_ptr;
		while (C && !issplit(C) && (C != pa_breakchar || !pa_breakchar || disabled))
		{
			if (pa_breakchar && C == pa_disable)
				disabled = 1;
			if (pa_breakchar && C == pa_enable)
				disabled = 0;
			if (pa_quote && C == '"')
				quote = 1;
			pa_the_ptr++;
			while (quote && C && C != '"')
			{
				pa_the_ptr++;
			}
			if (pa_quote && C == '"')
			{
				pa_the_ptr++;
			}
			quote = 0;
		}
	} else
	{
		if (C == pa_breakchar && pa_breakchar)
		{
			x = pa_the_ptr++;
		} else
		{
			while (C && (C == ' ' || C == 9 || C == 13 || C == 10 || issplit(C)))
				pa_the_ptr++;
			x = pa_the_ptr;
#define DEB(x) 

			while (C && C != ' ' && C != 9 && C != 13 && C != 10 && !issplit(C) &&
			 (C != pa_breakchar || !pa_breakchar || disabled))
			{

				if (pa_breakchar && C == pa_disable)
					disabled = 1;
				if (pa_breakchar && C == pa_enable)
					disabled = 0;
				if (pa_quote && C == '"')
				{
					quote = 1;
				pa_the_ptr++;
				while (quote && C && C != '"')
				{
					pa_the_ptr++;
				}
				if (pa_quote && C == '"')
				{
					pa_the_ptr++;
				}
				}
				else
					pa_the_ptr++;
				quote = 0;
			}
		}
		if (x == pa_the_ptr && C == pa_breakchar && pa_breakchar)
			pa_the_ptr++;
	}
	strncpy(pa_ord,pa_the_str + x,pa_the_ptr - x);
	pa_ord[pa_the_ptr - x] = 0;
}

void ParseBT::getword(char *s)
{
	ParseBT::getword();
	strcpy(s,pa_ord);
}

void ParseBT::getword(char *s,int l)
{
	ParseBT::getword();
	strncpy(s,pa_ord,l - 1);
	s[l - 1] = 0;
}

void ParseBT::getsplit(char *s)
{
	ParseBT::getsplit();
	strcpy(s,pa_ord);
}

void ParseBT::getword(char *s,char *fill,int l)
{
	ParseBT::getword();
	*s = 0;
	while (strlen(s) + strlen(pa_ord) < (size_t)l)
		strcat(s,fill);
	strcat(s,pa_ord);
}

void ParseBT::getrest(char *s)
{
	while (C && (C == ' ' || C == 9 || issplit(C)))
		pa_the_ptr++;
	strcpy(s,pa_the_str + pa_the_ptr);
}

long ParseBT::getvalue(void)
{
	ParseBT::getword();
	return atol(pa_ord);
}

void ParseBT::setbreak(char c)
{
	pa_breakchar = c;
}

int ParseBT::getwordlen(void)
{
	int x,y = pa_the_ptr,len;

	if (C == pa_breakchar && pa_breakchar)
	{
		x = pa_the_ptr++;
	} else
	{
		while (C && (C == ' ' || C == 9 || C == 13 || C == 10 || issplit(C)))
			pa_the_ptr++;
		x = pa_the_ptr;
		while (C && C != ' ' && C != 9 && C != 13 && C != 10 && !issplit(C) && (C != pa_breakchar || !pa_breakchar))
			pa_the_ptr++;
	}
	if (x == pa_the_ptr && C == pa_breakchar && pa_breakchar)
		pa_the_ptr++;
	len = pa_the_ptr - x;
	pa_the_ptr = y;
	return len;
}

int ParseBT::getrestlen(void)
{
	int y = pa_the_ptr,len;

	while (C && (C == ' ' || C == 9 || issplit(C)))
		pa_the_ptr++;
	len = strlen(pa_the_str + pa_the_ptr);
	pa_the_ptr = y;
	return len;
}

void ParseBT::getline(void)
{
	int x;

//	while (C && (C == 13 || C == 10))
//		pa_the_ptr++;
	x = pa_the_ptr;
	while (C && C != 13 && C != 10)
		pa_the_ptr++;
	strncpy(pa_ord,pa_the_str + x,pa_the_ptr - x);
	pa_ord[pa_the_ptr - x] = 0;
	if (C == 13)
		pa_the_ptr++;
	if (C == 10)
		pa_the_ptr++;
}

void ParseBT::getline(char *s)
{
	getline();
	strcpy(s,pa_ord);
}

void ParseBT::getline(char *s,int l)
{
	getline();
	strncpy(s,pa_ord,l - 1);
	s[l - 1] = 0;
}

/* end of implementation of class ParseBT */
/***************************************************/

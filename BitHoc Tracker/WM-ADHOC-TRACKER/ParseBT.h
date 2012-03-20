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
#ifndef _PARSE_H
#define _PARSE_H

//! Splits a string whatever way you want

class ParseBT
{
public:
	ParseBT();
	ParseBT(char *);
	ParseBT(char *,char *);
	ParseBT(char *,char *,short);
	~ParseBT();
	short issplit(char);
	void getsplit(void);
	void getsplit(char *);
	void getword(void);
	void getword(char *);
	void getword(char *,int);
	void getword(char *,char *,int);
	void getrest(char *);
	long getvalue(void);
	void setbreak(char);
	int getwordlen(void);
	int getrestlen(void);
	void enablebreak(char c) {
		pa_enable = c;
	}
	void disablebreak(char c) {
		pa_disable = c;
	}
	void getline(void);
	void getline(char *);
	void getline(char *,int);
	int getptr(void) { return pa_the_ptr; }
	void EnableQuote(bool b) { pa_quote = b; }

private:
	char *pa_the_str;
	char *pa_splits;
	char *pa_ord;
	int   pa_the_ptr;
	char  pa_breakchar;
	char  pa_enable;
	char  pa_disable;
	short pa_nospace;
	bool  pa_quote;
};

#endif // _PARSE_H

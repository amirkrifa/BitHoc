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
#ifndef _BITMAP_T_H
#define _BITMAP_T_H

#include <string>


//! 'have' bitmap array

class bitmap_t
{
public:
	bitmap_t(size_t number_of_pieces);
	~bitmap_t();

	unsigned char *GetBitmap() { return m_bitmap; }
	size_t GetBitmapSize() { return m_bitmap_size; }

	void set(size_t piece);
	bool IsSet(size_t piece);

	void Load(const std::string& );
	void Save(const std::string& );

	/** return number of set bits */
	size_t GetSet();
	size_t GetNumberOfPieces() { return m_number_of_pieces; }

private:
	bitmap_t(const bitmap_t& ) {} // copy constructor
	bitmap_t& operator=(const bitmap_t& ) { return *this; } // assignment operator

	unsigned char *m_bitmap;
	size_t m_bitmap_size;
	size_t m_number_of_pieces;
};




#endif // _BITMAP_T_H

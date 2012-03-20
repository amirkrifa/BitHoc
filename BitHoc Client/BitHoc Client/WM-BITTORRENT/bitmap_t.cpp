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

#include <stdio.h>

#include "bitmap_t.h"




bitmap_t::bitmap_t(size_t number_of_pieces)
:m_bitmap(NULL)
,m_bitmap_size(0)
,m_number_of_pieces(number_of_pieces)
{
	m_bitmap_size = number_of_pieces / 8;
	if (number_of_pieces % 8)
		m_bitmap_size++;
	m_bitmap = new unsigned char[m_bitmap_size];
	memset(m_bitmap, 0, m_bitmap_size);
}


bitmap_t::~bitmap_t()
{
	if (m_bitmap)
		delete[] m_bitmap;
}


void bitmap_t::set(size_t piece)
{
	size_t n = piece / 8;
	int bit[] = {128,64,32,16,8,4,2,1};
	m_bitmap[n] |= bit[piece % 8];
}


void bitmap_t::Load(const std::string& filename)
{
	FILE *fil = fopen(filename.c_str(), "rb");
	if (fil)
	{
		fread(m_bitmap, 1, m_bitmap_size, fil);
		fclose(fil);
	}
}


void bitmap_t::Save(const std::string& filename)
{
	FILE *fil = fopen(filename.c_str(), "wb");
	if (fil)
	{
		fwrite(m_bitmap, 1, m_bitmap_size, fil);
		fclose(fil);
	}
}


bool bitmap_t::IsSet(size_t piece)
{
	size_t n = piece / 8;
	int bit[] = {128,64,32,16,8,4,2,1};
	return (m_bitmap[n] & bit[piece % 8]) ? true : false;
}


size_t bitmap_t::GetSet()
{
	size_t q = 0;
	for (size_t i = 0; i < m_number_of_pieces; i++)
		if (IsSet(i))
			q++;
	return q;
}



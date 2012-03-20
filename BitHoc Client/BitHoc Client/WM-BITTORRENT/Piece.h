/*

  Copyright (C) 2008  INRIA, Planète Team

  Authors: 
  ------------------------------------------------------------
  Amir Krifa			:  Amir.Krifa@sophia.inria.fr
  Mohamed Karim Sbai	:  Mohamed_Karim.Sbai@sophia.inria.fr
  Chadi Barakat			:  Chadi.Barakat@sophia.inria.fr
  Thierry Turletti		:  Thierry.Turletty@sophia.inria.fr
  ------------------------------------------------------------

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

/*!
* \file Piece.h
* \brief Information about one piece of the transfer
**/

#include "stdafx.h"
#ifndef _PIECE_H
#define _PIECE_H

#include <map>
#include <vector>
#ifdef _WIN32
#define MIN(a,b) (a<b)?a:b
#endif
extern size_t iSliceSize;


typedef struct slice_length{
	size_t length;
}slice_length;

//class Session;

typedef std::map<size_t,slice_length*> mSlice;
//! Information about one piece of the transfer

class Piece
{
	
public:
	Piece(size_t nr,__int64 piece_length);
	~Piece();

	size_t GetNumber() { return m_nr; }
	//void Write(Session *,size_t offset, size_t length, unsigned char *);
	size_t NumberOfSlices() { return m_piece_length / iSliceSize; }
	size_t PieceLength() { return m_piece_length; }
	size_t SliceSize() { return iSliceSize; }
	
	size_t Complete(size_t offset) { 
		mSlice::iterator sIter=mComplete.find(offset);
		if(sIter== mComplete.end())
			return 0;
		else
			return sIter->second->length; 
	}
	void SetInComplete(size_t offset,size_t length);
	void SetComplete(size_t offset,size_t length);
	size_t Requested(size_t offset) { 
		mSlice::iterator iReq=mRequested.find(offset);
		if(iReq==mRequested.end())
			return 0;
		else
			return iReq->second->length; 
	}
	void SetRequested(size_t offset,size_t length);
	void SetNotRequested(size_t offset,size_t length);

	void ClearComplete();
	void ClearRequested();
	void ClearIncomplete();
	bool GetRandomNotRequested(size_t& offset, size_t& length) {
		std::vector<size_t> available;
		for (size_t i = 0; i < m_piece_length; i += iSliceSize)
		{
			if (!Complete(i) && !Requested(i))
			{
				available.push_back(i);
			}
		}
		size_t q;
		if ((q = available.size()) > 0)
		{
			offset = available[rand() % q];
			length = MIN(iSliceSize, m_piece_length - offset);
			return true;
		}
		return false;
	}
	
	bool GetRandomRequestedButIncomplete(size_t& offset, size_t& length) {
		std::vector<size_t> available;
		for (size_t i = 0; i < m_piece_length; i += iSliceSize)
		{
			if (!Complete(i) && Requested(i))
			{
				available.push_back(i);
			}
		}
		size_t q;
		if ((q = available.size()) > 0)
		{
			offset = available[rand() % q];
			length = MIN(iSliceSize, m_piece_length - offset);
			return true;
		}
		return false;
	}

	bool AllRequested() {
		for (size_t i = 0; i < m_piece_length; i += iSliceSize)
		{
			if (!Complete(i) && !Requested(i))
			{
				return false;
			}
		}
		return true;
	}

	size_t NumberComplete() {
		size_t q = 0;
		for (size_t i = 0; i < m_piece_length; i += iSliceSize)
		{
			if (Complete(i))
				q++;
		}
		return q;
	}
	size_t NumberRequested() {
		size_t q = 0;
		for (size_t i = 0; i < m_piece_length; i += iSliceSize)
		{
			if (Requested(i) && !Complete(i))
				q++;
		}
		return q;
	}
	bool Complete() {
		for (size_t i = 0; i < m_piece_length; i += iSliceSize)
		{
			if (!Complete(i))
			{
				return false;
			}
		}
		
		return true;
	}

	size_t GetCompletedBytes()
	{
		return NumberComplete()*iSliceSize;
	}
	void ReChek();
	void save_slice_m(FILE *,mSlice& );
	void load_slice_m(FILE *,mSlice& );
	mSlice& MapComplete() { return mComplete; }
	mSlice& MapRequested() { return mRequested; }
	size_t GetPieceLength(){return m_piece_length;};

private:
	Piece(const Piece& ) {} // copy constructor
	Piece& operator=(const Piece& ) { return *this; } // assignment operator
	
	size_t m_nr;
	size_t m_piece_length;
	mSlice mRequested;
	mSlice mComplete;
	mSlice mIncomplete;
};




#endif // _PIECE_H

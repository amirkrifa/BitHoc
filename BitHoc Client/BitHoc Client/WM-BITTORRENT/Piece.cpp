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
* \file Piece.cpp
* \brief Information about one piece of the transfer
**/

#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
//#include "Session.h"
#include "Piece.h"




Piece::Piece(size_t nr,__int64 piece_length)
:m_nr(nr)
,m_piece_length(piece_length)
{
}


Piece::~Piece()
{
	this->ClearComplete();
	this->ClearIncomplete();
	this->ClearRequested();
}





void Piece::ClearComplete()
{
	while (!mComplete.empty())
	{
		mSlice::iterator it = mComplete.begin();
		free(it->second);
		mComplete.erase(it);
	}
}

void Piece::ClearIncomplete()
{
	while (!mIncomplete.empty())
	{
		mSlice::iterator it = mIncomplete.begin();
		free(it->second);
		mIncomplete.erase(it);
	}
}


void Piece::ClearRequested()
{
	while (!mRequested.empty())
	{
		mSlice::iterator it = mRequested.begin();
		free(it->second);
		mRequested.erase(it);
	}
}


void Piece::save_slice_m(FILE *fil,mSlice& ref)
{
	size_t q = ref.size();
	fwrite(&q, sizeof(size_t), 1, fil);
	for (mSlice::iterator it = ref.begin(); it != ref.end(); it++)
	{
		size_t offset = it->first;
		size_t length;
		if(it->second!=NULL)
			length = it->second->length;
		else length =0;
		fwrite(&offset, sizeof(size_t), 1, fil);
		fwrite(&length, sizeof(size_t), 1, fil);
	}
}


void Piece::load_slice_m(FILE *fil,mSlice& ref)
{
	size_t q;
	fread(&q, sizeof(size_t), 1, fil);
	while (q--)
	{
		size_t offset, length;
		fread(&offset, sizeof(size_t), 1, fil);
		fread(&length, sizeof(size_t), 1, fil);
		slice_length* tS=(slice_length*)malloc(sizeof(slice_length));
		tS->length=length;
		ref.insert(std::make_pair<size_t,slice_length*>(offset,tS));
		tS=NULL;
	}
}
void Piece::SetComplete(size_t offset,size_t length) 
{ 
	slice_length *pTemp=(slice_length*)malloc(sizeof(slice_length));
	pTemp->length=length;
	mComplete.insert(std::make_pair<size_t,slice_length*>(offset,pTemp));
	pTemp=NULL;
}

void Piece::SetInComplete(size_t offset,size_t length) 
{ 
	slice_length *pTemp=(slice_length*)malloc(sizeof(slice_length));
	pTemp->length = length;
	mIncomplete.insert(std::make_pair<size_t,slice_length*>(offset,pTemp));
	pTemp=NULL;
}

void Piece::SetRequested(size_t offset,size_t length) 
{ 
	mSlice::iterator iReq=mRequested.find(length);
	if(iReq==mRequested.end())
	{
		slice_length* sL=(slice_length*)malloc(sizeof(slice_length));
		sL->length=length;
		mRequested.insert(std::make_pair<size_t,slice_length*>(offset,sL));
		sL=NULL;
	}else{
		if(!length ||length > iReq->second->length)
			iReq->second->length = length; 

	}
}

void Piece::SetNotRequested(size_t offset,size_t length)
{
	mSlice::iterator iReq = mRequested.find(length);
	if(iReq != mRequested.end())
	{
		free(iReq->second);
		mRequested.erase(iReq);
	}
}

void Piece::ReChek()
{

	if(!mRequested.empty() && mIncomplete.empty())
	{
		// Move the requested slice to Incomplete and clean all requested
		mSlice::iterator iReq = mRequested.begin();
		while(iReq != mRequested.end())
		{
			//Insert it into the Incomplete map
			if(!Complete(iReq->second->length))
				mIncomplete.insert(std::make_pair<size_t,slice_length*>(iReq->first,iReq->second));
			// Delete it
			iReq->second = NULL;
			iReq = mRequested.erase(iReq);
			
		}
	}
}

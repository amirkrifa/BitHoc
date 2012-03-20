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
* \file FileManager.h
* \brief Class used to manage the downloaded data file.
**/

#include "stdafx.h"

#ifndef _FILEMANAGER_H
#define _FILEMANAGER_H

#ifdef _WIN32
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#include <vector>
#include <string>


class PeerHandler;
class Session;

//! Manages read/write/verification of files defined in metainfo file

class FileManager
{
public:
	//! File handle + information for one file in metainfo block
	struct FIL {
		FIL(int64_t l,const std::string& p) : length(l),path(p),fil(NULL) {}
		int64_t length;
		std::string path;
		FILE *fil;
	};
	typedef std::vector<FIL *> fil_v;
public:
	FileManager(PeerHandler&,const std::string& hash,bool);
	~FileManager();

	size_t ReadPiece(int64_t index,unsigned char *buf,int64_t length);
	size_t ReadNextPiece(unsigned char *buf,size_t length);
	void Write(size_t index,unsigned char *buf,size_t length,size_t begin);
	PeerHandler& Handler() { return m_handler; }
	void OpenFiles();

	size_t GetFilesize(const std::string& );
	uint64_t GetTotalLength();
	bool Verify(size_t piece,size_t length);
	bool CompareHash(unsigned char *h1,unsigned char *h2);
	bool IsComplete();
	bool IsHere();
private:
	void GetFiles(fil_v& files);
	//
	std::string m_hash;
	PeerHandler& m_handler;
	std::string m_basename;
	fil_v m_files;
	int64_t m_length;
	FILE *m_single_file;
	bool bSeeding;
};




#endif // _FILEMANAGER_H

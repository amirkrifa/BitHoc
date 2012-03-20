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
* \file FileManager.cpp
* \brief Implementation of the functions used to manage the downloaded data file.
**/

#include "stdafx.h"

#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#ifndef _WIN32
#include <unistd.h>
#endif

extern size_t GetFileSizeWin32(const TCHAR *fileName);
#include <Parse.h>
#include <windows.h>
using namespace std;
#include <openssl/sha.h>
#include "PeerHandler.h"
#include "FileManager.h"
#include "Session.h"
#include <sstream>
#include "LogFile.h"

extern bool ACTIVATE_LOG;
extern LPWSTR ConvertLPCSTRToLPWSTR(char* pCstring);
			
FileManager::FileManager(PeerHandler& h,const std::string& hash, bool Seeding)
:m_hash(hash)
,m_handler(h)
,m_single_file(NULL)
,bSeeding(Seeding)
{
	Session *sess = h.GetSession(hash);
	if (sess)
	{	
		m_basename = h.GetTorrentDirectory() + "\\" + sess -> GetInfoHash() + "\\" + sess -> GetName();
		
		GetFiles(m_files);
		m_length = sess -> GetLength();

		OpenFiles();
	}
}


FileManager::~FileManager()
{
	if (m_single_file)
	{
		fclose(m_single_file);
	}
	for (fil_v::iterator it = m_files.begin(); it != m_files.end(); it++)
	{
		FIL *p = *it;
		if (p -> fil)
		{
			fclose(p -> fil);
		}
		delete p;
	}
}




void FileManager::OpenFiles()
{
	if (m_length) // single file
	{
		std::string full = m_basename;
		Handler().mkpath( full );
		m_single_file = fopen(full.c_str(),"r+b");
		if (!m_single_file)
		{
			m_single_file = fopen(full.c_str(),"w+b");
		}
		size_t size = 0;
		size=GetFilesize(full);
		if (size > m_length)
		{
			int test=0;
		}
		return;
	}
	// multi file
	for (fil_v::iterator it = m_files.begin(); it != m_files.end(); it++)
	{
		FIL *p = *it;
		std::string full = p -> path;
		Handler().mkpath( full );
		p -> fil = fopen(full.c_str(),"r+b");
		if (!p -> fil)
		{
			p -> fil = fopen(full.c_str(),"w+b");
		}
		size_t size = GetFilesize(full);
		if (size > p -> length)
		{
		}
	}
}


size_t FileManager::ReadPiece(int64_t index,unsigned char *buf,int64_t length)
{
	Session *sess = Handler().GetSession(m_hash);
	if (!sess)
		return 0;
	int64_t piece_length = sess -> GetPieceLength();
	if (m_length)
	{
		int64_t offset = index * piece_length;
		if (fseek(m_single_file,offset,SEEK_SET))
		{
		}
		size_t n = fread(buf,1,length,m_single_file);
		//buf[0]++;
		return n;
	}
	size_t piecestrcat_length=16;
	int64_t start = index * piecestrcat_length;
	int64_t bufset = 0; // offset in buf
	int64_t left = piece_length;
	int64_t ptr = 0;
	size_t read_length = 0;
	for (fil_v::iterator it = m_files.begin(); it != m_files.end() && left; it++)
	{
		FIL *p = *it;
		// start within this file?
		if (start < ptr + p -> length)
		{
			int64_t offset = start - ptr;
			int64_t len = p -> length - offset;
			if (len > left)
			{
				len = left;
			}
			if (fseek(p -> fil,offset,SEEK_SET))
			{
			}
			read_length += fread(buf + bufset,1,len,p -> fil);
			// update
			start += len;
			bufset += len;
			left -= len;
			if (!left)
				return read_length;
		}
		ptr += p -> length;
	}
	//buf[0]++;
	return read_length;
}


void FileManager::Write(size_t index,unsigned char *buf,size_t length,size_t begin)
{
	Session *sess = Handler().GetSession(m_hash);
	if (!sess)
		return;
	int64_t piece_length = sess -> GetPieceLength();
	if (m_length)
	{
		int64_t offset = index * piece_length + begin;
		if (fseek(m_single_file,offset,SEEK_SET))
		{
		}
		long pos = ftell(m_single_file);
		if (pos < offset)
		{
			int64_t diff = offset - pos;
			char *tmp = new char[diff];
			memset(tmp,0,diff);
			fwrite(tmp,1,diff,m_single_file);
			delete tmp;
		}
		fwrite(buf,1,length,m_single_file);
		fflush(m_single_file);
		return;
	}
	int64_t start = index * piece_length + begin;
	int64_t bufset = 0; // offset in buf
	int64_t left = length;
	int64_t ptr = 0;
	for (fil_v::iterator it = m_files.begin(); it != m_files.end() && left; it++)
	{
		FIL *p = *it;
		// start within this file?
		if (start < ptr + p -> length)
		{
			int64_t offset = start - ptr;
			int64_t len = p -> length - offset; // length of file remaining from start of this write
			if (len > left)
			{
				len = left;
			}
			if (fseek(p -> fil,offset,SEEK_SET))
			{
			}

			long pos = ftell(p -> fil);
			if (pos < offset)
			{
				int64_t diff = offset - pos;
				char *tmp = new char[diff];
				memset(tmp,0,diff);
				fwrite(tmp,1,diff,p -> fil);
				delete tmp;
			}

			if (fwrite(buf + bufset,1,len,p -> fil) != len)
			{
			}
			fflush(p -> fil);
			// update
			start += len;
			bufset += len;
			left -= len;
		}
		ptr += p -> length;
	}
}


size_t FileManager::GetFilesize(const std::string& path)
{
	// Modified by Amir Krifa
	return GetFileSizeWin32(ConvertLPCSTRToLPWSTR((char*)path.c_str()));
}


uint64_t FileManager::GetTotalLength()
{
	uint64_t l = 0;
	for (fil_v::iterator it = m_files.begin(); it != m_files.end(); it++)
	{
		FIL *p = *it;
		l += p -> length;
	}
	return l;
}


void FileManager::GetFiles(FileManager::fil_v& files)
{
	
	Session *sess = Handler().GetSession(m_hash);
	if (!sess)
		return;
	file_v& ref = sess -> GetFiles();
	for (file_v::iterator it = ref.begin(); it != ref.end(); it++)
	{
		file_t *f = *it; // name, offset, length
		std::string full = Handler().GetTorrentDirectory() + "\\" + sess -> GetInfoHash() + "\\" + f -> name;
		
		FIL *p2 = new FIL(f -> length, full);
		files.push_back(p2);
	}
}


bool FileManager::Verify(size_t index, size_t length)
{
	Session *sess = Handler().GetSession(m_hash);
	char szIndex[50];
	
	if (!sess)
	{
		return false;
	}
	Piece *ptr = sess -> GetIncomplete(index);
	if (!ptr)
	{
		return false;
	}
	unsigned char *p = new unsigned char[length];
	unsigned char hash[20];
	
	size_t read_length;
	read_length = ReadPiece(index, p, length );
	if (read_length != length)
	{
		delete[] p;
		return false;
	}
	sprintf(szIndex,"%u",index);
	
	SHA1( p, read_length, hash );
	delete[] p;
	std::string test(sess -> GetPieces().substr(index * 20,20).c_str());
	if (CompareHash(hash,(unsigned char *)sess -> GetPieces().substr(index * 20,20).c_str()))
	{
		if(ACTIVATE_LOG)
			sess->session_log->add_line(std::string("CompareHash Ok - Piece "+string(szIndex)),true);
	
		return true;
	}

	if(ACTIVATE_LOG)
		sess->session_log->add_line(std::string("CompareHash Error - Piece "+string(szIndex)),true);
	return false;
}


bool FileManager::CompareHash(unsigned char *h1,unsigned char *h2)
{
	for (size_t i = 0; i < 20; i++)
		if (h1[i] != h2[i])
			return false;
	return true;
}


bool FileManager::IsComplete()
{
	if (m_length) // single file
	{
		std::string full = m_basename;
		size_t size=GetFilesize(full);
		if (size == m_length)
		{
			return true;
		}
		return false;
	}
	// multi file
	int test=true;
	for (fil_v::iterator it = m_files.begin(); it != m_files.end(); it++)
	{
		FIL *p = *it;
		size_t size = GetFilesize(p -> path);
		if (size < p -> length)
		{
			test=false;
		}
	}
	if(test)
		return true;
	else return false;
}

bool FileManager::IsHere()
{
	if (m_length) // single file
	{
		std::string full = m_basename;
		FILE * temp=fopen((char *)full.c_str(),"r");
		if (temp)
		{
			fclose(temp);
			return true;
		}
		return false;
	}
	// multi file
	bool test=true;
	for (fil_v::iterator it = m_files.begin(); it != m_files.end(); it++)
	{
		FIL *p = *it;
		FILE *temp=fopen((char *)p -> path.c_str(),"r");
		if (!temp)
		{
			test=false;
			break;
		}else fclose(temp);
	}
	return test;
}
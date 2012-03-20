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


/*!
* \file CreateTorrentFile.cpp
* \brief The imlementation of the Class CreateTorrentFile used to create a metainfo file starting from either a single file or a directory.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#include "StdAfx.h"
#include "CreateTorrentFile.h"
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <openssl/sha.h>	
#include <sstream>

#define PIECE_LENGTH 262144
// http://btfaq.com/serve/cache/56.html , common piece size = 256KB
// Extern Functions
extern LPWSTR ConvertLPCSTRToLPWSTR(char* pCstring);
extern size_t GetFileSizeWin32(const TCHAR *fileName);

CreateTorrentFile::CreateTorrentFile(void):
iPieceLength(PIECE_LENGTH),
iTargetIsDirectory(0),
iNoCreationDate(0),
iPrivate(0)
{
	// Creating The Directory where to save the MetaInfoFile
	//CreateDirectory( ConvertLPCSTRToLPWSTR(".\\WM-MetaInfo-Files"),NULL);
	sMetaInfoFilePath.append(".\\My Documents\\Expeshare\\");

}

CreateTorrentFile::~CreateTorrentFile(void)
{
	// Clearing the files map
	if(!mTorrentFilesMap.empty())
	{
		map<string,FileNode *>::iterator iter=this->mTorrentFilesMap.begin();
		while(iter!=this->mTorrentFilesMap.end())
		{
			// Deleting the FileNode
			free(iter->second);
			iter++;
		}
		this->mTorrentFilesMap.clear();
	}
}

bool CreateTorrentFile::IsDirectory(string & sPath)
{
	// Test if the target file is a directory or a single file
	DWORD res;
	res=GetFileAttributes( ConvertLPCSTRToLPWSTR((char*)sPath.c_str()));
	if(res==FILE_ATTRIBUTE_DIRECTORY)
		return true;
	else return false;
}
int CreateTorrentFile::ListDirectoryFiles(string &sPath)
{
   WIN32_FIND_DATA ffd;
   TCHAR szDir[MAX_PATH];
   size_t length_of_arg;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   DWORD dwError=0;
   

   // Check that the input path plus 2 is not longer than MAX_PATH.

   StringCchLength(ConvertLPCSTRToLPWSTR((char *)sPath.c_str()), MAX_PATH, &length_of_arg);

   if (length_of_arg > (MAX_PATH - 2))
   {
     // Directory path is too long
      return (-1);
   }

   //Target directory is sPath

   // Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

   StringCchCopy(szDir, MAX_PATH, ConvertLPCSTRToLPWSTR((char *)sPath.c_str()));
   TCHAR tBasePath[400];
   wcscpy_s(tBasePath,400,szDir);
   StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

   // Find the first file in the directory.

   hFind = FindFirstFile(szDir, &ffd);

   if (INVALID_HANDLE_VALUE == hFind) 
   {
      ErrorHandler(TEXT("FindFirstFile"));
      return dwError;
   }else 
   {
	
   }
   // List all the files in the directory with some info about them.

   do
   {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        // <DIR> Another Directory ffd.cFileName
	  }
      else
      {
		// Get the absolute File Path
		TCHAR FullFileName[400];
		wcscpy_s(FullFileName,400,(const wchar_t*)tBasePath);
		wcscat_s(FullFileName,400,(const wchar_t*)ffd.cFileName);
		// Adding the file to the Files Map
		FileNode *NextFile=(FileNode*)malloc(sizeof(FileNode));
		wcstombs(NextFile->path,FullFileName,400);
		NextFile->ulSize=GetFileSizeWin32(FullFileName);
		this->lTotalSize+=NextFile->ulSize;
		this->mTorrentFilesMap[string(NextFile->path)]=NextFile;
      }
   }
   while (FindNextFile(hFind, &ffd) != 0);
 
   dwError = GetLastError();
   if (dwError != ERROR_NO_MORE_FILES) 
   {
      ErrorHandler(TEXT("FindFirstFile"));
   }

   FindClose(hFind);
   return dwError;
}

// Verify if the given path consist on just one directory
bool CreateTorrentFile::IsTheFinalDirectory(string &sPath)
{
   WIN32_FIND_DATA ffd;
   TCHAR szDir[MAX_PATH];
   size_t length_of_arg;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   DWORD dwError=0;
   

   // Check that the input path plus 2 is not longer than MAX_PATH.

   StringCchLength(ConvertLPCSTRToLPWSTR((char *)sPath.c_str()), MAX_PATH, &length_of_arg);

   if (length_of_arg > (MAX_PATH - 2))
   {
     // Directory path is too long
      return false;
   }

   //Target directory is sPath

   // Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

   StringCchCopy(szDir, MAX_PATH, ConvertLPCSTRToLPWSTR((char *)sPath.c_str()));
   TCHAR tBasePath[400];
   wcscpy_s(tBasePath,400,szDir);
   StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

   // Find the first file in the directory.

   hFind = FindFirstFile(szDir, &ffd);

   if (INVALID_HANDLE_VALUE == hFind) 
   {
      return false;
   }else 
   {
	
   }
   // List all the files in the directory with some info about them.

   do
   {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
       return false;
      }
      
   }
   while (FindNextFile(hFind, &ffd) != 0);
 
   FindClose(hFind);
   return true;
}

void CreateTorrentFile::ErrorHandler(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

void CreateTorrentFile::WriteMetaInfoFile(FILE * file)
{
	
	// every metainfo file is one big dictonary and the first entry is the announce url 
	fprintf(file, "d8:announce%u:%s",sAnnounceUrl.length(),sAnnounceUrl.c_str());
	
	// Add the Comment ( Optional)
	if (sComment.size()>0)
		fprintf(file, "7:comment%u:%s", sComment.length(),sComment.c_str());

	fprintf(file, "10:created by18:WM-BITTORRENT V1.0");
	
	// Add the creation date (Optional)
	if (!iNoCreationDate)
		fprintf(file, "13:creation datei%ue",
			(unsigned int) time(NULL));

	// Info Section Another Dictionary

	fprintf(file, "4:infod");

	// first entry is either 'length', which specifies the length of a
	  // single file torrent, or a list of files and their respective sizes 
	if (!iTargetIsDirectory)
		fprintf(file, "6:lengthi%lue",this->lTotalSize);
	else
		AddAllDirecoryFiles(file);

	// Name of the torrent, piece length and the hash string 
	fprintf(file, "4:name%u:%s12:piece lengthi%ue6:pieces%u:",
		sTorrentName.length(), sTorrentName.c_str(),
		iPieceLength, this->iNumberPieces*20);
	// Pieces Length iNumberPieces * SHA_DIGEST_LENGTH

	char * szFilesHash=this->MakeHashFromFiles();
	for(int i=0; i<abs(this->iNumberPieces)*20;i++)
		fputc(szFilesHash[i],file);
	free(szFilesHash);
	// Private Flag 
	if (iPrivate)
		fwrite("7:privatei1e",1,strlen("7:privatei1e"),file);
	// End of the info section and the root dictionary 
	fwrite("ee",1,strlen("ee"),file);
}

 // write the file list if the torrent consists of a directory

void CreateTorrentFile::AddAllDirecoryFiles(FILE * file)
{

	char *a, *b;

	fprintf(file, "5:filesl");

	// go through all the files in the map
	map<string,FileNode *>::iterator m_iter=this->mTorrentFilesMap.begin();

	while(m_iter!=this->mTorrentFilesMap.end()) {

		fprintf(file, "d6:lengthi%ue4:pathl",
			 m_iter->second->ulSize);
		
		a = m_iter->second->path;
		int iDirectoryPos=0;
		iDirectoryPos=string(a).find_first_of('\\');
		b =( a+iDirectoryPos);
		while (strlen(b)>0) {
		
			*b = '\0';
			if(strlen(a)>0)
				fprintf(file, "%u:%s", strlen(a), a);
			*b = '\\';
			a = b + 1;
			iDirectoryPos=string(a).find_first_of('\\');
			if(iDirectoryPos!=string::npos)
				b =( a+iDirectoryPos);
			else break;
		}
		fprintf(file, "%u:%see", strlen(a), a);
		m_iter++;
	}
	fprintf(file, "e");
}




 // go through the files in the map, split their contents into pieces
 // of size iPieceLength and create the hash string, which is the
 // concatenation of the (20 byte) SHA1 hash of every piece
 // last piece may be shorter
 
char * CreateTorrentFile::MakeHashFromFiles()
{
	
	char hash_string[SHA_DIGEST_LENGTH];	//the hash string
	char read_buf[PIECE_LENGTH];	// read buffer 
	FILE *fd;			// file descriptor 
	size_t r;		// number of bytes read from file(s) into the read buffer 
	char *sHash;

	sHash=(char *)malloc(this->iNumberPieces*SHA_DIGEST_LENGTH);
	unsigned long long counter = 0;	// number of bytes hashed should match size when done

	
	r = 0;

	map<string,FileNode *>::iterator mFiles_iter=this->mTorrentFilesMap.begin();
	while(mFiles_iter!=this->mTorrentFilesMap.end()) 
	{
		
		if ((fd = fopen(mFiles_iter->second->path, "rb"))) 
		{
		}
		size_t j=0;
		size_t uBytesToRead = this->iPieceLength;
		for(int i=0;i<abs(this->iNumberPieces);i++)
		{
			strcpy(read_buf,"");
			strcpy(hash_string,"");
			// Reading a piece
			if(i==this->iNumberPieces-1)
			{
				// Last Piece
				uBytesToRead=mFiles_iter->second->ulSize-counter;
				j=fread(read_buf,sizeof(unsigned char),uBytesToRead,fd);
				read_buf[uBytesToRead]='\0';
				SHA1((unsigned char *)read_buf,j, (unsigned char *)hash_string);
				
			}else 
			{
				j=fread(read_buf,sizeof(unsigned char),uBytesToRead,fd);
				SHA1((unsigned char *)read_buf, j, (unsigned char *)hash_string);
			}
			for(int k=0;k<20;k++)
			{
				
				sHash[i*20+k]=(unsigned char)(hash_string[k]);
			}
			
			
			counter+=j;
			
		}
		
		
		if (fclose(fd)) {
		}
		mFiles_iter++;
	}



	size_t uFileSize=GetFileSizeWin32(ConvertLPCSTRToLPWSTR((char*)this->sSourceFilePath.c_str()));
	if (counter != uFileSize ) {
	
		MessageBox( NULL,TEXT("Error encountred when creating the metainfo file."),
								    TEXT("Error."), MB_OK );
	}

	return sHash;
}

void CreateTorrentFile::ProcessSourceFile(string &sSourceFilePath, string &sTrackerAddress, string &sTorrentName)
{
	// Init the source file path, the announce URL and the Torrent Name
	this->SetSourceFilePath(sSourceFilePath);
	this->SetAnnounceUrl(sTrackerAddress);
	this->SetTorrentName(sTorrentName);
	
	// Parse the source file path
	this->ParseSourcePath();
	
	// Calculating the number of pieces
	this->CalculateNumberOfPieces();

	FILE *stMetaInfoFile;	// stream for writing to the metainfo file
	stMetaInfoFile=fopen(this->sMetaInfoFilePath.c_str(),"wb");
	
	// calculate hash string and write the metainfo to file 
	WriteMetaInfoFile(stMetaInfoFile);

	// close the file stream
	fclose(stMetaInfoFile);

	std::string message;
	message.assign("The MetaInfo file :");
	message.append(this->sMetaInfoFilePath.c_str());
	message.append(" was Successfully created.");

	MessageBox( NULL,ConvertLPCSTRToLPWSTR((char *)message.c_str()),
		 TEXT(".Torrent File Created"), MB_OK );
	

}

void CreateTorrentFile::CalculateNumberOfPieces()
{
	// calculate the number of pieces
	iNumberPieces = (lTotalSize + iPieceLength - 1) / iPieceLength;
}

void CreateTorrentFile::ParseSourcePath()
{
	if(this->IsDirectory(this->sSourceFilePath))
	{	
		iTargetIsDirectory=1;
		// It is a Directory
		ListDirectoryFiles(sSourceFilePath);
	}else {
		// It is a single File
		iTargetIsDirectory=0;
		this->lTotalSize=GetFileSizeWin32(ConvertLPCSTRToLPWSTR((char*)this->sSourceFilePath.c_str()));
		// Add the file to the map
		FileNode *SingleFile=(FileNode*)malloc(sizeof(FileNode));
		strcpy(SingleFile->path,this->sSourceFilePath.c_str());
		SingleFile->ulSize=lTotalSize;
		this->mTorrentFilesMap[string(this->sSourceFilePath.c_str())]=SingleFile;
	}
}
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
* \file CreateTorrentFile.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/


#ifndef _CREATE_TORRENT_FILE_H
#define _CREATE_TORRENT_FILE_H


#pragma once
#include <string>
#include <map>
using namespace std;

/*!
* \struct FileNode
* \brief Used to save a data file attributes( size in bytes and absolute path), this attributes will be used to create the metainfo file.
*/
typedef struct FileNode
{
	unsigned long ulSize;
	char path[500];
}FileNode;

/*!
* \class CreateTorrentFile
* \brief This class includes the main methods used to create a bittorrent file from the single file or a directory.
*/
class CreateTorrentFile
{
public:
	CreateTorrentFile(void);

	/*!
	* \fn void SetAnnounceUrl(string & sAnnounceUrl)
	* \brief Used the set the announce url that will be included in the generated metainfo file.
	*/	
	void SetAnnounceUrl(string & sAnnounceUrl)
	{
		this->sAnnounceUrl.assign(sAnnounceUrl);
	};
	/*!
	* \fn void SetSourceFilePath(string & sSourceFilePath)
	* \brief Used the set the source file path.
	*/
	void SetSourceFilePath(string & sSourceFilePath)
	{
		this->sSourceFilePath.assign(sSourceFilePath);
	};
	/*!
	* \fn void SetTorrentName(string &sTorrentName)
	* \brief Used the set the metainfo file name.
	*/
	void SetTorrentName(string &sTorrentName)
	{
		this->sTorrentName.assign(sTorrentName);
		// Add the name to the absolute path
		sMetaInfoFilePath.append(this->sTorrentName+".torrent");
	};
	/*!
	* \fn bool IsDirectory(string & sPath)
	* \brief Used to verify if the source path is a directory or not.
	*/
	bool IsDirectory(string & sPath);
	/*!
	* \fn int ListDirectoryFiles(string &sPath)
	* \brief Used to enumurate a given directory files.
	*/	
	int ListDirectoryFiles(string &sPath);
	/*!
	* \fn void ErrorHandler(LPTSTR lpszFunction)
	* \brief Used to handle errors.
	*/
	void ErrorHandler(LPTSTR lpszFunction);
	/*!
	* \fn void WriteMetaInfoFile(FILE * file)
	* \brief Used to generate the metainfo file.
	*/
	void WriteMetaInfoFile(FILE * file);
	/*!
	* \fn void AddAllDirecoryFiles(FILE * file)
	* \brief Used to a given directory files to the metainfo file.
	*/
	void AddAllDirecoryFiles(FILE * file);
	/*!
	* \fn char * MakeHashFromFiles()
	* \brief Returns the pieces hash string.
	*/
	char * MakeHashFromFiles();
	/*!
	* \fn void CalculateNumberOfPieces()
	* \brief Used to calculate the number of pieces.
	*/
	void CalculateNumberOfPieces();
	/*!
	* \fn void ParseSourcePath()
	* \brief Used to parse the source path and to calculate the total files size.
	*/
	void ParseSourcePath();
	/*!
	* \fn void ProcessSourceFile(string &sSourceFilePath, string &sTrackerAddress, string &sTorrentName)
	* \brief Main method that proccess the source path and generates the metainfo file.
	*/
	void ProcessSourceFile(string &sSourceFilePath, string &sTrackerAddress, string &sTorrentName);
	/*!
	* \fn bool IsTheFinalDirectory(string &sPath)
	* \brief Used to verify if the source path is a single directory or not.
	*/
	bool IsTheFinalDirectory(string &sPath);


public:
	~CreateTorrentFile(void);
private:
	//! Piece length.
	size_t iPieceLength;
	//! Announce url. 
	string sAnnounceUrl;
	//! Name of the torrent (name of directory). 
	string sTorrentName;
	//! Absolute path to the metainfo file. 
	string sMetaInfoFilePath;
	//! Optional comment to add to the metainfo. 
	string sComment;
	//! The source file path.
	string sSourceFilePath;
	//! Target is a directory not just a single file. 
	int iTargetIsDirectory;
	//! Don't write the creation date. 
	int iNoCreationDate;
	//! Set the private flag.
	int iPrivate;
	//! Map of source files.
	map<string,FileNode *> mTorrentFilesMap;
	//! The combined size of all files in the torrent.
	size_t lTotalSize ;
	//! Number of pieces. 
	unsigned int iNumberPieces;			
};


#endif _CREATE_TORRENT_FILE_H	

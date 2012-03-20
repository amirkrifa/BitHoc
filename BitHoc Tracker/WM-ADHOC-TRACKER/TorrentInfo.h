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
* \file TorrentInfo.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _TORRENT_INFO_H
#define _TORRENT_INFO_H

#pragma once
#pragma warning(disable:4786)
#include<string>
#include<map>

using namespace std;

/*!
* \class TorrentInfo
* \brief Used to store and manage the informations about a given Torrent. 
*/
class TorrentInfo
{
public:
	
	TorrentInfo(void);
	~TorrentInfo(void);
	/*!
	* \fn void set_tracker_announce_url(std::string &url)
	* \brief Method used to set the tracker announce URL. 
	*/
	inline void set_tracker_announce_url(std::string &url){
		this->tracker_announce_url=url;
	};
	/*!
	* \fn void set_file_length(int length)
	* \brief Method used to set torrent file length. 
	*/
	inline void set_file_length(int length){
		this->file_length=length;
	};
	/*!
	* \fn void set_file_name(std::string &name)
	* \brief Method used to set torrent file name. 
	*/	
	inline void set_file_name(std::string &name){
		this->file_name.assign(name);
	};
	/*!
	* \fn void set_piece_length(int length)
	* \brief Method used to set the piece length. 
	*/		
	inline void set_piece_length(int length){
		this->piece_length=length;
	};
	/*!
	* \fn std::string get_tracker_announce_url()
	* \brief Function used to return the tracker announce url. 
	*/	
	inline std::string get_tracker_announce_url(){return this->tracker_announce_url;};
	/*!
	* \fn int get_file_length()
	* \brief Function used to return the torrent file length. 
	*/
	inline int get_file_length(){return this->file_length;};
	/*!
	* \fn std::string get_file_name()
	* \brief Function used to return the torent file name. 
	*/
	inline std::string get_file_name(){return file_name;};
	/*!
	* \fn int get_piece_length()
	* \brief Function used to return the piece length. 
	*/
	inline int get_piece_length(){return piece_length;};
	/*!
	* \fn std::string get_pieces_hash()
	* \brief Function used to return the file pieces hash. 
	*/
	inline std::string get_pieces_hash(){return pieces_hash;};
	inline void set_info_hash(string ih)
	{
		this->infoHash = ih;
	}
	
	inline string get_info_hash()
	{
		return this->infoHash;
	}
private:
	/*!
	* \brief The tracker announce url. 
	*/
	std::string tracker_announce_url;
	/*!
	* \brief Length of the torrent file in bytes. 
	*/	
	int file_length;
	/*!
	* \brief The name of the file described in the MetaInfo file. 
	*/
	std::string file_name;
	/*!
	* \brief The piece length. 
	*/
	int piece_length;
	
	/*!
	* \brief String consisting of the concatenation of all 20
	* sbyte SHA1 hash values, one per piece.(raw binary encoded). 
	*/
	std::string pieces_hash;

	string infoHash;
	

};


#endif _TORRENT_INFO_H

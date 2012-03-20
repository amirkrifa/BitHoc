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
* \file TorrentManager.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _TORRENT_MANAGER_H
#define _TORRENT_MANAGER_H


#pragma once
#include <map>
#include <string>
#include <list>
using namespace std;
class PeersManager;
class TorrentInfo;
/*! 
* \brief A list used to store peers ids.
*/
typedef std::list<std::string> PeersIds;
/*! 
* \brief A map used to store the lits of peers ids interrested in a given torrent.
*/
typedef std::map<std::string,PeersIds> TorrentPeers;

typedef std::list<int> IntList;
typedef std::map<string, TorrentInfo*> TorrentsDetails;


/*!
* \class TorrentManager
* \brief This class is used to store and update the status of the torrents managed by the tracker.
*/
class TorrentManager
{
public:
	TorrentManager(PeersManager * ptr_pm);
	/*!
	* \fn void add_peer_to_torrent(char * info_hash,string fileName char * peer_id)
	* \brief A method used to add new peer to the list of peers interested in a given torrent.
	*/
	void add_peer_to_torrent(char * info_hash, string fileName, char * peer_id);
	/*!
	* \fn int get_number_of_ppers_with_entire_file(string &info_hash,string &current_id)
	* \brief Function used to get the number of peers that has already downloaded the entire torrent files.
	*/
	int get_number_of_ppers_with_entire_file(string &info_hash,string &current_id);
	/*!
	* \fn int get_number_of_non_seeder_peers(string &info_hash,string &current_id)
	* \brief Function used to get the number of non-seeder peers. 
	*/
	int get_number_of_non_seeder_peers(string &info_hash,string &current_id);
	/*!
	* \fn std::string get_bencoded_fail_message(std::string &message)
	* \brief Function used to translate a failure message to a bencoded message. 
	*/
	std::string get_bencoded_fail_message(std::string &message);
	/*!
	* \fn int check_info_hash(std::string  &info_hash, string &peer_id)
	* \brief Function used to the presence of a given torrent in the torrents list. 
	*/
	int check_info_hash(std::string  &info_hash, string &peer_id);
	/*!
	* \fn string get_list_of_interested_peers(string& info_hash, int numwant,int *,string&)
	* \brief Function used to return the list of interested peers in a given torrent. 
	*/
	string get_list_of_interested_peers(string& info_hash, int numwant,int *,string&);
	string get_html_list_of_interested_peers(string& info_hash, int numwant,int *,string&);
	string get_limited_list_of_interested_peers(string &asking_id,string& info_hash, int numwant,int *);
	/*!
	* \fn string get_torrents_list()
	* \brief Function used to return the list of torrents maintained by the tracker. 
	*/	
	string get_torrents_list();
	
	/*!
	* \fn int get_number_of_torrents()
	* \brief Function used to return the number of torrents maintained by the tracker. 
	*/	
	int get_number_of_torrents()
	{
		
		EnterCriticalSection(&torrent_peers_cs);
		int nt= this->torrent_peers.size();
		LeaveCriticalSection(&torrent_peers_cs);
		return nt;
	};
	/*!
	* \fn void delete_peer_from_torrent(string &torrent_id,string &peer_id)
	* \brief Function used to remove a peer from the list of peers interested in a given torrent. 
	*/	
	void delete_peer_from_torrent(string &torrent_id,string &peer_id);
	/*!
	* \fn bool eliminate_confusion(string &torrent_id, string &peer_id, string &ip)
	* \brief Function used to eliminate confusion while adding a new peer. 
	*/	
	bool eliminate_confusion(string &torrent_id, string &peer_id, string &ip);
	
	/*!
	* \fn void DeleteOutOfDatePeers()
	* \brief Delete Peers Out of Date. 
	*/	
	void DeleteOutOfDatePeers();
	string get_html_torrents_list();
	

	void AddNewTorrentDetails(string fileName, string infoHash);
	void RemoveTorrentDetails();
	string GetTorrentInfoHash(string fileName);
	~TorrentManager(void);

private:
	
	CRITICAL_SECTION torrent_peers_cs;
	/*!
	* \brief Map used to maitain the list of peers interested in a given torrent. 
	*/
	TorrentPeers torrent_peers;
	TorrentsDetails torrentDetailsMap;
	PeersManager * ptr_pm;
};

#endif _TORRENT_MANAGER_H
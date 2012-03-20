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
* \file PeersManager.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _PEERS_MANAGER_H
#define _PEERS_MANAGER_H

#pragma once
#pragma warning(disable:4786)
#include <string>
#include <iostream>
#include <map>
#include "PeerInfo.h"
using namespace std ;
/*!
* \class PeersManager
* \brief Class used by the tracker to manage a set of peers and all related attributes.
*/
class PeersManager
{
public:
	PeersManager(void);
	/*!
	* \fn int add_peer(PeerInfo *peer)
	* \brief Function used to add a peer.
	*/
	int add_peer(PeerInfo *peer);
	/*!
	* \fn PeerInfo* add_peer_2(string &peer_id,string &peer_ip,int peer_port,int nwant)
	* \brief Function used to add a peer and to return a ptr if it is already in the list.
	*/
	PeerInfo* add_peer_2(string &peer_id,string &peer_ip,int peer_port,int nwant,int nbr_hops,int i);
	/*!
	* \fn int get_peer_port(std::string &peer_id)
	* \brief Function that returns a given peer port.
	*/
	int get_peer_port(std::string &peer_id);
	/*!
	* \fn std::string get_peer_ip_adr(std::string &peer_id)
	* \brief Function that returns a given peer ip @.
	*/
	std::string get_peer_ip_adr(std::string &peer_id);
	/*!
	* \fn string get_peer_bencoded_info(string &peer_id,string &info_hash)
	* \brief Function that returns a given peer bencoded status.
	*/
	string get_peer_bencoded_info(string &peer_id,string &info_hash);
	string get_html_peer_bencoded_info(string &peer_id,string &info_hash);
	/*!
	* \fn string get_limited_peer_bencoded_info(string &peer_id,string &info_hash)
	* \brief Function that returns a given peer restricted bencoded status.
	*/
	string get_limited_peer_bencoded_info(string &peer_id,string &info_hash);
	/*!
	* \fn PeerInfo * get_peer_info(std::string &peer_id, string &peer_ip)
	* \brief Function that returns a PeerInfo object given its id and ip @.
	*/
	PeerInfo * get_peer_info(std::string &peer_id, string &peer_ip);
	/*!
	* \fn int get_number_of_known_peers()
	* \brief Function that returns the number of managed peers.
	*/
	inline int get_number_of_known_peers(){return this->peers_map.size();};
	/*!
	* \fn string get_peers_list()
	* \brief Function that returns the peers list as a bencoded message.
	*/
	string get_peers_list();
	/*!
	* \fn bool eliminate_confusion(string &peer_id, string &torrent_id,string &ip, char * to_delete)
	* \brief Function used to eliminate confusion while adding a new peer.
	*/
	bool eliminate_confusion(string &peer_id, string &torrent_id,string &ip, char * to_delete);
	/*!
	* \fn void update_peer(string &p_id,string &ip,int port,int nw)
	* \brief Method used to update a given peer status.
	*/	
	void update_peer(string &p_id,string &ip,int port,int nw);
	/*!
	* \fn void add_torrent_of_interest(string &peer_id,string &info_hash, int uploaded,int downloaded,int left,std::string &ev)
	* \brief Method used to a torrent of interest to a given peer.
	*/	
	void add_torrent_of_interest(string &peer_id,string &info_hash, int uploaded,int downloaded,int left,std::string &ev,string &ip);
	/*!
	* \fn bool is_it_a_seeder(string &peer_id,string &info_hash)
	* \brief Function that returns if a given peer is a seeder or not.
	*/	
	bool is_it_a_seeder(string &peer_id,string &info_hash);
	/*!
	* \fn int get_number_of_peers()
	* \brief Function that returns the number of peers.
	*/
	inline int get_number_of_peers()
	{
		EnterCriticalSection(&peers_map_cs);
		int np=this->peers_map.size();
		LeaveCriticalSection(&peers_map_cs);
		return np;
	};
	/*!
	* \fn bool CheckOutOfDate(string & sPeerId)
	* \brief Check if the peer is out of date in order to delete it.
	*/
	bool CheckOutOfDate(string & sPeerId)
	{
		EnterCriticalSection(&peers_map_cs);
		std::map<string,PeerInfo* >::iterator PeersIter=this->peers_map.find(sPeerId);
		if(PeersIter!=this->peers_map.end())
		{
			if(PeersIter->second->OutOfDate())
			{
				// This peer should be deleted
				delete PeersIter->second;
				this->peers_map.erase(PeersIter);
				LeaveCriticalSection(&peers_map_cs);
				return true;
			}
		}
		LeaveCriticalSection(&peers_map_cs);
		return false;
		
	};
	string get_html_peers_list();
	~PeersManager(void);
	
private:
	/*!
	* \brief Map used to store the status object of each peer.
	*/
    std::map<string,PeerInfo* > peers_map;
	CRITICAL_SECTION peers_map_cs;
};



#endif _PEERS_MANAGER_H

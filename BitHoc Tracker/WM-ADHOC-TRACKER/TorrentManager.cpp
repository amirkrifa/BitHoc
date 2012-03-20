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
* \file TorrentManager.cpp
* \brief Implementation of the class TorrentManager used to store and update the status of the torrents managed by the tracker. 
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "StdAfx.h"
#include "TorrentManager.h"
#include "PeersManager.h"
#include "PeerInfo.h"
#include "TorrentInfo.h"
#include <sstream>

TorrentManager::TorrentManager(PeersManager * ptr_pm)
{
	this->ptr_pm=ptr_pm;
	InitializeCriticalSection(&torrent_peers_cs);
}

TorrentManager::~TorrentManager(void)
{
	if(this->ptr_pm!=NULL)
		this->ptr_pm=NULL;
	if(!this->torrent_peers.empty())
	{
		std::map<std::string,std::list<std::string>>::iterator iter_map=this->torrent_peers.begin();
		while(iter_map!=this->torrent_peers.end())
		{
			iter_map->second.clear();
		}
		this->torrent_peers.clear();
		
	}

	// Clearing the torrent'details map
	if(!this->torrentDetailsMap.empty())
	{
		TorrentsDetails::iterator tmpIter = torrentDetailsMap.begin();
		while(tmpIter != torrentDetailsMap.end())
		{
			delete tmpIter->second;
			tmpIter = torrentDetailsMap.erase(tmpIter);
		}
	}

	DeleteCriticalSection(&torrent_peers_cs);
}


// Add an id of a new peer interested in the torrent

void TorrentManager::add_peer_to_torrent(char * info_hash,string fileName, char * peer_id)
{

	if(strlen(info_hash) > 0 && strlen(peer_id) > 0)
	{
		EnterCriticalSection(&torrent_peers_cs);

		string str_peer_id(peer_id);
		string str_info_hash(info_hash);

		std::map<std::string,std::list<std::string>>::iterator iter_map;

		iter_map=this->torrent_peers.find(str_info_hash);
		if(iter_map == this->torrent_peers.end())
		{ // Adding a new torrent and its associated peer
			PeersIds pids;
			this->torrent_peers.insert(make_pair(str_info_hash,pids));
			this->torrent_peers[str_info_hash].push_back(str_peer_id);
			
			// If it comes from an announce request <-> fileName != NULL then add torrent details
			if(fileName.length() > 0)
				this->AddNewTorrentDetails(fileName, string(info_hash));
		}else 
		{
			// Torrent already exist, adding associated peers
			// Verify first if the peer already exists or not
			bool exists=false;
			std::list<string>::iterator list_iter=iter_map->second.begin();
			while(list_iter!=iter_map->second.end())
			{
				if(strcmp(str_peer_id.c_str(),list_iter->c_str())==0)
				{
					exists=true;
					break;
				}
				list_iter++;
			}
			if(!exists)
				iter_map->second.push_back(str_peer_id);

			// If it comes from an announce request <-> fileName != NULL then add torrent details
			if(fileName.length() > 0)
				this->AddNewTorrentDetails(fileName, string(info_hash));

		}

		LeaveCriticalSection(&torrent_peers_cs);
	}

}

// Return the number of seeders

int TorrentManager::get_number_of_ppers_with_entire_file(string &info_hash, string &current_id)
{	
	DeleteOutOfDatePeers();
	EnterCriticalSection(&torrent_peers_cs);
	
	int number_of_seeders=0;
	// The file id
	string file_id;
	file_id.append(info_hash);
	
	std::map<string,list<string>>::iterator iter;
	iter=this->torrent_peers.find(file_id);

	std::list<string>::iterator list_iter;
	if(iter!=this->torrent_peers.end())
	{
		// File founded, looking for seeders
		list_iter=iter->second.begin();
		while(list_iter!=iter->second.end())
		{
			if(list_iter->compare(current_id)!=0)
			{
				if(this->ptr_pm->is_it_a_seeder(*list_iter,file_id))
						number_of_seeders++;
			}
			list_iter++;
		}
		

	}

	LeaveCriticalSection(&torrent_peers_cs);
	return number_of_seeders;
}

// Return the number of leechers

int TorrentManager::get_number_of_non_seeder_peers(string &info_hash, string &current_id)
{
	DeleteOutOfDatePeers();
	EnterCriticalSection(&torrent_peers_cs);
	
	int number_of_non_seeders=0;
	
	// The file id
	std::string file_id;
	file_id.append(info_hash);
	
	std::map<string,list<string>>::iterator iter;
	iter=this->torrent_peers.find(file_id);
	
	std::list<string>::iterator list_iter;
	
	if(iter!=this->torrent_peers.end())
	{
		// File founded, looking for seeders
		list_iter=iter->second.begin();
		PeerInfo *ptr_pi;
		
		while(list_iter!=iter->second.end())
		{
			if(list_iter->compare(current_id)!=0)
			{
			ptr_pi=this->ptr_pm->get_peer_info(string(list_iter->c_str()),string(""));
			if(ptr_pi!=NULL)
			{
				if(!ptr_pi->am_i_seeder(string(info_hash)))
					number_of_non_seeders++;
			}
			}
			list_iter++;
		}
		

	}

	LeaveCriticalSection(&torrent_peers_cs);
	return number_of_non_seeders;
}

std::string TorrentManager::get_bencoded_fail_message(std::string &message)
{

	// Making the failure reason bencoded message
	string temp;
	temp.append("d");
	temp.append("14:failure reason");
	int ml=message.length();
	char ch_ml[100];
	sprintf(ch_ml,"%i",ml);
	temp.append(ch_ml);
	temp.append(":");
	temp.append(message);
	temp.append("e");
	
	return temp;
}

int TorrentManager::check_info_hash(std::string &info_hash, string &peer_id)
{
	
	EnterCriticalSection(&torrent_peers_cs);
	
	if(info_hash.length()==20)
	{
		std::map<std::string,PeersIds>::iterator iter=this->torrent_peers.begin();
		while(iter!=this->torrent_peers.end())
		{
			if(strcmp(iter->first.c_str(),info_hash.c_str())==0)
			{	// Torrent file founded
				// Looking if there is another peer interested in this torrent apart the one asking for :)
				if(iter->second.size()==1)
				{
					PeersIds::iterator peers_iter=iter->second.begin();
					if((*peers_iter).compare(peer_id)!=0)
					{
						LeaveCriticalSection(&torrent_peers_cs);
						return 1;
					}
				}else if( iter->second.size()>1){ 
					// There is more then one peer interested in this torrent
					LeaveCriticalSection(&torrent_peers_cs);
					return 1;
				}
				
			}
			iter++;
		}
		// We dont found the torrent file
		LeaveCriticalSection(&torrent_peers_cs);
		return 0;
	
	} 
	else 
		{
			// Incorrect info_hash size
			LeaveCriticalSection(&torrent_peers_cs);
			return -1;
		}
	LeaveCriticalSection(&torrent_peers_cs);
}
string TorrentManager::get_html_list_of_interested_peers(string& info_hash, int numwant,int * number_of_interested,string &demander_ip)
{
	DeleteOutOfDatePeers();
	EnterCriticalSection(&torrent_peers_cs);
	
	*number_of_interested=0;
	string list_ip;
	int i=0;

	TorrentPeers::iterator iter=this->torrent_peers.find(info_hash);
	int max_peers=50;
	if(numwant>0) max_peers=numwant;
	if(iter!=this->torrent_peers.end())
	{	
		PeersIds::iterator peers_iter=iter->second.begin();
		while(peers_iter!=iter->second.end() && i<=max_peers)
		{	
			if(this->ptr_pm->get_peer_ip_adr(*peers_iter).compare(demander_ip)!=0)
			{
				list_ip.append(this->ptr_pm->get_html_peer_bencoded_info(*peers_iter,info_hash));
				(*number_of_interested)++;
			}
			i++;
			peers_iter++;
		}

	}

	LeaveCriticalSection(&torrent_peers_cs);
	return list_ip;
}
string TorrentManager::get_list_of_interested_peers(string& info_hash, int numwant,int * number_of_interested,string &demander_ip)
{
	DeleteOutOfDatePeers();
	EnterCriticalSection(&torrent_peers_cs);
	
	*number_of_interested=0;
	string list_ip;
	int i=0;
	list_ip.append("d");
	list_ip.append("5:peers");
	list_ip.append("l");
	TorrentPeers::iterator iter=this->torrent_peers.find(info_hash);
	int max_peers=50;
	if(numwant>0) max_peers=numwant;
	if(iter!=this->torrent_peers.end())
	{	
		PeersIds::iterator peers_iter=iter->second.begin();
		while(peers_iter!=iter->second.end() && i<=max_peers)
		{	
			if(this->ptr_pm->get_peer_ip_adr(*peers_iter).compare(demander_ip)!=0)
			{
				list_ip.append(this->ptr_pm->get_peer_bencoded_info(*peers_iter,info_hash));
				(*number_of_interested)++;
			}
			i++;
			peers_iter++;
		}

	}
	list_ip.append("e");
	list_ip.append("e");
	LeaveCriticalSection(&torrent_peers_cs);
	return list_ip;
}
string TorrentManager::get_limited_list_of_interested_peers(string &asking_ip,string& info_hash, int numwant,int * number_of_interested)
{
	DeleteOutOfDatePeers();
	EnterCriticalSection(&torrent_peers_cs);
	
	*number_of_interested=0;
	string list_ip;
	int i=0;
	
	list_ip.append("5:peers");
	list_ip.append("l");
	TorrentPeers::iterator iter=this->torrent_peers.find(info_hash);
	int max_peers=50;
	if(numwant>0) max_peers=numwant;
	if(iter!=this->torrent_peers.end())
	{
		PeersIds::iterator peers_iter=iter->second.begin();
		while(peers_iter!=iter->second.end() && i<=max_peers)
		{	
			if(this->ptr_pm->get_peer_ip_adr(*peers_iter).compare(asking_ip)!=NULL)
			{
				list_ip.append(this->ptr_pm->get_limited_peer_bencoded_info(*peers_iter,info_hash));
				(*number_of_interested)++;
				i++;
			}
			peers_iter++;
		}

	}
	list_ip.append("e");
	
	LeaveCriticalSection(&torrent_peers_cs);
	return list_ip;
}
string TorrentManager::get_torrents_list()
{
	DeleteOutOfDatePeers();
	string tl;
	EnterCriticalSection(&torrent_peers_cs);
	
	TorrentPeers::iterator iter=this->torrent_peers.begin();
	while(iter!=this->torrent_peers.end())
	{
		tl.append("Torrent info_hash: ");
		tl.append(iter->first);
		tl.append("\r\n");
		iter++;
	}
	LeaveCriticalSection(&torrent_peers_cs);
	return tl;
}

string TorrentManager::get_html_torrents_list()
{
	DeleteOutOfDatePeers();
	string tl;
	EnterCriticalSection(&torrent_peers_cs);
	
	TorrentPeers::iterator iter=this->torrent_peers.begin();
	while(iter!=this->torrent_peers.end())
	{
		tl.append("<p>");
		tl.append("Torrent info_hash: ");
		tl.append(iter->first);
		tl.append("</p>");
		int iNumberOfInterestedPeers=0;
		string sListOfInterestedPeers=this->get_html_list_of_interested_peers(string(iter->first),100,&iNumberOfInterestedPeers,string(""));
		std::ostringstream oss;
		oss<<iNumberOfInterestedPeers;
		tl.append("<p>");
		tl.append(oss.str()+" interested peers:");
		tl.append(sListOfInterestedPeers);
		tl.append("</p>");
		iter++;
	}
	LeaveCriticalSection(&torrent_peers_cs);
	return tl;
}
void TorrentManager::delete_peer_from_torrent(string &torrent_id,string &peer_id)
{
	EnterCriticalSection(&torrent_peers_cs);
	TorrentPeers::iterator iter=this->torrent_peers.find(torrent_id);
	if(iter!=this->torrent_peers.end())
	{	// We found the torrent
		PeersIds::iterator peers_iter=iter->second.begin();
		while(peers_iter!=iter->second.end())
		{
			if((*peers_iter).compare(peer_id)==0)
			{
				// dropping the peer 
				peers_iter=iter->second.erase(peers_iter);
				break;
			}
			peers_iter++;
		}
	}

	LeaveCriticalSection(&torrent_peers_cs);
}

bool TorrentManager::eliminate_confusion(string &torrent_id, string &peer_id, string &ip)
{
	
	char pid[30];
	strcpy(pid,"");
	
	if(this->ptr_pm->eliminate_confusion(peer_id,torrent_id,ip,pid)&&  strlen(pid)>=20)
	{
		// We have a peer to delete
		delete_peer_from_torrent(torrent_id,string(pid));
		return true;
	}
	return false;
 
}

void TorrentManager::DeleteOutOfDatePeers()
{
	/*
	EnterCriticalSection(&torrent_peers_cs);
	if(this->torrent_peers.size()>0)
	{
		TorrentPeers::iterator iter=this->torrent_peers.begin();
		PeersIds::iterator PeersIter;
		int tmp=0;
		while(iter!=torrent_peers.end())
		{
			if(iter->second.size()>0)
			{
				PeersIter=iter->second.begin();
				tmp=iter->second.size();
				while(PeersIter!=iter->second.end())
				{
					if(this->ptr_pm->CheckOutOfDate(*PeersIter))
					{
						// This peer should be deleted
						PeersIter=iter->second.erase(PeersIter);
						continue;
					}
					if(iter->second.size()==0)
						break;

					PeersIter++;					
				}
				// if the number of peers interested in the torrent == 0 --> delete the torrent
				if(iter->second.size()==0 && tmp>0)
				{
					// delete the torrent itself
					torrent_peers.erase(iter);
					if(torrent_peers.size()==0)
						break;
				}
				
			}
			iter++;
		}
	}
	LeaveCriticalSection(&torrent_peers_cs);
	*/
	
}


// Not a thread safe method
void TorrentManager::AddNewTorrentDetails(string fileName, string infoHash)
{
	// Verify that the entry does not already exist
	if(torrentDetailsMap.find(fileName) == torrentDetailsMap.end() || torrentDetailsMap.size() == 0)
	{
		TorrentInfo * ti = new TorrentInfo();
		ti->set_file_name(fileName);
		ti->set_info_hash(infoHash);
		torrentDetailsMap.insert(make_pair<string, TorrentInfo*>(fileName, ti));
		ti = NULL;
	}

}
// Not a thread safe method
string TorrentManager::GetTorrentInfoHash(string fileName)
{
	TorrentsDetails::iterator tmpIter;
	if((tmpIter = torrentDetailsMap.find(fileName)) != torrentDetailsMap.end())
	{
		return tmpIter->second->get_info_hash();
	}else return string("");
	
}
// Not a thread safe method
void TorrentManager::RemoveTorrentDetails()
{
	
}

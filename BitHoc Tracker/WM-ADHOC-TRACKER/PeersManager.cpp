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
* \file PeersManager.cpp
* \brief Implementation of the class PeersManager used to store and update the list of peers managed by the tracker. 
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "StdAfx.h"
#include "PeersManager.h"
#include "TorrentManager.h"


PeersManager::PeersManager(void)
{
	InitializeCriticalSection(&peers_map_cs);
}

PeersManager::~PeersManager(void)
{
	DeleteCriticalSection(&peers_map_cs);
	// Deleting the peer map
	if(!this->peers_map.empty())
	{
		std::map<std::string,PeerInfo *>::iterator iter=this->peers_map.begin();
		while(iter!=this->peers_map.end())
		{
			if(iter->second!=NULL)
				delete iter->second;
			iter++;
		}
	}
}

int PeersManager::add_peer(PeerInfo *peer)
{
	EnterCriticalSection(&peers_map_cs);
	map<string, PeerInfo*>::iterator iter=this->peers_map.find(peer->get_peer_id());
	if(iter==this->peers_map.end())
		this->peers_map.insert(make_pair<std::string,PeerInfo*>(string(peer->get_peer_id().c_str()),peer));
	
	LeaveCriticalSection(&peers_map_cs);
	return 1;
}
PeerInfo* PeersManager::add_peer_2(string &peer_id,string &peer_ip,int peer_port,int nwant,int nbr_hops,int in)
{
	if(peer_id.empty() || peer_ip.empty() || peer_port == 0 )
		return NULL;

	EnterCriticalSection(&peers_map_cs);
	
	map<string, PeerInfo*>::iterator iter=this->peers_map.find(peer_id);
	PeerInfo *pr;
	if(iter==this->peers_map.end())
	{
		PeerInfo *pi=new PeerInfo();
		pi->set_peer_port(peer_port);
		pi->set_peer_ip_adr(peer_ip);
		pi->set_peer_id(peer_id);
		pi->SetNumberOfHops(nbr_hops);
		pi->SetTrackerClientPollInterval(in);
		if(nwant==0)
			pi->set_number_of_wanted_peers(50);
		else pi->set_number_of_wanted_peers(nwant);
		this->peers_map.insert(make_pair<std::string,PeerInfo*>(peer_id,pi));
		pr=pi;
		pi=NULL;
	}else pr=this->peers_map[peer_id];

	LeaveCriticalSection(&peers_map_cs);

    return pr; 

}
	
// Looking for a peer port from the peer map

int PeersManager::get_peer_port(std::string &peer_id)
{
	EnterCriticalSection(&peers_map_cs);

	// Looking for a peer in the map
	std::map<std::string,PeerInfo*>::iterator iter;
	iter=this->peers_map.find(peer_id);
	if(iter==peers_map.end())
	{ // we don't findthe peer
		LeaveCriticalSection(&peers_map_cs);
		return -1;
	}else{
		PeerInfo *pi=this->peers_map[peer_id];
		int port=pi->get_peer_port();
		LeaveCriticalSection(&peers_map_cs);
		return port;
		
	}

	LeaveCriticalSection(&peers_map_cs);

}

// Looking for a peer ip address from the peer map

std::string PeersManager::get_peer_ip_adr(std::string &peer_id)
{
	EnterCriticalSection(&peers_map_cs);

	// Looking for a peer in the map
	std::map<std::string,PeerInfo*>::iterator iter=this->peers_map.begin();
	while(iter!=this->peers_map.end())
	{
		if(iter->second->get_peer_id().compare(peer_id)==0)
		{
			string ip(iter->second->get_peer_ip_adr());
			LeaveCriticalSection(&peers_map_cs);
			return ip;
		}
		iter++;
	}
	LeaveCriticalSection(&peers_map_cs);
	return string("");

}
string PeersManager::get_html_peer_bencoded_info(string & peer_id,string &info_hash)
{
	EnterCriticalSection(&peers_map_cs);

	string peer_info;
	std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.find(peer_id);
	if(iter!=this->peers_map.end())
	{
		peer_info.assign(iter->second->get_html_bencoded_info(info_hash));
	}

	LeaveCriticalSection(&peers_map_cs);
	return peer_info;
}
string PeersManager::get_peer_bencoded_info(string &peer_id,string &info_hash)
{
	EnterCriticalSection(&peers_map_cs);

	string peer_info;
	std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.find(peer_id);
	if(iter!=this->peers_map.end())
	{
		peer_info.assign(iter->second->get_bencoded_info(info_hash));
	}

	LeaveCriticalSection(&peers_map_cs);
	return peer_info;
}

string PeersManager::get_limited_peer_bencoded_info(string &peer_id,string &info_hash)
{
	EnterCriticalSection(&peers_map_cs);

	string peer_info;
	std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.find(peer_id);
	if(iter!=this->peers_map.end())
	{
		peer_info.assign(iter->second->get_limited_bencoded_info(info_hash));
	}

	LeaveCriticalSection(&peers_map_cs);
	return peer_info;
}
string PeersManager::get_peers_list()
{
	EnterCriticalSection(&peers_map_cs);
	
	string pl;
	std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.begin();
	while(iter!=this->peers_map.end())
	{
		pl.append("Peer Id :");
		pl.append(iter->first);
		pl.append("\r\n  Peer Ip :");
		pl.append(iter->second->get_peer_ip_adr());
		pl.append("\r\n");
		pl.append("-------------------------------");
		pl.append("\r\n");
		
		iter++;
	}

	LeaveCriticalSection(&peers_map_cs);
	return pl;
}
string PeersManager::get_html_peers_list()
{
	EnterCriticalSection(&peers_map_cs);
	
	string pl;
	std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.begin();
	while(iter!=this->peers_map.end())
	{
		pl.append("<p>");
		pl.append("Peer Id :");
		pl.append(iter->first);
		pl.append("  --------  Peer Ip :");
		pl.append(iter->second->get_peer_ip_adr());
		pl.append("</p>");
		
		iter++;
	}

	LeaveCriticalSection(&peers_map_cs);
	return pl;
}
void PeersManager::update_peer(string &p_id,string &ip,int port,int nw)
{
	EnterCriticalSection(&peers_map_cs);
	
	PeerInfo * pi=get_peer_info(p_id,ip);
	if(pi!=NULL)
	{
		pi->set_number_of_wanted_peers(nw);
		pi->set_peer_ip_adr(ip);
		pi->set_peer_port(port);
		pi=NULL;
	}
	LeaveCriticalSection(&peers_map_cs);
}


void PeersManager::add_torrent_of_interest(string &peer_id,string &info_hash, int uploaded,int downloaded,int left,std::string &ev,string &ip)
{
	
	if(!peer_id.empty() && !info_hash.empty() && ! ip.empty())
	{
		PeerInfo * pi=get_peer_info(peer_id,ip);
		if(pi!=NULL)
		{
			EnterCriticalSection(&peers_map_cs);
			pi->UpdateLastUpdateDate();
			pi->add_torrent_of_interest(info_hash,uploaded,downloaded,left,ev);
			pi=NULL;
			LeaveCriticalSection(&peers_map_cs);
		
		}
	}

}
	
bool PeersManager::is_it_a_seeder(string &peer_id,string &info_hash)
{
	EnterCriticalSection(&peers_map_cs);
	bool resp=false;
	PeerInfo * pi=get_peer_info(peer_id,string(""));
	if(pi!=NULL)
	{
		resp=pi->am_i_seeder(info_hash);
		pi=NULL;
	}
	LeaveCriticalSection(&peers_map_cs);
	return resp;

}

PeerInfo * PeersManager::get_peer_info(string &peer_id, string &peer_ip)
{
	EnterCriticalSection(&peers_map_cs);

	std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.begin();
	
	while(iter!=this->peers_map.end())
	{
		if(iter->first.compare(peer_id)==0 && peer_ip.size()&& iter->second->get_peer_ip_adr().compare(peer_ip)==0)
			break;
		if(iter->first.compare(peer_id)==0 )
			break;

		iter++;
	}

	if(iter==this->peers_map.end())
		{
			LeaveCriticalSection(&peers_map_cs);
			return NULL;
	}
	else {
			
			LeaveCriticalSection(&peers_map_cs);
			return iter->second;
	}

	LeaveCriticalSection(&peers_map_cs);

}

bool PeersManager::eliminate_confusion(string &peer_id, string &torrent_id,string &peer_ip, char * to_delete)
{
EnterCriticalSection(&peers_map_cs);

std::map<std::string,PeerInfo* >::iterator iter=this->peers_map.begin();
	
	while(iter!=this->peers_map.end())
	{
		if(iter->first.compare(peer_id)!=0)
		{
			// Consider the case : if the peer changed his id but his has the same ip @
			// In that case just update the id and the other attributes without adding a new
			// Peer to the map or drop the last one and add another
			if(iter->second->get_peer_ip_adr().compare(peer_ip)==0)
			{
				// The same ip @
				// drop the other one
				
				strcpy(to_delete,(char*)iter->first.c_str());
				strcat(to_delete,"\0");
				// Drop the peer
				delete iter->second;
				iter=this->peers_map.erase(iter);
				LeaveCriticalSection(&peers_map_cs);
				return true;
			}else{
				// It is not the same ip @
				// Continue looking for the peer
				iter++;
				continue;
			}
		}
		iter++;
	}
	LeaveCriticalSection(&peers_map_cs);
	return false;
	
}
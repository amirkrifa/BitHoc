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
* \file PeerInfo.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _PEER_INFO_H
#define _PEER_INFO_H

#pragma once
#pragma warning(disable:4786)
#include <string>
#include <iostream>
#include <map>
#include <list>
using namespace std;

/*!
* \struct torrent_state
* \brief This structure is used to maintain a torrent status(number of uploaded bytes,number of downloaded,number of bytes left, and the event).
*/

typedef struct torrent_state{
		int uploaded;
		int downloaded;
		int left;
		char state_event[20];
		
}torrent_state;


/*!
* \class PeerInfo
* \brief This Class is used to store a peer attributes and status.
*/
class PeerInfo
{
public:
	PeerInfo(void);
	PeerInfo(PeerInfo &p);
	/*!
	* \fn void set_peer_port(int port)
	* \brief Used to set the peer communication port.
	*/
	inline void set_peer_port(int port)
	{
		UpdateLastUpdateDate();
		this->peer_port=port;
	};
	/*!
	* \fn void set_peer_id(string &id)
	* \brief Used to set the peer id.
	*/
	inline void set_peer_id(string &id)
	{
		UpdateLastUpdateDate();
		this->peer_id.assign(id);
	};
	/*!
	* \fn void set_peer_ip_adr(string &ip_adr)
	* \brief Used to set the peer ip @.
	*/
	inline void set_peer_ip_adr(string &ip_adr)
	{
		UpdateLastUpdateDate();
		this->peer_ip_adr.assign(ip_adr);
	};
	/*!
	* \fn void set_number_of_wanted_peers(int nw)
	* \brief Used to set the number of wanted peers.
	*/
	inline void set_number_of_wanted_peers(int nw)
	{
		UpdateLastUpdateDate();
		this->numwant=nw;
	};
	/*!
	* \fn void add_torrent_of_interest(string &info_hash,int uploaded,int downloaded,int left,std::string &ev)
	* \brief Method used to add a torrent of interest.
	*/
	void add_torrent_of_interest(string &info_hash,int uploaded,int downloaded,int left,std::string &ev);
	/*!
	* \fn get_number_of_wanted_peers()
	* \brief Function used to get the number of wanted peers.
	*/
	inline int get_number_of_wanted_peers()
	{
		return this->numwant;
	};
	/*!
	* \fn int get_peer_port()
	* \brief Function used to get the peer communication port.
	*/
	inline int get_peer_port(){return peer_port;};
	/*!
	* \fn string get_peer_id()
	* \brief Function used to get the peer id.
	*/
	inline string get_peer_id(){return peer_id;};
	/*!
	* \fn string get_peer_ip_adr()
	* \brief Function used to get the peer ip address.
	*/
	inline string get_peer_ip_adr(){return peer_ip_adr;};
	/*!
	* \fn bool am_i_seeder(std::string &info_hash)
	* \brief Function used to check if the peer is a seeder or not.
	*/
	inline bool am_i_seeder(std::string &info_hash)
	{
		std::map<std::string, struct torrent_state *>::iterator iter;
		if(this->torrents_of_interest.size())
		{
			iter=this->torrents_of_interest.find(info_hash);
			if(iter!=this->torrents_of_interest.end())
				return (iter->second->left==0);
			else return false;
		}
		return false;
	}
	/*!
	* \fn string get_bencoded_info(string &torrent_id)
	* \brief Function used to get the peer status as a bencoded message.
	*/
	string get_bencoded_info(string &torrent_id);
	string get_html_bencoded_info(string &torrent_id);
	/*!
	* \fn string get_limited_bencoded_info(string &torrent_id)
	* \brief Function used to get the restricted peer status as a bencoded message.
	*/
	string get_limited_bencoded_info(string &torrent_id);
	
	/*!
	* \fn void SetNumberOfHops(int i)
	* \brief Function used to set the number of hops.
	*/
	
	inline void SetNumberOfHops(int i)
	{
		this->number_of_hops=i;
	};
	/*!
	* \fn 	int GetNumberOfHops()
	* \brief Function used to get the number of hops.
	*/
	inline int GetNumberOfHops()
	{
		return this->number_of_hops;
	};
	/*!
	* \fn 	void UpdateLastUpdateDate()
	* \brief Function used to update the last update date.
	*/
	inline void UpdateLastUpdateDate()
	{
		// Getting the local current time
		SYSTEMTIME systime={0};
		GetLocalTime(&systime);
		uLastModified=systime.wSecond;
	}
	/*!
	* \fn 	void UpdateLastUpdateDate()
	* \brief Function used to update the last update date.
	*/
	bool OutOfDate()
	{
		// Getting the local current time
		SYSTEMTIME systime={0};
		GetLocalTime(&systime);
		WORD c_second=systime.wSecond;
		
		if(uLastModified>c_second)
		{
			if((c_second+60)>uLastModified+iTrackerClientPollInterval*2+1)
			{
				// out of date
				return true;
			}
		}else if(uLastModified<c_second){
			if(c_second>uLastModified+iTrackerClientPollInterval*2+1)
			{
				// out of date
				return true;
			}
		}
		return false;
	}
	/*!
	* \fn 	void SetTrackerClientPollInterval(int i)
	* \brief Function used to the tracker client poll interval.
	*/
	inline void SetTrackerClientPollInterval(int i)
	{
		iTrackerClientPollInterval=i;
	};

	
	
	~PeerInfo(void);

private:
	/*!
	* \brief The peer Id.
	*/
	string peer_id;
	/*!
	* \brief The peer port.
	*/
	int peer_port;
	/*!
	* \brief The peer Ip @.
	*/
	string peer_ip_adr;
	/*!
	* \brief Number of peers that the client would like to recive from the tracker,this value is permitted to be zero, if ommited typically defaults to 50 peers.
	*/	
	int numwant;
	/*!
	* \brief Number of hops to the peer.
	*/	
	int number_of_hops;
	
	/*!
	* \brief Map used to store the list of torrents of interest to the peer.
	*/
	map<string,torrent_state *> torrents_of_interest;
	//! Last time the peer status was updated
	WORD uLastModified;
	int iTrackerClientPollInterval;

};

#endif _PEER_INFO_H
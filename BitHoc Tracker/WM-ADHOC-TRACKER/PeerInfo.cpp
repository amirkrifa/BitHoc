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
* \file PeerInfo.cpp
* \brief Implementation of the class PeerInfo used to store the status of a given peers. 
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#include "StdAfx.h"
#include "PeerInfo.h"

extern int AdHocVersion;
PeerInfo::PeerInfo(void)
{
	// Getting the local current time
	SYSTEMTIME systime={0};
	GetLocalTime(&systime);
	uLastModified=systime.wSecond;
	iTrackerClientPollInterval=0;
}
PeerInfo::PeerInfo(PeerInfo &p)
{
	// Getting the local current time
	SYSTEMTIME systime={0};
	GetLocalTime(&systime);
	uLastModified=systime.wSecond;
}

void PeerInfo::add_torrent_of_interest(string &info_hash, int uploaded,int downloaded,int left,std::string &ev)
{

	UpdateLastUpdateDate();
	std::map<std::string,torrent_state *>::iterator iter;
	string temp2(info_hash);
	int i=this->torrents_of_interest.size();
	iter=this->torrents_of_interest.find(temp2);
	
	if(iter!=this->torrents_of_interest.end())
	{// Updating an existing torrent
		iter->second->downloaded=downloaded;
		iter->second->uploaded=uploaded;
		iter->second->left=left;
		strcpy(iter->second->state_event,ev.c_str());
		
	}else // Adding a new torrent
	{
		torrent_state * ptr_ts=(torrent_state *)malloc(sizeof(torrent_state));
		ptr_ts->uploaded=uploaded;
		ptr_ts->downloaded=downloaded;
		ptr_ts->left=left;
		strcpy(ptr_ts->state_event,"");
		strcpy(ptr_ts->state_event,ev.c_str());
		strcat(ptr_ts->state_event,"\0");
		bool x=true;
		this->torrents_of_interest.insert(make_pair<string,torrent_state *>(temp2,ptr_ts));
		ptr_ts=NULL;
	}
	
}

PeerInfo::~PeerInfo(void)
{
	if(this->torrents_of_interest.size()>0)
	{
		// Cleaning the map of torrents
		std::map<std::string, torrent_state*>::iterator iter=this->torrents_of_interest.begin();
		while(iter!=this->torrents_of_interest.end())
		{
			if(iter->second!=NULL)
				free(iter->second);
			iter++;
		}
		this->torrents_of_interest.clear();
	}
	
}
string PeerInfo::get_html_bencoded_info(string &torrent_id)
{
	string binfo;
	//! start of the dictionary.
	
	binfo.append("<p>");
	//! The peer id.
	binfo.append("peer id: ");
	binfo.append(this->peer_id.c_str());
	binfo.append("  --  ");
	//! The peer ip address.
	binfo.append("ip @: ");
	binfo.append(this->peer_ip_adr);
	binfo.append("  --  ");
	//! The peer port.
	binfo.append("port: ");
	char pport[10];
	sprintf(pport,"%i",this->get_peer_port());
	binfo.append(pport);
	binfo.append("  --  ");
	//! The peer number of hops.
	if(AdHocVersion)
	{
		binfo.append("hops: ");
		char nbrhops[10];
		sprintf(nbrhops,"%i",this->number_of_hops);
		binfo.append(nbrhops);
		binfo.append("  --  ");
	}
	map<string,torrent_state *>::iterator iter= this->torrents_of_interest.find(torrent_id);
	if(iter!=this->torrents_of_interest.end())
	{
		//!Uploaded.
		binfo.append("Uploaded: ");
		char up[10];
		sprintf(up,"%i",iter->second->uploaded);
		binfo.append(up);
		binfo.append("  --  ");
		//!Downloaded.
		binfo.append("Downloaded: ");
		char dow[10];
		sprintf(dow,"%i",iter->second->downloaded);
		binfo.append(dow);
		binfo.append("  --  ");		
		//!Left.
		binfo.append("Left:");
		char lef[10];
		sprintf(lef,"%i",iter->second->left);
		binfo.append(lef);
		binfo.append("  --  ");
		//!Event.
		binfo.append("Event: ");
		string temp(iter->second->state_event);
		binfo.append(temp);
	}
	//! End of the dictionary.
	binfo.append("</p>");
	
	return binfo;
}
string PeerInfo::get_bencoded_info(string &torrent_id)
{
	string binfo;
	//! start of the dictionary.
	binfo.append("d");
	
	//! The peer id.
	binfo.append("7:peer id");
	int pid_length=this->peer_id.length();
	char str_pid_length[30];
	sprintf(str_pid_length,"%i:",pid_length);
	binfo.append(str_pid_length);
	binfo.append(this->peer_id.c_str());
	
	//! The peer ip address.
	binfo.append("2:ip");
	int pip_length=this->peer_ip_adr.length();
	char str_pip_length[20];
	sprintf(str_pip_length,"%i:",pip_length);
	binfo.append(str_pip_length);	
	binfo.append(this->peer_ip_adr);
	
	//! The peer port.
	binfo.append("4:port");
	char pport[10];
	sprintf(pport,"%i",this->get_peer_port());
	binfo.append("i");
	binfo.append(pport);
	binfo.append("e");
	
	//! The peer number of hops.
	if(AdHocVersion)
	{
		binfo.append("4:hops");
		char nbrhops[10];
		sprintf(nbrhops,"%i",this->number_of_hops);
		binfo.append("i");
		binfo.append(nbrhops);
		binfo.append("e");
	}
	map<string,torrent_state *>::iterator iter= this->torrents_of_interest.find(torrent_id);
	if(iter!=this->torrents_of_interest.end())
	{
		//!Uploaded.
		binfo.append("8:uploaded");
		char up[10];
		sprintf(up,"%i",iter->second->uploaded);
		binfo.append("i");
		binfo.append(up);
		binfo.append("e");
		
		//!Downloaded.
		binfo.append("10:downloaded");
		char dow[10];
		sprintf(dow,"%i",iter->second->downloaded);
		binfo.append("i");
		binfo.append(dow);
		binfo.append("e");
		
		//!Left.
		binfo.append("4:left");
		char lef[10];
		sprintf(lef,"%i",iter->second->left);
		binfo.append("i");
		binfo.append(lef);
		binfo.append("e");
		
		//!Event.
		binfo.append("5:event");
		string temp(iter->second->state_event);
		int event_length=temp.length();
		char str_event_length[20];
		sprintf(str_event_length,"%i:",event_length);
		binfo.append(str_event_length);
		binfo.append(temp);
	
	}
	//! End of the dictionary.
	binfo.append("e");	
	
	return binfo;
}
string PeerInfo::get_limited_bencoded_info(string &torrent_id)
{
	string binfo;
	//! start of the dictionary.
	binfo.append("d");
	
	//! The peer id.
	binfo.append("7:peer id");
	int pid_length=this->peer_id.length();
	char str_pid_length[30];
	sprintf(str_pid_length,"%i:",pid_length);
	binfo.append(str_pid_length);
	binfo.append(this->peer_id.c_str());
	
	//! The peer ip address.
	binfo.append("2:ip");
	int pip_length=this->peer_ip_adr.length();
	char str_pip_length[20];
	sprintf(str_pip_length,"%i:",pip_length);
	binfo.append(str_pip_length);	
	binfo.append(this->peer_ip_adr);
	
	//! The peer port.
	binfo.append("4:port");
	char pport[10];
	sprintf(pport,"%i",this->get_peer_port());
	binfo.append("i");
	binfo.append(pport);
	binfo.append("e");

	//! The peer number of hops.
	if(AdHocVersion)
	{
		binfo.append("4:hops");
		char nbrhops[10];
		sprintf(nbrhops,"%i",this->number_of_hops);
		binfo.append("i");
		binfo.append(nbrhops);
		binfo.append("e");
	}
	//! End of the dictionary.
	binfo.append("e");	
	
	return binfo;
}

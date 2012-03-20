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
* \file NeighborsManager.cpp
* \brief Implementation of the class NeighborsManager used to store and manage the trackers overlay. 
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "StdAfx.h"
#include "NeighborsManager.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include "SpyThreadParam.h"
#include "socket.h"
#include "WMRoutingTable.h"
#include "Ping.h"
#include <sstream>
#include "BlackIpList.h"

bool STOP_SPY=FALSE;
extern int iUseTrackerOverlay;
NeighborsManager::NeighborsManager(WMRoutingTable * wmrt)
{ 
	this->wmrt=wmrt;
	
	InitializeCriticalSection(&neighbors_map_cs);
	InitializeCriticalSection(&trackers_map_cs);
}

NeighborsManager::~NeighborsManager(void)
{
	DeleteCriticalSection(&neighbors_map_cs);
	DeleteCriticalSection(&trackers_map_cs);
	
	this->wmrt=NULL;
	if(!this->neighbors_map.empty())
		this->neighbors_map.clear();
	
	if(!this->overlay_trackers_map.empty())
		this->overlay_trackers_map.clear();
	
	// Cleaning the trackers map
	if(!this->trackers_map.empty())
	{
		TrackersMap::iterator tm_iter=this->trackers_map.begin();
		while(tm_iter!=this->trackers_map.end())
		{
			TrackersList::iterator tl_iter=tm_iter->second.begin();
			while(tl_iter!=tm_iter->second.end())
			{
				if(*tl_iter)
					free(*tl_iter);
				tl_iter++;
			}
			tm_iter++;
		}
		this->trackers_map.clear();
	}
}


// Adding a new neighbor
int NeighborsManager::add_new_neighbor(std::string &ip_adr,std::string &status)
{	
	EnterCriticalSection(&neighbors_map_cs);
	std::map<std::string,std::string>::iterator iter;
	iter=this->neighbors_map.find(ip_adr);
	if(iter==this->neighbors_map.end())
	{
		this->neighbors_map[ip_adr]=status;
		LeaveCriticalSection(&neighbors_map_cs);
		return 1;
	}
	else {
		// The neighbor already exists
		LeaveCriticalSection(&neighbors_map_cs);
		return -1;
	}
	LeaveCriticalSection(&neighbors_map_cs);
}

// Get the neighbor status
std::string NeighborsManager::get_neighbor_status(std::string &ip_adr)
{
	EnterCriticalSection(&neighbors_map_cs);
	std::map<std::string,std::string>::iterator iter;
	iter=this->neighbors_map.find(ip_adr);
	if(iter==this->neighbors_map.end())
	{
		LeaveCriticalSection(&neighbors_map_cs);
		return NULL;
	}
	else {
		// The neighbor already exists
		LeaveCriticalSection(&neighbors_map_cs);
		return this->neighbors_map[ip_adr];
	}

}

UINT NeighborsManager::spy_thread_function(LPVOID param)
{
	
	DWORD ret;
	SpyThreadParam *rt_state=(SpyThreadParam *)param;
	if(rt_state->wmrt->is_it_the_first_time_updating_rt())
	{
		rt_state->wmrt->update_rt_map();
		rt_state->wmrt->continue_updating_rt();
	}
	
	while(!STOP_SPY)
	{
		ret = NotifyAddrChange(NULL, NULL);
		if (ret != NO_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				char * error=new char[40];
				sprintf(error, "NotifyAddrChange error...%d\n", WSAGetLastError());				
				rt_state->set_tracking_error(error);
				continue;
			}
	
		}else {
				// IP Address table changed
				rt_state->rt_is_modified();
			
		}
		// Wait some seconds before reverifying the routing table
		int iI=1000;
		while(STOP_SPY&&iI>0)
		{
			Sleep(5);
			iI--;
		}

	}

	if(rt_state!=NULL)
		delete rt_state;
	AfxEndThread(0);
	return 0;

	
}

bool NeighborsManager::notification_of_changing_in_rt()
{

	SpyThreadParam *parm=new SpyThreadParam(this->wmrt); 
	parm->ptr_nm=this;
	this->m_pThread = AfxBeginThread(spy_thread_function,parm);
    if(!m_pThread)
    {
       // The thread could not be created 
       return false;
    }
       return true;       
}

// Add a tracker to the map ( Corresponding to a specific torrent)
void NeighborsManager::add_tracker(string &info_hash,string &ip,int port)
{
	EnterCriticalSection(&neighbors_map_cs);
	
	bool found=false;
	TrackersMap::iterator map_iter;
	map_iter=this->trackers_map.find(info_hash);
	if(map_iter==this->trackers_map.end())
	{
		// New Torrent
		TrackersList tl;
		tracker_neighbor_info * tni=(tracker_neighbor_info*)malloc(sizeof(tracker_neighbor_info));
		strcpy(tni->ip,"");
		strcpy(tni->ip,ip.c_str());
		strcat(tni->ip,"\0");
		//tni->ip.assign(ip.c_str());
		tni->port=port;
		this->trackers_map.insert(make_pair<string,TrackersList>(info_hash,tl));
		this->trackers_map[info_hash].push_back(tni);
		
	}else 
	{
		// The torrent already exists
		// Looking for the tracker
		TrackersList::iterator list_iter=map_iter->second.begin();
		while(list_iter!=map_iter->second.end())
		{
			if(ip==(*list_iter)->ip)
			{
				// Tracker already exists
				found=true;
				break;
			}
			list_iter++;
		}
		if(found)
		{
			// Adding the new Tracker
			tracker_neighbor_info * tni=(tracker_neighbor_info*)malloc(sizeof(tracker_neighbor_info));
			strcpy(tni->ip,"");
			strcpy(tni->ip,ip.c_str());
			strcat(tni->ip,"\0");
			//tni->ip.assign(ip.c_str());
			tni->port=port;
			map_iter->second.push_back(tni);
		}
	}

	LeaveCriticalSection(&neighbors_map_cs);
}


// Send a peers dictionary for all trackers interested in the torrent info_hash

void NeighborsManager::send_peers_interested_in(string &info_hash, string &peers_dict, string &info_source,int port, BlackIpList * bl)
{
	struct thread_info_param * __unaligned tip=(struct thread_info_param *__unaligned) malloc(sizeof(struct thread_info_param));
	tip->black_list = bl;
	tip->nm=this;
	
	strcpy(tip->peers_dict,"");
	strcpy(tip->peers_dict,peers_dict.c_str());
	strcat(tip->peers_dict,"\0");
	
	strcpy(tip->info_hash,"");
	strcpy(tip->info_hash,info_hash.c_str());
	strcat(tip->info_hash,"\0");
	
	strcpy(tip->info_source,"");
	strcpy(tip->info_source,info_source.c_str());
	strcat(tip->info_source,"\0");
	tip->pWmrt=this->wmrt;
	tip->port=port;
	CWinThread *TPCThread; 
	if(!iUseTrackerOverlay)
		TPCThread=AfxBeginThread(T2T_Info_to_all,(LPVOID)tip);
	else
		TPCThread=AfxBeginThread(T2T_Info_to_the_nearest,(LPVOID)tip);

	
}

void NeighborsManager::send_sync_request_to_all(int port,string &hash , BlackIpList * bl)
{
	struct thread_info_param * __unaligned tip=(struct thread_info_param *__unaligned) malloc(sizeof(struct thread_info_param));
	tip->nm=this;
	tip->pWmrt=this->wmrt;
	tip->port = port;
	tip->black_list = bl;
	strcpy(tip->info_hash,(char*)hash.c_str());
	AfxBeginThread(T2T_sync,(LPVOID)tip);
}

UINT NeighborsManager::T2T_Info_to_the_nearest(LPVOID ptr_tp)
{
	struct thread_info_param *__unaligned tip=(struct thread_info_param * )ptr_tp;
	if(!tip->nm->overlay_trackers_map.empty())
	{
		OverlayTrackersMap::iterator ov_iter=tip->nm->overlay_trackers_map.begin();
		tip->nm->update_tracker_overlay_list();
		// Creating the future black list
		while (ov_iter!=tip->nm->overlay_trackers_map.end())
		{	
			if(ov_iter->first.compare(tip->info_source)!=0 && !tip->black_list->IsIpInTheBlackList(ov_iter->first))
			{
				string tmp = ov_iter->first;
				tip->black_list->AddIpToBlackList(tmp);	
			}	
			ov_iter++;
		}

		ov_iter=tip->nm->overlay_trackers_map.begin();
		while (ov_iter!=tip->nm->overlay_trackers_map.end())
		{	
			if(ov_iter->first.compare(tip->info_source)!=0)
			{
			
				try{
					SocketClient out(ov_iter->first,ov_iter->second);
					string message;
					message.append("GET /t2t-info?peers=");
					message.append(tip->peers_dict);
					message.append("&info_hash=");
					message.append(tip->info_hash);
					message.append("&ip=");
					message.append(tip->nm->wmrt->get_local_ip_adr());
					message.append("&port=");
					char temp_port[10];
					sprintf(temp_port,"%i",ov_iter->second);
					message.append(temp_port);
					// Add the blackList
					message.append("&black_list=");
					message.append(tip->black_list->GetFormattedList());
					message.append(" HTTP/1.1\n\n");
					out.SendLine(message);
	
					out.Close();
					ov_iter++;
				}
				catch(string  e)
				{
					if(ov_iter!=tip->nm->overlay_trackers_map.end())
					{
						ov_iter++;
						continue;
					}else break;
				}
			}else {
					if(ov_iter!=tip->nm->overlay_trackers_map.end())
					{
						ov_iter++;
						continue;
					}else break;		
			}
		}
	}
	delete tip->black_list;
	free(tip);
	AfxEndThread(0);
	return 0;
}

UINT NeighborsManager::T2T_Info_to_all(LPVOID ptr_tp)
{
	
	struct thread_info_param * tip=(struct thread_info_param * )ptr_tp;
	tip->pWmrt->get_exploration_map();
	tip->pWmrt->enter_exp_map_cs();
	
	if(!tip->pWmrt->exploration_map.empty())
	{
		// Iterators for the exploration map
		map<int,list<string>>::iterator exp_map_iter=tip->pWmrt->exploration_map.begin();
		list<string>::iterator ips_iter;
		
		// Creating the future black list
		while((exp_map_iter!=tip->pWmrt->exploration_map.end()))
		{
			ips_iter=exp_map_iter->second.begin();
			int i=0;
			while(ips_iter!=exp_map_iter->second.end())
			{
				if((*ips_iter).compare(tip->info_source) != 0 && tip->black_list->IsIpInTheBlackList(*ips_iter))
				{
					tip->black_list->AddIpToBlackList(*ips_iter);
				}
				ips_iter++;
			}
			exp_map_iter++;
		}


		exp_map_iter=tip->pWmrt->exploration_map.begin();
		while((exp_map_iter!=tip->pWmrt->exploration_map.end()))
		{
			ips_iter=exp_map_iter->second.begin();
			int i=0;
			while(ips_iter!=exp_map_iter->second.end())
			{
				if((*ips_iter).compare(tip->info_source)!=0)
				{
					try
					{
						SocketClient out((*ips_iter),tip->port);
						string message;
						message.append("GET /t2t-info?peers=");
						message.append(tip->peers_dict);
						message.append("&info_hash=");
						message.append(tip->info_hash);
						message.append("&ip=");
						message.append(tip->nm->wmrt->get_local_ip_adr());
						message.append("&port=");
						char temp_port[10];
						sprintf(temp_port,"%i",tip->port);
						message.append(temp_port);
						
						// Adding the black list
						message.append("&black_list=");
						message.append(tip->black_list->GetFormattedList());
						
						message.append(" HTTP/1.1\n\n");
						out.SendLine(message);
	
						out.Close();
						ips_iter++;
						continue;
					}
					catch(string  e)
					{
						if(ips_iter!=exp_map_iter->second.end())
						{
							ips_iter++;
							continue;
						}else break;
					}
				}
				ips_iter++;
			}
			exp_map_iter++;
		}
	}
	tip->pWmrt->exit_exp_map_cs();
	free(tip);
	AfxEndThread(0);
	return 0;
}


UINT NeighborsManager::T2T_sync(LPVOID param)
{
	struct thread_info_param *__unaligned tip=(struct thread_info_param * )param;
	tip->pWmrt->get_exploration_map();
	tip->pWmrt->enter_exp_map_cs();
	
	if(!tip->pWmrt->exploration_map.empty())
	{
		// Iterators for the exploration map
		map<int,list<string>>::iterator exp_map_iter=tip->pWmrt->exploration_map.begin();
		list<string>::iterator ips_iter;
		

				
		while((exp_map_iter!=tip->pWmrt->exploration_map.end()))
		{
			ips_iter=exp_map_iter->second.begin();
			while(ips_iter!=exp_map_iter->second.end())
			{
				if((*ips_iter).compare(tip->info_source)!=0 && !tip->black_list->IsIpInTheBlackList(*ips_iter))
				{
					tip->black_list->AddIpToBlackList(*ips_iter);
				}
				ips_iter++;
			}
			exp_map_iter++;
		}
		
		exp_map_iter=tip->pWmrt->exploration_map.begin();
		while((exp_map_iter!=tip->pWmrt->exploration_map.end()))
		{
			ips_iter=exp_map_iter->second.begin();
			int i=0;
			while(ips_iter!=exp_map_iter->second.end())
			{
				if((*ips_iter).compare(tip->info_source)!=0)
				{
					try
					{
						SocketClient out((*ips_iter),tip->port);
						string message;
						message.append("GET /sync-forward");
						message.append("?info_hash=");
						message.append(tip->info_hash);
						// Add the black list
						message.append("&black_list=");
						message.append(tip->black_list->GetFormattedList());
						message.append(" HTTP/1.1\n\n");
						out.SendLine(message);
						out.Close();
						ips_iter++;
						continue;
					}
					catch(string  e)
					{
						if(ips_iter!=exp_map_iter->second.end())
						{
							ips_iter++;
							continue;
						}else break;
					}
				}
				ips_iter++;
			}
			exp_map_iter++;
		}
	}
	tip->pWmrt->exit_exp_map_cs();
	free(tip);
	AfxEndThread(0);
	return 0;

}
void NeighborsManager::add_tracker(string ip,int port)
{
	OverlayTrackersMap::iterator ov_iter=this->overlay_trackers_map.find(ip);
	if(ov_iter==this->overlay_trackers_map.end())
	{
		// New Tracker
		this->overlay_trackers_map.insert(make_pair<string,int>(ip,port));

	}
}

void NeighborsManager::update_tracker_overlay_list()
{
Ping send_ping;
OverlayTrackersMap::iterator ov_iter;
ov_iter=this->overlay_trackers_map.begin();
while(ov_iter!=this->overlay_trackers_map.end())
{
	string ping_res;
	
	ping_res.append(send_ping.send_a_ping((char *)ov_iter->first.c_str()));
	
	if(ping_res.find("Received")==string::npos)
	{// deleting the tracker
		ov_iter=this->overlay_trackers_map.erase(ov_iter);continue;
	}

	ov_iter++;
}
}

string NeighborsManager::get_overlay_trackers_list()
{
	if(!this->overlay_trackers_map.empty())
	{
		string res;
		OverlayTrackersMap::iterator ov_iter=this->overlay_trackers_map.begin();
		while(ov_iter!=this->overlay_trackers_map.end())
		{
			res.append(ov_iter->first);
			res.append(", ");
			ov_iter++;
		}
		return res;
	}
	int iNumberOfKnownTrackers=this->wmrt->GetNumberOfKnownTrackers();
	std::ostringstream oss;
	oss<<iNumberOfKnownTrackers;

	return string(oss.str()+" Trackers");
}


// Used to forward the fact of publishing a new local file
void NeighborsManager::send_locally_stored_file(string & fn,unsigned int fs, string &ip, int tPort,BlackIpList * bl )
{
	struct storing_thread_param * __unaligned tip = (struct storing_thread_param *__unaligned) malloc(sizeof(struct storing_thread_param));
	tip->black_list = bl;
	tip->nm = this;
	tip->pWmrt = this->wmrt;
	strcpy(tip->file_name, fn.c_str());
	strcpy(tip->node_ip, ip.c_str());
	tip->tracker_port = tPort;
	tip->file_size = fs;
	CWinThread *TPCThread; 
	TPCThread = AfxBeginThread(T2tStorageInfoToAll,(LPVOID)tip);
}

UINT NeighborsManager::T2tStorageInfoToAll(LPVOID ptr_tp)
{
	
	struct storing_thread_param *__unaligned tip=(struct storing_thread_param * )ptr_tp;
	tip->pWmrt->get_exploration_map();
	tip->pWmrt->enter_exp_map_cs();
	string local_ip(tip->pWmrt->get_local_ip_adr());

	if(!tip->pWmrt->exploration_map.empty())
	{
		// Iterators for the exploration map
		map<int,list<string>>::iterator exp_map_iter=tip->pWmrt->exploration_map.begin();
		list<string>::iterator ips_iter;
			
		// Creating the future black list
		while((exp_map_iter!=tip->pWmrt->exploration_map.end()))
		{
			ips_iter=exp_map_iter->second.begin();
			int i=0;
			while(ips_iter!=exp_map_iter->second.end())
			{
				if((*ips_iter).compare(local_ip) != 0 && tip->black_list->IsIpInTheBlackList(*ips_iter))
				{
					tip->black_list->AddIpToBlackList(*ips_iter);
				}
				ips_iter++;
			}
			exp_map_iter++;
		}


		exp_map_iter=tip->pWmrt->exploration_map.begin();
		
		while((exp_map_iter!=tip->pWmrt->exploration_map.end()))
		{
			ips_iter = exp_map_iter->second.begin();
			int i = 0;
			while(ips_iter != exp_map_iter->second.end())
			{
				if((*ips_iter).compare(local_ip) !=0 )
				{
					try
					{
						SocketClient out((*ips_iter), tip->tracker_port);
						string message;
						message.append("GET /NewTorrent?file_name=");
						message.append(tip->file_name);
						message.append("&file_size=");
						char tmp[100];
						sprintf(tmp,"%i", tip->file_size);
						message.append(tmp);
						message.append("&ip=");
						message.append(tip->node_ip);
						// Adding the black list
						message.append("&black_list=");
						message.append(tip->black_list->GetFormattedList());
						message.append(" HTTP/1.1\n\n");
						out.SendLine(message);
						out.Close();
						ips_iter++;
						continue;
					}
					catch(string  e)
					{
						if(ips_iter != exp_map_iter->second.end())
						{
							ips_iter++;
							continue;
						}else break;
					}
				}
				ips_iter++;
			}
			exp_map_iter++;
		}
	}
	tip->pWmrt->exit_exp_map_cs();
	delete tip->black_list;
	free(tip);
	AfxEndThread(0);
	return 0;
}

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
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*!
* \file WMRoutingTable.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _WM_ROUTING_TABLE_H
#define _WM_ROUTING_TABLE_H

#pragma once

#include <stdio.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <ws2tcpip.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#include <string>
#include <map>
#include <list>

using namespace std;

typedef list<string> IpList;
typedef map<int, IpList>ExplorationMap;
typedef list<int> IntList;
typedef struct {
	double hour;
	double minute;
	double second;
}time_stamp;
typedef map<string,time_stamp*> NeighborsTS;

class SocketServerBroadcast;
/*!
* \class WMRouting table
* \brief This class is used to maintain an updated view of the device routing table.
*/
class WMRoutingTable
{
public:
	WMRoutingTable(
);
	/*!
	* \fn string get_broadcast_addr()
	* \brief Returns the broadcast address used by the broadcast server in order to catch the 1 hop neighbors.
	*/
	string get_broadcast_addr();
	/*!
	* \fn char* get_routing_table()
	* \brief Returns the device routing table.
	*/
	char* get_routing_table();
	char* get_routing_table_from_the_map();
	char* get_html_routing_table_from_the_map();
	/*!
	* \fn bool update_rt_map()
	* \brief Used to update the routing table map.
	*/
	bool update_rt_map();

	void continue_updating_rt(){first_time=false;};
	bool is_it_the_first_time_updating_rt(){return first_time;};
	/*!
	* \fn bool add_route_entry()
	* \brief Used to get a new routing entry.
	*/
	bool add_route_entry();
	/*!
	* \fn bool get_exploration_map()
	* \brief Used to extract the exploration map of the differents known peers in increasing order of their metrics.
	*/
	bool get_exploration_map();
	/*!
	* \fn bool get_exploration_map()
	* \brief Update the list of metrics from the routing table and sort it in increasing order.
	*/
	bool update_metrics_list();
	void enter_exp_map_cs(){EnterCriticalSection(&exploration_map_sync);};
	void exit_exp_map_cs(){LeaveCriticalSection(&exploration_map_sync);};
	/*!
	* \fn bool is_valid(string &ip)
	* \brief Used to verify if a routing table entry is valid or not.
	*/
	bool is_valid(const string &ip);
	/*!
	* \fn string get_local_ip_adr()
	* \brief Used to get the local ip @.
	*/
	string get_local_ip_adr();
	/*!
	* \fn bool is_it_reachable(string &ip)
	* \brief Used to verify if a given host is reachable or not by sending a ping request.
	*/
	bool is_it_reachable(string &ip);
	/*!
	* \fn void SetBroadcastParam(string &a,int p,int bi,int httpport)
	* \brief Used to set the broadcast parameters, ip @,port and interval.
	*/
	void SetBroadcastParam(string &a,int p,int bi,int httpport);
	/*!
	* \fn void update_1hop_neighbors_map()
	* \brief Used to update the 1 hop neighbors map.
	*/ 
	void update_1hop_neighbors_map();
	/*!
	* \fn void add_1hop_neighbor(string &ip)
	* \brief Used to add new entry to the one hop neighbors map.
	*/
	void add_1hop_neighbor(string &ip);
	/*!
	* \fn void start_brodcast_messages_receiver(int port)
	* \brief Used to start the broadcast messages receiver.
	*/
	void start_brodcast_messages_receiver(int port);
	/*!
	* \fn void look_for_1hop_neighbors()
	* \brief Used to start looking for the 1 hop neighbors.
	*/
	void look_for_1hop_neighbors();
	/*!
	* \fn static UINT receive_broadcast_message(LPVOID param)
	* \brief Used as a thread start point that will take in charge a new received broadcast message.
	*/
	static UINT receive_broadcast_message(LPVOID param);
	/*!
	* \fn static UINT send_broadcast_message(LPVOID param)
	* \brief Used as a thread start point that will send periodically broadcast messages.
	*/
	static UINT send_broadcast_message(LPVOID param);
	/*!
	* \fn int GetNumberOfHops(std::string ip)
	* \brief Used to get the number of hops for a given ip @.
	*/
	int GetNumberOfHops(std::string &ip);
	/*!
	* \fn int GetNumberOfKnownTrackers()
	* \brief Used to get the number Known trackers.
	*/
	int GetNumberOfKnownTrackers()
	{
		int iNumberOfEntiesExplorationMap=0;
		EnterCriticalSection(&exploration_map_sync);
		ExplorationMap::iterator exp_iter=this->exploration_map.begin();
		while(exp_iter!=this->exploration_map.end())
		{
			IpList::iterator iplist_iter=exp_iter->second.begin();
			while(iplist_iter!=exp_iter->second.end())
			{
				iNumberOfEntiesExplorationMap++;
				iplist_iter++;
			}
			exp_iter++;
		}
		LeaveCriticalSection(&exploration_map_sync);
		//if(iNumberOfEntiesExplorationMap>1)
		return iNumberOfEntiesExplorationMap;
		//else return 0;
	};
	//! Indicates if the routing table has been modified
	bool routing_table_was_modified;
	//! Exploration map
	ExplorationMap exploration_map;
	~WMRoutingTable(void);
	
private:	
	typedef struct
	{
		int b_port;
		int b_interval;
		char b_message[200];
		char b_address[30];
		WMRoutingTable * ptr_wmrt;
		SocketServerBroadcast *ssb;
	}b_thread_param;
	//! Routing table map.
	std::map<std::string,int> rt_map;
	//! Bolean that indicates if we are updating the routing table or not.
	bool updating;
	//! Indicates if it is the first time to get the routing table.
	bool first_time;
	//! Metrics List.
	IntList metrics_list;
	//! Critical section associated to the exploration map.
	CRITICAL_SECTION exploration_map_sync;
	//! Neighbors map.
	NeighborsTS neighbors_map;
	//! Broadcast address.
	string broadcast_addr;
	//! Broadcast port.
	int broadcast_port;
	SocketServerBroadcast *ssb;
	int broadcast_interval;
	int http_server_port;
	
};



#endif _WM_ROUTING_TABLE_H
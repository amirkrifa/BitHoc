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
* \file NeighborsManager.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _NEIGHBORS_MANAGER_H
#define _NEIGHBORS_MANAGER_H


#pragma once
#include <Afxwin.h>
#include <string>
#include <map>
#include <list>
using namespace std;

/*!
* \struct tracker_neighbor_info
* \brief This structer is used to store some infos about a neighbor(ip @ and communication port).
*/
struct tracker_neighbor_info{
	char ip[20];
	int port;
};



typedef list<tracker_neighbor_info*> TrackersList;
typedef std::map<string,TrackersList> TrackersMap;
typedef std::map<string,int> OverlayTrackersMap;

class WMRoutingTable;
class BlackIpList;
/*!
* \class NeighborsManager
* \brief This Class is used by the Tracker to manage the Network Trackers overlay.
*/
class NeighborsManager
{
public:
	NeighborsManager(WMRoutingTable * wmrt);
	~NeighborsManager(void);
	/*!
	* \fn bool notification_of_changing_in_rt()
	* \brief This function returns a notification when there is any change in the device routing table.
	*/	
	bool notification_of_changing_in_rt();
	/*!
	* \fn static UINT spy_thread_function(LPVOID param)
	* \brief Function used as a start point to track any modification on the device rouitng table.
	*/
	static UINT spy_thread_function(LPVOID param);
	/*!
	* \fn int add_new_neighbor(std::string &ip_adr,std::string &status)
	* \brief Function used to add a new member to the neighbors list.
	*/
	int add_new_neighbor(std::string &ip_adr,std::string &status);
	/*!
	* \fn std::string get_neighbor_status(std::string &ip_adr)
	* \brief Function used to a given neighbor status.
	*/
	std::string get_neighbor_status(std::string &ip_adr);
	/*!
	* \fn void add_tracker(string info_hash,string ip,int port)
	* \brief Method used to add a tracker to the overlay.
	*/	
	void add_tracker(string &info_hash,string &ip,int port);
	void add_tracker(string ip,int port);
	/*!
	* \fn void send_peers_interested_in(string &info_hash, string &peers_dict,string &info_source,int port)
	* \brief Method used to send a given bencoded message to a set of peers.
	*/	
	void send_peers_interested_in(string &info_hash, string &peers_dict,string &info_source,int port, BlackIpList *  bl);
	/*!
	* \fn void delete_tracker_from_the_overlay(string &ip)
	* \brief Method used to delete a tracker from the overlay.
	*/
	inline void delete_tracker_from_the_overlay(string &ip)
	{
		this->overlay_trackers_map.erase(ip);
	}
	/*!
	* \fn void update_tracker_overlay_list()
	* \brief Method used to update the trackers overlay.
	*/
	void update_tracker_overlay_list();
	/*!
	* \fn static UINT T2T_Info_to_the_nearest(LPVOID ptr_tp)
	* \brief Function used as a start point for a new thread to send a bencoded message to the nearest tracker (in term of hops) within the overlay.
	*/
	static UINT T2T_Info_to_the_nearest(LPVOID ptr_tp);
	/*!
	* \fn static UINT T2T_Info_to_all(LPVOID ptr_tp)
	* \brief Function used as a start point for a new thread to send a bencoded message to all the network trackers.
	*/
	static UINT T2T_Info_to_all(LPVOID ptr_tp);
	/*!
	* \fn int get_trackers_overlay_size()
	* \brief Function that return the trackers overlay size.
	*/
	int get_trackers_overlay_size()
	{
		return this->overlay_trackers_map.size();
	}
	/*!
	* \fn static UINT T2T_sync(LPVOID param)
	* \brief Forwards a synchronisation request to all the trackers.
	*/
	static UINT T2T_sync(LPVOID param);
	/*!
	* \fn void send_sync_request_to_all()
	* \brief Sends a synchronisation request to all the known trackers.
	*/
	void send_sync_request_to_all(int,string& , BlackIpList * bl);
	string get_overlay_trackers_list();

	// Functions used to send updates describing the locally stored torrent files
	void send_locally_stored_file(string & fn,unsigned int file_size, string &ip, int tport,BlackIpList * bl );
	static UINT T2tStorageInfoToAll(LPVOID ptr_tp);
		
private:
	/*!
	* \struct thread_info_param
	* \brief This structer is used as a parameter for the thread that will be in charge of sending a bencoded message to the trackers overlay.
	*/
	struct thread_info_param{
		NeighborsManager * nm;
		char peers_dict[1024];
		char info_hash[100];
		char info_source[1024];
		int port;
		BlackIpList * black_list;
		WMRoutingTable * pWmrt;
	};
	
	typedef struct storing_thread_param{
	
	char node_ip[100];
	char file_name[1024];
	int tracker_port;
	unsigned int file_size;
	NeighborsManager * nm;
	WMRoutingTable * pWmrt;
	BlackIpList * black_list;
	}storing_thread_param;
	

	/*!
	* \brief A ptr to the WMRoutingTable object.
	*/
	WMRoutingTable * wmrt;
	/*!
	* \brief Neighbors map.
	*/
	std::map<std::string,std::string> neighbors_map;
	CRITICAL_SECTION neighbors_map_cs;
	/*!
	* \brief Trackers overlay map.
	*/
	TrackersMap trackers_map;
	// Associated Critical Session
	CRITICAL_SECTION trackers_map_cs;
	// Thread that will maintains the map of neighbors updated
	CWinThread *m_pThread;    
	
	OverlayTrackersMap overlay_trackers_map;
};



#endif _NEIGHBORS_MANAGER_H

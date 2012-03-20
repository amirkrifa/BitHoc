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
* \file WMRoutingTable.cpp
* \brief This class is used to extract and update the device routing table each time it changes.
* \version 1.0 
*/

#include "StdAfx.h"
#pragma warning(disable:4786)
#include "WMRoutingTable.h"
#include <string>
#include <iostream>
#include<winsock2.h>
#include "Ping.h"
#include "socket.h"
#include <windows.h>
#include <sstream>
#include <fstream>
#include "LogFile.h"
bool BroadCastServerIsRunning=true;


WMRoutingTable::WMRoutingTable()
{
	routing_table_was_modified=false;
	this->first_time=true;
	InitializeCriticalSection(&exploration_map_sync);
	

}

WMRoutingTable::~WMRoutingTable(void)
{
	DeleteCriticalSection(&exploration_map_sync);
	BroadCastServerIsRunning=false;
	this->ssb->Close();
	delete this->ssb;
	this->rt_map.clear();
	this->exploration_map.clear();
	// Cleaning the neighbors map
	if(this->neighbors_map.size()>0)
	{
		NeighborsTS::iterator nm_iter=this->neighbors_map.begin();
		while(nm_iter!=this->neighbors_map.end())
		{
			if(nm_iter->second)
			{
				free(nm_iter->second);
			}
			nm_iter++;
		}
		this->neighbors_map.clear();
	}
}
void WMRoutingTable::SetBroadcastParam(string &a,int p,int bi,int httpport)
{
	this->broadcast_addr.assign(a);
	this->broadcast_port=p;
	this->broadcast_interval=bi;
	this->http_server_port=httpport;
}
string WMRoutingTable::get_local_ip_adr()
{

	WSADATA WSAData;
	// Initialize Winsock DLL
	::WSAStartup(MAKEWORD(2,0),&WSAData);
	

	// Get Local Hostname
	char host_name[128];
	if(::gethostname(host_name,sizeof(host_name)))
	{
		return string("");// Error Hnadling
	}

	// Get Local Ip Address
	
	struct hostent *pHost=0;
	pHost=::gethostbyname(host_name);

	if(!pHost)
	{
		return string("");//Error
	}
	string l_ip;
	l_ip.assign(inet_ntoa(*(struct in_addr*)*pHost->h_addr_list));
	WSACleanup();
 	return l_ip;
}
bool WMRoutingTable::is_it_reachable(string &ip)
{
	Ping ping;
	string pr(ping.send_a_ping((char *)ip.c_str()));
	if(pr.find("Received")!=string::npos)
		return true;
	else return false;
}

// return the broadcast address
string WMRoutingTable::get_broadcast_addr()
{
	
	std::string res; 
	PMIB_IPFORWARDTABLE pIpForwardTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	char szDestIp[128];
	char szMaskIp[128];
	

	struct in_addr IpAddr;

	int i;
	string lip_address(this->get_local_ip_adr());
	try
	{

	pIpForwardTable = (MIB_IPFORWARDTABLE*) MALLOC(sizeof(MIB_IPFORWARDTABLE));
	if (pIpForwardTable == NULL) {
		throw string("Error allocating memory ");
	}

	if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		FREE(pIpForwardTable);
		pIpForwardTable = (MIB_IPFORWARDTABLE*) MALLOC(dwSize);
    if (pIpForwardTable == NULL) {
		throw string("Error allocating memory");
    }
}

/* Note that the IPv4 addresses returned in 
 * GetIpForwardTable entries are in network byte order 
 */
	if ((dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0)) == NO_ERROR) 
	{
		for (i = 0; i < (int) pIpForwardTable->dwNumEntries; i++) {
			/* Convert IPv4 addresses to strings */
			IpAddr.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardDest;
			strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(IpAddr) );
			IpAddr.S_un.S_addr =  (u_long) pIpForwardTable->table[i].dwForwardMask;
			strcpy_s(szMaskIp, sizeof(szMaskIp), inet_ntoa(IpAddr) );
			if(lip_address.compare(szDestIp)==0)
			{// Get the mask
				res.append(szMaskIp);
				return res;
			}
		}
		FREE(pIpForwardTable);
		
	}
	else {
		throw string("\tGetIpForwardTable failed.");
		FREE(pIpForwardTable);
		
	}
}
	catch(string)
	{
		return string("Error getting the netmask");
	}
	return string("");
}


char * WMRoutingTable::get_routing_table()
{
	char temp[1024];
	std::string ptr_rt; 
	PMIB_IPFORWARDTABLE pIpForwardTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	char szDestIp[128];
	char szMaskIp[128];
	char szGatewayIp[128];

	struct in_addr IpAddr;

	int i;

	pIpForwardTable = (MIB_IPFORWARDTABLE*) MALLOC(sizeof(MIB_IPFORWARDTABLE));
	if (pIpForwardTable == NULL) {
		ptr_rt.append("Error allocating memory \r\n");
		char * rt_result=new char[ptr_rt.length()];
		strcpy(rt_result,ptr_rt.c_str());
		return rt_result;
	}

	if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		FREE(pIpForwardTable);
		pIpForwardTable = (MIB_IPFORWARDTABLE*) MALLOC(dwSize);
    if (pIpForwardTable == NULL) {
		ptr_rt.append("Error allocating memory \r\n");
		char * rt_result=new char[ptr_rt.length()];
		strcpy(rt_result,ptr_rt.c_str());
		return rt_result;
    }
}

/* Note that the IPv4 addresses returned in 
 * GetIpForwardTable entries are in network byte order 
 */
	if ((dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0)) == NO_ERROR) {
		sprintf(temp,"\tNumber of entries: %d\r\n", (int) pIpForwardTable->dwNumEntries);
		ptr_rt.append(temp);
		for (i = 0; i < (int) pIpForwardTable->dwNumEntries; i++) {
			/* Convert IPv4 addresses to strings */
			IpAddr.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardDest;
			strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(IpAddr) );
			IpAddr.S_un.S_addr =  (u_long) pIpForwardTable->table[i].dwForwardMask;
			strcpy_s(szMaskIp, sizeof(szMaskIp), inet_ntoa(IpAddr) );
			IpAddr.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardNextHop;
			strcpy_s(szGatewayIp, sizeof(szGatewayIp), inet_ntoa(IpAddr) );

	        sprintf(temp,"\r\n     Route[%d] Dest IP: %s \r\n", i, szDestIp);
		    ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] Subnet Mask: %s \r\n", i, szMaskIp);
			ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] Next Hop: %s \r\n", i, szGatewayIp);
			ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] If Index: %ld \r\n", i, pIpForwardTable->table[i].dwForwardIfIndex);
			ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] Type: %ld \r\n", i, pIpForwardTable->table[i].dwForwardType);
			ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] Proto: %ld \r\n", i, pIpForwardTable->table[i].dwForwardProto);
			ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] Age: %ld \r\n", i, pIpForwardTable->table[i].dwForwardAge);
			ptr_rt.append(temp);
			sprintf(temp,"     Route[%d] Metric1: %ld \r\n", i, pIpForwardTable->table[i].dwForwardMetric1);
			ptr_rt.append(temp);
		}
		FREE(pIpForwardTable);
		ptr_rt.append("---------------------------- \r\n");
		char * rt_result=new char[ptr_rt.length()];
		strcpy(rt_result,ptr_rt.c_str());
		return rt_result;
	}
	else {
		ptr_rt.append("\tGetIpForwardTable failed.\r\n");
		FREE(pIpForwardTable);
		char * rt_result=new char[ptr_rt.length()];
		strcpy(rt_result,ptr_rt.c_str());
		return rt_result;
	}
}

bool WMRoutingTable::update_rt_map()
{
	this->routing_table_was_modified=true;
	// Block the access to the routing table map
	this->updating=true;

	// Cleaning the past map
	if(rt_map.size()>0)
		this->rt_map.clear();
	
	// Getting the new routing table 
	
	PMIB_IPFORWARDTABLE pIpForwardTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	char szDestIp[128];
	char szMaskIp[128];
	char szGatewayIp[128];

	struct in_addr IpAddr;

	int i;

	pIpForwardTable = (MIB_IPFORWARDTABLE*) MALLOC(sizeof(MIB_IPFORWARDTABLE));
	if (pIpForwardTable == NULL) 
		{
			//Error allocating memory
			this->updating=false;
			return false;
		}

	if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) 
		{
			FREE(pIpForwardTable);
			pIpForwardTable = (MIB_IPFORWARDTABLE*) MALLOC(dwSize);
		if (pIpForwardTable == NULL) 
			{
			//Error allocating memory
			this->updating=false;
			return false;
			}
		}
		
	if ((dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0)) == NO_ERROR) 
	{
		for (i = 0; i < (int) pIpForwardTable->dwNumEntries; i++) 
		{
			/* Convert IPv4 addresses to strings */
			IpAddr.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardDest;
			strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(IpAddr) );
			
			IpAddr.S_un.S_addr =  (u_long) pIpForwardTable->table[i].dwForwardMask;
			strcpy_s(szMaskIp, sizeof(szMaskIp), inet_ntoa(IpAddr) );
			
			IpAddr.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardNextHop;
			strcpy_s(szGatewayIp, sizeof(szGatewayIp), inet_ntoa(IpAddr) );
			std::string dest_ip;
			dest_ip.assign(szDestIp);
			rt_map.insert(pair<std::string,int>(dest_ip,pIpForwardTable->table[i].dwForwardMetric1));
		}

		// Get the 1 hop neighbors
		if(this->neighbors_map.size()>0)
		{
			// Adding 1hop neighbors
			NeighborsTS::iterator nm_iter=this->neighbors_map.begin();
			i++;
			while(nm_iter!=this->neighbors_map.end())
			{
				if(rt_map.find(nm_iter->first) == rt_map.end())
					rt_map.insert(pair<std::string,int>(nm_iter->first,1));
				nm_iter++;
			}
		}
		FREE(pIpForwardTable);
		this->updating=false;
		return true;
	}
	else {
		//GetIpForwardTable failed
		FREE(pIpForwardTable);
		this->updating=false;
		return false;
	}
	
	// Updating the exploration map
	this->get_exploration_map();

	this->updating=false;
		
}

char* WMRoutingTable::get_routing_table_from_the_map()
{	
	this->update_1hop_neighbors_map();
	this->update_rt_map();
	this->routing_table_was_modified=false;
	if(!updating)
	{
		std::string rt;
		std::map<std::string,int>::iterator iter;
		iter=this->rt_map.begin();
		
		while(iter!=this->rt_map.end())
		{	
			if(this->is_valid(iter->first))
			{
				rt.append(" IP dest : ");
				rt.append(iter->first.c_str());
				rt.append(" Metric :  ");
				char temp[10];
				sprintf(temp,"%i",iter->second);
				rt.append(temp);
				rt.append("\r\n");
			}
			iter++;
		}
		if(rt_map.size()>0)
		{
			char *result=new char[rt.length()];
			strcpy(result,rt.c_str());
			
			return result;
		} else return NULL;
	} else return NULL;	

}
char* WMRoutingTable::get_html_routing_table_from_the_map()
{	
	this->update_1hop_neighbors_map();
	this->update_rt_map();
	this->routing_table_was_modified=false;
	if(!updating)
	{
		std::string rt;
		std::map<std::string,int>::iterator iter;
		iter=this->rt_map.begin();
		
		while(iter!=this->rt_map.end())
		{	if(this->is_valid(iter->first))
			{
				rt.append("<p>");
				string sLink;
				sLink.append("<a href=\"http://");
				sLink.append(iter->first.c_str());
				sLink.append(":");
				std::ostringstream oss;
				oss<<this->http_server_port;
				sLink.append(oss.str());
				sLink.append("/GetStatus\">");
				
				rt.append(sLink);
				rt.append(" IP dest : ");
				rt.append(iter->first.c_str());
				rt.append(" Metric :  ");
				char temp[10];
				sprintf(temp,"%i",iter->second);
				rt.append(temp);
				rt.append("</a>");
				rt.append("</p>");
		}
			iter++;
		}
		if(rt_map.size()>0)
		{
			char *result=new char[rt.length()];
			strcpy(result,rt.c_str());
			
			return result;
		} else return NULL;
	} else return NULL;	

}
bool WMRoutingTable::add_route_entry()
{
	return true;
}

bool WMRoutingTable::is_valid(const string &ip)
{
	
	int pp=ip.find_last_of(".");
	string last_part=ip.substr(pp+1);
	if(ip.find("192.168")!=string::npos)
		return false;
	if(last_part.compare("0")==0||last_part.compare("255")==0)
		return false;

	pp=ip.find_first_of(".");
	string first_part=ip.substr(0,pp);
	if(first_part.compare("225")==0 || first_part.compare("224")==0 ||first_part.compare("127")==0)
		return false;
	
	return true;
}
// Create the exploration map from the routing table 
// routing_matric -------> List of IP(s)
//
bool WMRoutingTable::get_exploration_map()
{
	string l_ip(this->get_local_ip_adr());

	this->enter_exp_map_cs();
	// Updating the metrics list from the routing table
	this->update_metrics_list();

	list<int>::iterator metrics_iter=this->metrics_list.begin();
	map<string,int>::iterator rt_iter;
	this->exploration_map.clear();
	map<int,list<string>>::iterator exp_iter;

	

	while(metrics_iter!=this->metrics_list.end())
	{
		rt_iter=this->rt_map.begin();
		
		while(rt_iter!=this->rt_map.end())
		{
				if(*metrics_iter==rt_iter->second)
				{
					if(is_valid(rt_iter->first) && l_ip.compare(rt_iter->first)!=0 )
					{
			
						// The same routing metric
						exp_iter=this->exploration_map.find(*metrics_iter);
						// Look if its the first element to add in a new entry
						if(exp_iter==this->exploration_map.end())
						{
							IpList ip_list;
						
							this->exploration_map.insert(make_pair(*metrics_iter,ip_list));
							this->exploration_map[*metrics_iter].push_back(rt_iter->first);
						} else 
						{
							// Adding an ip for an already existing metric entry
							// Look if the ip does not already exists
							IpList::iterator l_iter=this->exploration_map[*metrics_iter].begin();
							bool found=false;
							while(l_iter!=this->exploration_map[*metrics_iter].end())
							{
								if(*l_iter==rt_iter->first)
									{found=true;break;}
								l_iter++;
							}
								if(!found)
								{
									
									this->exploration_map[*metrics_iter].push_back(rt_iter->first);
								}	
						}
					}
				}
			
			rt_iter++;
		}
		metrics_iter++;
	}

	// Adding the 1 HOP Neighbors
	this->update_1hop_neighbors_map();
	if(this->neighbors_map.size()>0)
	{
		NeighborsTS::iterator nm_iter=this->neighbors_map.begin();
		IpList one_hop_nl;
		while(nm_iter!=this->neighbors_map.end())
		{
			one_hop_nl.push_back(nm_iter->first);
			
			nm_iter++;
		}
		this->exploration_map.insert(make_pair<int,IpList>(1,one_hop_nl));
	}
	

	this->exit_exp_map_cs();
	return true;
}

// Update the list of metrics from the routing table and sort it in increasing order
bool WMRoutingTable::update_metrics_list()
{
	this->metrics_list.clear();
	std::map<string,int>::iterator rt_iter=this->rt_map.begin();
	IntList::iterator metrics_iter;
	bool founded;
	while(rt_iter!=this->rt_map.end())
	{
		metrics_iter=this->metrics_list.begin();
		founded=false;
		while(metrics_iter!=this->metrics_list.end())
		{
			if(*metrics_iter==rt_iter->second)
				{founded=true;break;}
			metrics_iter++;
		}
		if(!founded)
			this->metrics_list.push_back(rt_iter->second);
		rt_iter++;
	}
	// sorting the metric list in increasing order
	this->metrics_list.sort();
	return true;
}

void WMRoutingTable::start_brodcast_messages_receiver(int port)
{
	b_thread_param __unaligned  *broadcast_tp=(b_thread_param __unaligned  *)malloc(sizeof(b_thread_param));
	strcpy(broadcast_tp->b_address,"\0");
	broadcast_tp->b_port=port;
	strcpy(broadcast_tp->b_message,"\0");
	broadcast_tp->ptr_wmrt=this;
	ssb=new SocketServerBroadcast();
	broadcast_tp->ssb=ssb;
	// Starting the server thread
	CWinThread * current_thread=AfxBeginThread(receive_broadcast_message,(LPVOID)broadcast_tp);
	current_thread=NULL;
}
UINT WMRoutingTable::receive_broadcast_message(LPVOID param)
{
	b_thread_param __unaligned  *broadcast_tp=(b_thread_param __unaligned  *)param;
	SocketServerBroadcast *ssb=broadcast_tp->ssb;
	char source_ip[30];
	char received_message[200];
	int numbytes=0;
	string l_ip(broadcast_tp->ptr_wmrt->get_local_ip_adr());
	ssb->bind_broadcast_socket(broadcast_tp->b_port);
	
	while(BroadCastServerIsRunning)
	{	
		strcpy(source_ip,"");
		strcpy(received_message,"");
		
		numbytes=ssb->receive_broadcast_message(source_ip,received_message);
		if(numbytes>0)
		{
			// New reveived broadcast message
			if(l_ip.compare(string(source_ip))!=0)
			{	
				// respond to the neighbor request
				string received_mess;
				received_mess.assign(received_message);
				if(received_mess.find("I am")!=string::npos)
				{
					
					// adding a new 1hop neighbor
					broadcast_tp->ptr_wmrt->add_1hop_neighbor(string(source_ip));
				
				}

			}
		}
	}

	ssb->Close();
	free(broadcast_tp);
	AfxEndThread(0);
	return 0;
}

void WMRoutingTable::look_for_1hop_neighbors()
{

	b_thread_param __unaligned  *broadcast_tp=(b_thread_param __unaligned  *)malloc(sizeof(b_thread_param));
	

	broadcast_tp->ptr_wmrt=this;
	strcpy(broadcast_tp->b_message,"I am your neighbor");
	strcat(broadcast_tp->b_message,"\0");
	strcpy(broadcast_tp->b_address,this->broadcast_addr.c_str());
	broadcast_tp->b_port=this->broadcast_port;
	broadcast_tp->b_interval=this->broadcast_interval;
	CWinThread * current_thread=AfxBeginThread(send_broadcast_message,(LPVOID)broadcast_tp);
	current_thread=NULL;
}



UINT WMRoutingTable::send_broadcast_message(LPVOID param)
{
	b_thread_param __unaligned  *broadcast_tp=(b_thread_param __unaligned  *)param;
	SocketClientBroadcast scb;
	// Keep sending broadcast hello messages each hello_interval
	while(BroadCastServerIsRunning)
	{
		scb.send_broadcast_message(broadcast_tp->b_message,broadcast_tp->b_address,broadcast_tp->b_port);
		// Waiting for the hello interval before send the next broadcast hello message
		Sleep(broadcast_tp->b_interval*1000);
	}
	scb.Close();
	free(broadcast_tp);
	AfxEndThread(0);
	return 0;
}

void WMRoutingTable::update_1hop_neighbors_map()
{
	NeighborsTS::iterator nm_iter=this->neighbors_map.begin();
	
	// Getting the local current time
	SYSTEMTIME systime={0};
	GetLocalTime(&systime);
	double c_hour=systime.wHour;
	double c_minute=systime.wMinute;
	double c_second=systime.wSecond;

	while(nm_iter!=this->neighbors_map.end())
	{
		// compare the neighbor timestamp and the local time
		if(nm_iter->second->second>c_second)
		{
			if((c_second+60)>nm_iter->second->second+3*this->broadcast_interval)
			{
				// delete the current neighbor
				nm_iter=this->neighbors_map.erase(nm_iter);
				if(nm_iter==this->neighbors_map.end())
					break;
				else continue;
			}
		}else if(nm_iter->second->second<c_second){
			if(c_second>nm_iter->second->second+3*this->broadcast_interval)
			{// delete the current neighbor
				nm_iter=this->neighbors_map.erase(nm_iter);
						if(nm_iter==this->neighbors_map.end())
					break;
				else continue;
			}
		}
		nm_iter++;
		
	}
}

void  WMRoutingTable::add_1hop_neighbor(string &ip)
{
	this->update_1hop_neighbors_map();
	// Getting the local current time
	SYSTEMTIME systime={0};
	GetLocalTime(&systime);
	double c_hour=systime.wHour;
	double c_minute=systime.wMinute;
	double c_second=systime.wSecond;

	// adding the new neighbor if it does not exist
	
	NeighborsTS::iterator nm_iter=this->neighbors_map.find(ip);
	
	if(nm_iter!=this->neighbors_map.end())
	{
		// The neighbor is already here
		// update its timestamp
		nm_iter->second->hour=c_hour;
		nm_iter->second->minute=c_minute;
		nm_iter->second->second=c_second;
	}else {
		// a new neighbor 
		time_stamp * nt=(time_stamp*)malloc(sizeof(time_stamp));
		nt->hour=c_hour;
		nt->minute=c_minute;
		nt->second=c_second;
		this->neighbors_map.insert(make_pair<string,time_stamp*>(ip,nt));
	}

}

int WMRoutingTable::GetNumberOfHops(std::string &ip)
{
	enter_exp_map_cs();
	get_exploration_map();
	
	if(exploration_map.size()>0)
	{
		// Iterators for the exploration map
		map<int,list<string>>::iterator exp_map_iter=exploration_map.begin();
		list<string>::iterator ips_iter;
			
		while((exp_map_iter!=exploration_map.end()))
		{
			ips_iter=exp_map_iter->second.begin();
			int i=0;
			while(ips_iter!=exp_map_iter->second.end())
			{
				if((*ips_iter).compare(ip)==0)
				{
					return exp_map_iter->first;
				}
				ips_iter++;
			}
			exp_map_iter++;
		}
	}
	exit_exp_map_cs();
	return -1;
}


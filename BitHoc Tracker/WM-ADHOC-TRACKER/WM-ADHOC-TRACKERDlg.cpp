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
* \file WM_ADHOC_TRACKERDlg.cpp
* \brief This file include the main tracker implementation, especially the implementation of the class TrackerClientServer used to manage the HTTP cummunications between the client and the server.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "stdafx.h"
// WAITING CONST = 1 second
#define WAITING_CONST 1000 

// TIME To WAIT BETWEEN LINES
#define WAIT_BETWEEN_LINES(x,y) (x*y)

// TIME TO WAIT BETWEEN TWO IP ADDRESSES
#define WAIT_BETWEEN_IPS 1000

// Specify if we use the tracker response internet version or the one adapted to AdHoc Networks.
int AdHocVersion=1;
// Specify if we will build a tracker overlay or we will send to all
int iUseTrackerOverlay=0;

bool isRunning=true;


#define TORRENT_FILES_LOOKUP_WAITING_TIME 120000
#define PEERS_LOOKUP_WAITING_TIME 120000


extern bool BroadCastServerIsRunning;
extern bool STOP_SPY;
#include "WM-ADHOC-TRACKER.h"
#include "WM_ADHOC_TRACKER_MENU.h"
#include "PeerInfo.h"
#include "PeersManager.h"
#include "TorrentManager.h"
#include "WMRoutingTable.h"
#include "NeighborsManager.h"
#include "WM-ADHOC-TRACKERDlg.h"
#include <ctime>
#include "socket.h"
#include "UrlHelper.h"
#include "base64.h" 
#include "BString.h"
#include "BTList.h"
#include "BTInteger.h"
#include "BTString.h"
#include "BTObject.h"
#include "BTDictionary.h"
#include "LogFile.h"
#include <malloc.h>
#include "TorrentStorageManager.h"
#include "BlackIpList.h"
#include <stdlib.h>


using namespace std;


// Boîte de dialogue CWMADHOCTRACKERDlg

CWMADHOCTRACKERDlg::CWMADHOCTRACKERDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWMADHOCTRACKERDlg::IDD, pParent)
	, Cstr_LP(_T(""))
	, Cstr_BLWT(_T(""))
	, CString_TRACKER_CLIENT_POLL_INTERVAL(_T(""))
	, BOOL_CHECK_TRACE(FALSE)
	, CString_BROADCAST_PORT(_T(""))
	, CString_BROADCAST_ADDRESS(_T(""))
	, CString_BROADCAST_INTERVAL(_T(""))
{

	tracker_started=false;
	ptr_pm= new PeersManager();
	ptr_tm= new TorrentManager(ptr_pm);
	ptr_wmrt= new WMRoutingTable();
	ptr_nm= new NeighborsManager(ptr_wmrt);
	ptr_tsm = new TorrentStorageManager();
	
}
CWMADHOCTRACKERDlg::~CWMADHOCTRACKERDlg()
{
	BroadCastServerIsRunning = false;
	STOP_SPY = true;
	isRunning = false;

	if(ptr_nm != NULL)
		delete ptr_nm;
	if(ptr_wmrt != NULL)
		delete ptr_wmrt;
	if(this->ptr_tm != NULL)
		delete ptr_tm;
	if(ptr_pm != NULL)
		delete ptr_pm;
	if(ptr_tcs != NULL)
		delete ptr_tcs;
	if(TPCThread != NULL)
		TPCThread = NULL;
	if(ptr_tsm != NULL)
		delete ptr_tsm;
	// Close LogFiles
	if(BOOL_CHECK_TRACE)
	{	
		delete sync_requests;
		delete announce_lf;
		delete t2t_get_peers_log;
		delete error_in_response_encoding;
	}

}

int convert_string_to_int(const std::string &str) 
{
		std::string m_value = str;
		int m_val = 0;
		for (size_t i = 0; i < str.size(); i++)
		{
			m_val = m_val * 10 + (str[i] - 48);
		}
	return m_val;
}
	
string CWMADHOCTRACKERDlg::toStlString(const CString& aString)
{
    wchar_t *tmpStr =  (wchar_t *)malloc(sizeof( wchar_t)*(aString.GetLength() + 1));    
    char * tmpChar=(char *)malloc(sizeof( char)*(aString.GetLength() + 1));
	_tcscpy(tmpStr,aString);
    wcstombs(tmpChar,tmpStr,aString.GetLength() + 1);
	string myString(tmpChar);
    free(tmpStr);
	free(tmpChar);
    return myString;
}

void Get(TrackerClientServer::http_request* r) 
{
  
	r->view->ptr_tm->DeleteOutOfDatePeers();
	std::string tracker_response;
	// The peer  id
	string peer_id(r->params_["peer_id"].c_str());
	// The peer ip @
	string peer_ip(r->params_["ip"]);

	if(peer_ip.empty())
	{
		// Setting the local ip @
		peer_ip.assign(r->view->ptr_wmrt->get_local_ip_adr());
	}

	// The number of wanted peers
	int nwant=convert_string_to_int(r->params_["numwant"]);
	// The peer'tracker communication port
	int peer_port=convert_string_to_int(r->params_["port"]);
	// The torrent id
	string torrent_id(r->params_["info_hash"].c_str());
	bool isSeeder=((convert_string_to_int(r->params_["left"])==0)&&(r->params_["event"].compare("completed")==0));
	
	if(r->path_ == "/PublishTorrent") 
	{
		// Local Request coming from the .Net application 
		string file_name;
		string file_description;
		// The torrent file size in bytes
		unsigned int file_size = 0;
		file_name.assign(r->params_["file_name"]);
		file_description.assign(r->params_["file_description"]);
		file_size = atol(r->params_["file_size"].c_str());
		// Adding the peer to the peers manager.
		string local_ip = r->view->ptr_wmrt->get_local_ip_adr();
		// Adding a new local torrent file
		if(local_ip.length() > 0 && file_name.length() > 0 && file_size > 0)
			r->view->ptr_tsm->AddNewTorrentFile(local_ip, file_name, file_description, file_size);
		// Forwarding the Info to the other trackers
		
		// Create the black list of Ips, and forward the publishing act to other trackers
		/*
		BlackIpList * bl = new BlackIpList();
		bl->InitBlackList(r->params_["black_list"]);	
		r->view->ptr_nm->send_locally_stored_file(file_name,file_size,local_ip,r->view->get_tracker_port(), bl);
		bl = NULL;
		*/

		// Editing the response message
		tracker_response.append("Dont Respond");
	}
	else if(r->path_ == "/NewTorrent") 
	{
		// Remote request coming from another traker, used to annouce the publishing of a new torrent file
		string file_name;
		string file_description;
		string ip_adr;
		unsigned int file_size = 0;
		file_name = r->params_["file_name"];
		file_description.assign(r->params_["file_description"]);
		ip_adr = r->params_["ip"];
		file_size = atol(r->params_["file_size"].c_str());
		// Adding a new remote stored torrent file
		if(ip_adr.length() > 0 && file_name.length() > 0 && file_size > 0)
			r->view->ptr_tsm->AddNewTorrentFile(ip_adr, file_name, file_description, file_size);
		// Editing the response message
		tracker_response.append("Dont Respond");
	} 
	else if(r->path_ == "/GetListOfPublishedTorrents") 
	{
		// Sending the list of published torrents
		string list;
		r->view->ptr_tsm->GetTheListOfPublishedTorrents(list, r->view->ptr_tm, peer_id);
		// Editing the response message
		tracker_response.append(list);

	}
	else if(r->path_ == "/GetTorrentOwner") 
	{
		// Get the owner of a given torrent
		int numberOfNodes = 0;
		string fileName;
		string ownerNode = r->view->ptr_tsm->GetFileNode(r->params_["file_name"], numberOfNodes, fileName, r->view->ptr_tm, peer_id, (r->params_["black_list"].compare("local") == 0));
		
		
		if(numberOfNodes == 0)
		{
			// We dont find infos in the local database, so we will forward the request
			
			BlackIpList bl;
			if(r->params_["black_list"].compare("local") != 0)
				bl.InitBlackList(r->params_["black_list"]);
			else bl.InitBlackList(string(""));
		
			r->view->ptr_tcs->set_tf_ss(r->params_["file_name"],true);
			
			// Forwarding the request to the other known trackers
			r->view->ptr_tcs->send_t2t_torrent_file_request(r->params_["file_name"], r->view->get_tracker_port(),"", false, r->view, peer_ip, &bl);
			
			// Recheck for the file
			ownerNode = r->view->ptr_tsm->GetFileNode(r->params_["file_name"], numberOfNodes, fileName, r->view->ptr_tm, peer_id, (r->params_["black_list"].compare("local") == 0));
		}

		// If the request is coming from another tracker of the overlay
		// Verify if we have more informations to forward to the remote tracker about the 
		// torrent he is looking for.
		
		string interested_peers;
		int ni;
		if(r->params_["black_list"].compare("local") != 0)
		{
			r->singleLineAnswer = false;

			interested_peers.assign(r->view->ptr_tm->get_list_of_interested_peers(r->view->ptr_tm->GetTorrentInfoHash(fileName),50,&ni,peer_ip));
		
			if(ni>0)
			{	
				//!Updating the trackers overlay.
				if(iUseTrackerOverlay)
					r->view->ptr_nm->add_tracker(peer_ip,peer_port);
				
				// Interrested peers
				string tmp;
				tmp.append(interested_peers);
				tmp.append("|");
				// File Name
				tmp.append(fileName);
				tmp.append("|");
				// Info_hash
				tmp.append(r->view->ptr_tm->GetTorrentInfoHash(fileName));

				
				r->answers_[r->numberOfAnswerLines].assign(tmp);
				r->numberOfAnswerLines++;
				
			}else {
				// We dont find any sharing information related to the requested file
			}
			
			// The last sent message should be always the owner node
			// The scheduling of messages is important
			r->answers_[r->numberOfAnswerLines].assign(ownerNode);
			r->numberOfAnswerLines++;

		}else
		{
			// The request is comming from the local TorrentF application
			// The answer is a single line
			// format: node1Ip#file1*file1.size*file1.description*file1.numberofleechers*file1.numberofseeders \r\n
			tracker_response.append(ownerNode);
		}
		
			
	}else if(r->path_ == "/GetLocalInfos")
	{
		
		string infos;
		r->view->ptr_tsm->GetTheListOfPublishedTorrentsJustForInfo(infos);
		tracker_response.append(infos);
	}
	else if(r->path_ == "/GetStatus") 
	{
	
		tracker_response.append ("<html>");
		tracker_response.append ("<head>");
		tracker_response.append ("<title>");
		tracker_response.append ("Status of the node : ");
		tracker_response.append(r->view->ptr_wmrt->get_local_ip_adr());
		tracker_response.append ("</title>");
		tracker_response.append ("</head>");
		tracker_response.append ("<body>");
	
		tracker_response.append ("<p align=\"center\" class=\"style1\">");
		tracker_response.append ("<BIG>");
		tracker_response.append ("<FONTSIZE=+30>");
		tracker_response.append ("STATUS OF THE NODE : ");
		tracker_response.append(r->view->ptr_wmrt->get_local_ip_adr());
		tracker_response.append ("</FONT>");
		tracker_response.append ("</BIG>");
		tracker_response.append ("</p>");
	
		tracker_response.append ("<p TEXT=\"#0000FF\">");
	
		r->view->ptr_wmrt->get_exploration_map();
		int nbr_entries=r->view->ptr_wmrt->GetNumberOfKnownTrackers();
		tracker_response.append ("<B>");
		tracker_response.append ("<U>");
		tracker_response.append (" Routing Table: ");
		tracker_response.append ("</B>");
		tracker_response.append ("</U>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");

		if(nbr_entries>0)
		{
			tracker_response.append ("<p>");
			tracker_response.append(r->view->ptr_wmrt->get_html_routing_table_from_the_map());
			tracker_response.append ("</p>");
		}
		else 
		{
			tracker_response.append ("<p>");	
			tracker_response.append("Sorry it is Empty for the moment :)");
			tracker_response.append ("</p>");
		}
		// Internal Parameters
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("<B>");
		tracker_response.append ("<U>");
		tracker_response.append ("             Parameters:");
		tracker_response.append ("</B>");
		tracker_response.append ("</U>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append (" Broadcast @: "+r->view->broadcast_addr);
		tracker_response.append ("</p>");

		tracker_response.append ("<p>");
		tracker_response.append (" Broadcast port: ");
		std::ostringstream oss;
		oss<<r->view->broadcast_port;
		tracker_response.append (oss.str());
		tracker_response.append ("</p>");
	
		tracker_response.append ("<p>");
		tracker_response.append (" Broadcast interval: ");
		std::ostringstream oss2;
		oss2<<r->view->broadcast_interval;
		tracker_response.append (oss2.str()+" (s)");
		tracker_response.append ("</p>");
	
		tracker_response.append ("<p>");
		tracker_response.append (" Tracker client poll interval: ");
		std::ostringstream oss3;
		oss3<<r->view->tracker_client_poll_interval;
		tracker_response.append (oss3.str()+" (s)\n");
		tracker_response.append ("</p>");
	

		// List of Known Peers
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("<B>");
		tracker_response.append ("<U>");
		tracker_response.append ("             List of peers:");
		tracker_response.append ("</B>");
		tracker_response.append ("</U>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		string sLp(r->view->ptr_pm->get_html_peers_list());
		if(!sLp.empty())
		{	
			tracker_response.append ("<p>");
			tracker_response.append (sLp);
			tracker_response.append ("</p>");
		}
		else {
			tracker_response.append ("<p>");
			tracker_response.append("Number of peers = 0.\n");
			tracker_response.append ("</p>");
		}
	
		// List of Known Torrents
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("<B>");
		tracker_response.append ("<U>");
		tracker_response.append ("             List of torrents:");
		tracker_response.append ("</B>");
		tracker_response.append ("</U>");
		tracker_response.append ("</p>");
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		string sLt(r->view->ptr_tm->get_html_torrents_list());
		if(!sLt.empty())
		{
			tracker_response.append ("<p>");
			tracker_response.append (sLt);
			tracker_response.append ("</p>");
		}
		else {
			tracker_response.append ("<p>");
			tracker_response.append("Number of torrents = 0.");
			tracker_response.append ("</p>");
		}
		tracker_response.append ("<p>");
		tracker_response.append ("</p>");
		tracker_response.append ("</body>");
		tracker_response.append ("</html>");
	}else if(r->path_ == "/synchronize")
	{
		// Synchronisation request received from the bittorrent client
		
		// Create the black list of Ips
		BlackIpList *bl = new BlackIpList();
		bl->InitBlackList(r->params_["black_list"]);	

		r->view->ptr_nm->send_sync_request_to_all(r->view->get_tracker_port(),torrent_id, bl);
		bl = NULL;
		if(r->view->BOOL_CHECK_TRACE)
			r->view->sync_requests->add_line(string("Sync forwarded for torrent :"+torrent_id),true);

	}else if(r->path_=="/sync-forward")
	{
		// we have to inform the Bittorrent client about the synchronisation request
		if(r->view->BOOL_CHECK_TRACE)
			r->view->sync_requests->add_line(string("Sync requested for torrent :"+torrent_id),true);
		SocketClient out(string("127.0.0.1"),2628);
		string message(torrent_id);
		out.SendLine(message);
		out.Close();
		if(r->view->BOOL_CHECK_TRACE)
			r->view->sync_requests->add_line(string("Sync request for torrent :"+torrent_id+" Forwarded to the Bittorrent Client"),true);

	}
	else if(r->path_ == "/t2t-info")
	{
		//! Reciving info about peers i'm interested in from other neighbors Tracker.
		//! the message is in the form of dict describing the list of peers interested in a specific torrent.
		string peers;
		peers.assign(r->params_["peers"]);
		//! Parsing the bencoded message.
		BString bs;
		bs.read_buf(peers.c_str(),peers.length());
	
		BTList * lp= dynamic_cast<BTList *>(bs.GetBTObject("peers"));
		if(lp)
		{
			//!peers found.
			for(btobject_v::iterator it=lp->GetList().begin();it!=lp->GetList().end();it++)
			{
					BTDictionary *p=dynamic_cast<BTDictionary *>(*it);
					string ip;
					string id;
					int port=0;
					int uploaded=0;
					int downloaded=0;
					int left=0;
					string ev;
					if(p)
						{
						//!Dictionary found.
						for(btmap_t::iterator it=p->GetMap().begin();it!=p->GetMap().end();it++)
							{	
								string key(it->first);
								BTObject *p0=it->second;
								if(key.compare("ip")==0)
								{	
								BTString *p=dynamic_cast<BTString *>(p0);
								if(p)
									{
										ip.assign(p->GetValue());
									}
								}else if(key.compare("port")==0)
										{
											BTInteger *p=dynamic_cast<BTInteger*>(p0);
											if(p)
											{
												port=atoi(p->GetValue().c_str());
											}
									}else if(key.compare("peer id")==0)
										{
											BTString *p=dynamic_cast<BTString *>(p0);
											if(p)
											{
												id.assign(p->GetValue());
											}
										}else if(key.compare("uploaded")==0)
										{
											BTInteger *p=dynamic_cast<BTInteger *>(p0);
											if(p)
											{
												uploaded=atoi(p->GetValue().c_str());
											}
										}else if(key.compare("downloaded")==0)
										{
											BTInteger *p=dynamic_cast<BTInteger *>(p0);
											if(p)
											{
												downloaded=atoi(p->GetValue().c_str());
											}
										}else if(key.compare("left")==0)
										{
											BTInteger *p=dynamic_cast<BTInteger *>(p0);
											if(p)
											{
												left=atoi(p->GetValue().c_str());
											}
										}else if(key.compare("event")==0)
										{
											BTString *p=dynamic_cast<BTString *>(p0);
											if(p)
											{
												ev.assign(p->GetValue());
											}
										}
						}
						  
							//!Adding the peer interested in tp->info_hash.
							if(id.size()==20 && port>0 && ip.size()>0 && torrent_id.size()==20)
								{
									r->view->ptr_tm->eliminate_confusion(torrent_id,peer_id,peer_ip);
									//!Adding the peer to the peers manager.
									PeerInfo *pPeer=r->view->ptr_pm->get_peer_info(id,ip);
									if(NULL==pPeer){
										//!Creating a new peer based on the recived info.
										r->view->ptr_pm->add_peer_2(id,ip,port,nwant,r->view->ptr_wmrt->GetNumberOfHops(ip),r->view->tracker_client_poll_interval);
									}else {
										//!Updating the peer status.
										r->view->ptr_pm->update_peer(id,ip,port,nwant);
										pPeer=NULL;
									}
									//!Adding the peer to torrents manager.
									r->view->ptr_tm->add_peer_to_torrent((char *)torrent_id.c_str(),string(""),(char *) id.c_str());
								
									//!Adding the torrent of interest.
									r->view->ptr_pm->add_torrent_of_interest(id,torrent_id,uploaded,downloaded,left,ev,ip);
								}
					}
			}

			//!Sending  the info about the list of peers we received by the /t2t-info request to other trackers which belongs to the overlay.
			if( iUseTrackerOverlay && r->view->ptr_nm->get_trackers_overlay_size()>0)
			{
				string peer_dict;
				peer_dict.append(peers);
				
				BlackIpList  * bl = new BlackIpList();
				bl->InitBlackList(r->params_["black_list"]);
				r->view->ptr_nm->send_peers_interested_in(torrent_id,peer_dict,peer_ip,r->view->get_tracker_port(),bl);
				bl = NULL;
			}
	
		}
	}
	else if(r->path_== "/announce")
	{	  
	  
		bool new_torrent_file;
		int number_of_seeders=0;
		int number_of_leechers=0;
		std::string interested_peers;
		PeerInfo *pi;
		int new_peer=0;
	  
		if(r->view->BOOL_CHECK_TRACE)
		{
			r->view->announce_lf->add_line(string(" /announce request received from: ")+peer_ip,true);
			r->view->announce_lf->add_line(string("      /announce info_hash: ")+r->params_["info_hash"],false);
			r->view->announce_lf->add_line(string("      /announce left: ")+r->params_["left"],false);
			r->view->announce_lf->add_line(string("      /announce downloaded: ")+r->params_["downloaded"],false);
			r->view->announce_lf->add_line(string("      /announce uploaded: ")+r->params_["uploaded"],false);
			r->view->announce_lf->add_line(string("      /announce event: ")+r->params_["event"],false);
		}
	  
		// Eliminate Confusion
	  	r->view->ptr_tm->eliminate_confusion(torrent_id,peer_id,peer_ip);
	  
		//!Look for the peer if it is already in the data base or not.
		pi=r->view->ptr_pm->get_peer_info(peer_id,peer_ip);
	  
		if(pi==NULL){
			//!Creating a new peer based on the recived info.
			pi=r->view->ptr_pm->add_peer_2(peer_id,peer_ip,peer_port,nwant,r->view->ptr_wmrt->GetNumberOfHops(peer_ip),r->view->tracker_client_poll_interval);
			new_peer=1;
		}else {
			//!Updating the peer status.
			r->view->ptr_pm->update_peer(peer_id,peer_ip,peer_port,nwant);
		}
	  
	  
		//!Check the info hash.
		int info_hash_test=r->view->ptr_tm->check_info_hash(torrent_id,peer_id);
	  
	  
		if(info_hash_test==0 && (r->view->ptr_nm->get_trackers_overlay_size()==0&&iUseTrackerOverlay) )
		{ //!New torrent and we cannot find any tracker in the overlay.
			if(!isSeeder)
			{ //!If the peer is a seeder ---> no need for a Lookup Proccess.
		  
				//!Initialize the search status for the current torrent.
				new_torrent_file=false;
				//!Starting the lookup loop.
				r->view->ptr_tcs->set_ss(torrent_id,true);
				// Create The black list
				BlackIpList bl;
				bl.InitBlackList(r->params_["black_list"]);
				r->view->ptr_tcs->send_t2t_request(string(torrent_id),r->view->get_tracker_port(),"",false,r->view, peer_ip, &bl);
			
				//!check the search results.
				info_hash_test=r->view->ptr_tm->check_info_hash(torrent_id,peer_id);

				if(info_hash_test==1) new_torrent_file=true;
			}
		}else if(info_hash_test==0 && (r->view->ptr_nm->get_trackers_overlay_size()>0 && iUseTrackerOverlay))
		{
			//!New torrent but we have at least one tracker in the overlay.
			//!So just we have to ask this tracker for informations.
			if(!isSeeder)
			{ //!If it is a seeder ---> no need for a Lookup Proccess 
		  
				//!Initialize the search status for the current torrent. 
				new_torrent_file=false;
				//! Starting the lookup loop.
				r->view->ptr_tcs->set_ss(torrent_id,true);

				BlackIpList bl;
				bl.InitBlackList(r->params_["black_list"]);
				r->view->ptr_tcs->send_t2t_request(string(torrent_id),r->view->get_tracker_port(),"",true,r->view,peer_ip, &bl);
		  
				//!check the search results.
				info_hash_test=r->view->ptr_tm->check_info_hash(torrent_id,peer_id);
			

				if(info_hash_test==1) new_torrent_file=true;
				else {
						//!The trackers in the overlay does not have the information.
						//!Initialize the search status for the current torrent. 
						new_torrent_file=false;
						//!Starting the lookup loop.
						r->view->ptr_tcs->set_ss(torrent_id,true);
				
						BlackIpList bl;
						bl.InitBlackList(r->params_["black_list"]);

						r->view->ptr_tcs->send_t2t_request(string(torrent_id),r->view->get_tracker_port(),"",false,r->view,peer_ip,&bl);
		  				//!check the search results.
						info_hash_test=r->view->ptr_tm->check_info_hash(torrent_id,peer_id);


						if(info_hash_test==1) new_torrent_file=true;
				}
			}
		}else if(!iUseTrackerOverlay)
		{	  // Just Get a look in our databases
			if(!isSeeder)
			{ 	
				new_torrent_file=false;
				//!check the search results.
				info_hash_test=r->view->ptr_tm->check_info_hash(torrent_id,peer_id);
				if(info_hash_test==1) new_torrent_file=true;
				else {
						//! Nothing in our databases
						//!Initialize the search status for the current torrent. 
						//!Starting the lookup loop.
						r->view->ptr_tcs->set_ss(torrent_id,true);
		
						BlackIpList bl;
						bl.InitBlackList(r->params_["black_list"]);

						r->view->ptr_tcs->send_t2t_request(string(torrent_id),r->view->get_tracker_port(),"",false,r->view,peer_ip, &bl);
		  				//!check the search results.
						info_hash_test=r->view->ptr_tm->check_info_hash(torrent_id,peer_id);


						if(info_hash_test==1) new_torrent_file=true;
				}
			
			}
		}
	  
		//!Adding or updating the new peer information. 
		if( info_hash_test==1 ||info_hash_test==0 )
		{
			//!Adding the torrent to the TorrentManager.
			r->view->ptr_tm->add_peer_to_torrent((char *)torrent_id.c_str(), r->params_["file_name"],(char *)peer_id.c_str());
			r->view->ptr_pm->add_torrent_of_interest(peer_id,torrent_id,convert_string_to_int(r->params_["uploaded"]),convert_string_to_int(r->params_["downloaded"]),convert_string_to_int(r->params_["left"]),r->params_["event"],peer_ip);
		}
	  
		  	
		 if( info_hash_test==1 )
		{
			//!Get the number of seeders and leechers if we have informations about the torrent.

			number_of_seeders=r->view->ptr_tm->get_number_of_ppers_with_entire_file(torrent_id,peer_id);
			number_of_leechers=r->view->ptr_tm->get_number_of_non_seeder_peers(torrent_id,peer_id);
		
			//!Sending the bencoded response.
			tracker_response.append("d");
			//!interval in seconds that the client should wait between sending regular.
			//!requests to the tracker.
			tracker_response.append("8:interval");
			tracker_response.append("i");
			char interval[100];
			sprintf(interval,"%i",r->view->tracker_client_poll_interval);
			tracker_response.append(interval);
			tracker_response.append("e");
	  
			//!Number of peers with the entire file.
			tracker_response.append("8:complete");
			char seeders[100];
			sprintf(seeders,"%i",number_of_seeders);
			tracker_response.append("i");
			tracker_response.append(seeders);
			tracker_response.append("e");

			//! Number of peers with the incomplete file.
			tracker_response.append("10:incomplete");
			char leechers[100];
			sprintf(leechers,"%i",number_of_leechers);
			tracker_response.append("i");
			tracker_response.append(leechers);
			tracker_response.append("e");
	  
			//! The list of the interested peers.
			//! Get the list of interested peers in the torrent.
			int ni;
			interested_peers=r->view->ptr_tm->get_limited_list_of_interested_peers(r->view->ptr_wmrt->get_local_ip_adr(),torrent_id,nwant,&ni);
			tracker_response.append( interested_peers);
	   
			tracker_response.append("e");
		  
		}
		else if (info_hash_test == -1 )
		{
			//!Incorrect info_hash.
			tracker_response=r->view->ptr_tm->get_bencoded_fail_message(string("Invalid info_hash"));
		}else if(info_hash_test==0 && new_torrent_file==false )
		{	//!The search process does not result in any information about the torrent the peer is asking for.
			tracker_response=r->view->ptr_tm->get_bencoded_fail_message(string("I cannot find another tracker that knows the torrent: ").append(torrent_id));
		}

		//!Sending the info about the peer that sends the /announce request to other trackers which belongs to the overlay.
		if((r->view->ptr_nm->get_trackers_overlay_size()>0 && iUseTrackerOverlay) || !iUseTrackerOverlay && pi!=NULL)
		{
			string peer_dict;
			peer_dict.append("d");
			peer_dict.append("5:peers");
			peer_dict.append("l");
			peer_dict.append(pi->get_bencoded_info(torrent_id));
			peer_dict.append("e");
			peer_dict.append("e");

			BlackIpList *bl = new BlackIpList();
			bl->InitBlackList(r->params_["black_list"]);
			r->view->ptr_nm->send_peers_interested_in(torrent_id,peer_dict,peer_ip,r->view->get_tracker_port(), bl);
			bl = NULL;
		 }

		pi=NULL;
	  
		if(r->view->BOOL_CHECK_TRACE)
		{
			r->view->announce_lf->add_line(string(" Answer to : ")+peer_ip,false);
			r->view->announce_lf->add_line(string("Answer : ")+tracker_response,false);
		}
	  
	}
	else if(r->path_== "/t2t-get-peers")
	{
		//!Returning the list of the peers interested in a specific torrent.
	
		if(r->view->BOOL_CHECK_TRACE)
		{
			r->view->t2t_get_peers_log->add_line(string(" A request /t2t-get-peers was received from :")+r->params_["ip"],true);
			r->view->t2t_get_peers_log->add_line(string("      /t2t-get-peers info_hash:")+r->params_["info_hash"],false);
			r->view->t2t_get_peers_log->add_line(string(" Answer to:")+r->params_["ip"],false);
		}
		string interested_peers;
		int ni;
		interested_peers.assign(r->view->ptr_tm->get_list_of_interested_peers(r->params_["info_hash"],50,&ni,peer_ip));
		if(ni>0)
		{	//!Updating the trackers overlay.
			if(iUseTrackerOverlay)
				r->view->ptr_nm->add_tracker(peer_ip,peer_port);
			tracker_response.append(interested_peers.c_str());	
			if(r->view->BOOL_CHECK_TRACE)
			{
				r->view->t2t_get_peers_log->add_line(interested_peers,false);
			}
			//!Add the node as a tracker neighbor.
		}else {
			tracker_response.append("not interested in ");
			tracker_response.append(r->params_["info_hash"]);
			if(r->view->BOOL_CHECK_TRACE)
			{
				r->view->t2t_get_peers_log->add_line(string(" not interested in ")+r->params_["info_hash"],false);
			}
		}

 }
	else {
    
		//!Sending a bencoded message saying that the announce url is incorrect.
		tracker_response= r->view->ptr_tm->get_bencoded_fail_message(string("Incorrect Announce URL"));
	}

if(r->path_ == "/t2t-info" && r->singleLineAnswer)
	r->answer_=string("Dont Respond");
else if(r->singleLineAnswer)
	r->answer_=tracker_response;

if(r->path_ != "/GetStatus" && r->path_ != "/PublishTorrent" && r->path_ != "/NewTorrent" && r->path_ !="/GetListOfPublishedTorrents" && r->path_ !="/GetTorrentOwner" && r->path_ !="/GetLocalInfos")
{
	try{
		  if(r->answer_.find("not interested")==string::npos)
		  {
			BString bs;
			bs.read_buf(tracker_response.c_str(),tracker_response.length());
		  }
	  }
	catch(string e)
	  {
		  if(r->view->BOOL_CHECK_TRACE)
		  {
		 	r->view->error_in_response_encoding->add_line(string("Error in the encoding of the following response: ")+tracker_response,true);
		  }
	}
}
  

}


void CWMADHOCTRACKERDlg::show_message(string msg)
{
	string temp;
	temp.append("-----------------------------\r\n");
	temp.append(msg);
	UpdateData(FALSE);
}
	
UINT CWMADHOCTRACKERDlg::TrackerPeerConnectionThreadLauncher(LPVOID p)
{
	tracker_peer_connectio_thread_param * obj=(tracker_peer_connectio_thread_param* )(p);
	CWMADHOCTRACKERDlg * papa=obj->parent;
	int temp1=obj->blwt;
	WMRoutingTable * temp2=papa->ptr_wmrt;
	papa->ptr_tcs= new TrackerClientServer(obj->server_port,Get,papa,temp2,temp1);
	AfxEndThread(0);
	return 0;
	
}
	
void CWMADHOCTRACKERDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LP, Cstr_LP);
	DDX_Control(pDX, IDC_BUTTON_CAS, Cbutton_Configure_and_Start);
	DDX_Control(pDX, IDC_EDIT_LP, CEdit_lp);
	DDX_Text(pDX, IDC_EDIT_PI, CString_TRACKER_CLIENT_POLL_INTERVAL);
	DDX_Control(pDX, IDC_EDIT_PI, CEdit_TRACKER_CLIENT_POLL_INTERVAL);
	DDX_Check(pDX, IDC_CHECK_TRACE, BOOL_CHECK_TRACE);
	DDX_Text(pDX, IDC_EDIT_BROADCAST_PORT, CString_BROADCAST_PORT);
	DDX_Control(pDX, IDC_EDIT_BROADCAST_PORT, CEdit_BROADCAST_PORT);
	DDX_Control(pDX, IDC_EDIT_BROADCAST_ADDRESS, CEdit_BROADCAST_ADDRESS);
	DDX_Text(pDX, IDC_EDIT_BROADCAST_ADDRESS, CString_BROADCAST_ADDRESS);
	DDX_Text(pDX, IDC_EDIT_BI, CString_BROADCAST_INTERVAL);
	DDX_Control(pDX, IDC_EDIT_BI, CEdit_BI);
}

BEGIN_MESSAGE_MAP(CWMADHOCTRACKERDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CAS, &CWMADHOCTRACKERDlg::OnBnClickedButtonCas)
	//ON_EN_CHANGE(IDC_EDIT_BLWT, &CWMADHOCTRACKERDlg::OnEnChangeEditBlwt)
	ON_EN_CHANGE(IDC_EDIT_LP, &CWMADHOCTRACKERDlg::OnEnChangeEditLp)
	ON_EN_UPDATE(IDC_EDIT_LP, &CWMADHOCTRACKERDlg::OnEnUpdateEditLp)
	//ON_EN_UPDATE(IDC_EDIT_BLWT, &CWMADHOCTRACKERDlg::OnEnUpdateEditBlwt)
	ON_EN_SETFOCUS(IDC_EDIT_LP, &CWMADHOCTRACKERDlg::OnEnSetfocusEditLp)
	//ON_EN_SETFOCUS(IDC_EDIT_BLWT, &CWMADHOCTRACKERDlg::OnEnSetfocusEditBlwt)
	ON_BN_CLICKED(IDC_BUTTON_TRACE_MENU, &CWMADHOCTRACKERDlg::OnBnClickedButtonTraceMenu)
	ON_BN_CLICKED(IDC_CHECK_TRACE, &CWMADHOCTRACKERDlg::OnBnClickedCheckTrace)
	ON_BN_KILLFOCUS(IDC_CHECK_TRACE, &CWMADHOCTRACKERDlg::OnBnKillfocusCheckTrace)
	ON_EN_UPDATE(IDC_EDIT_PI, &CWMADHOCTRACKERDlg::OnEnUpdateEditPi)
	ON_EN_SETFOCUS(IDC_EDIT_PI, &CWMADHOCTRACKERDlg::OnEnSetfocusEditPi)
	ON_EN_CHANGE(IDC_EDIT_BROADCAST_PORT, &CWMADHOCTRACKERDlg::OnEnChangeEditBroadcastPort)
	ON_EN_SETFOCUS(IDC_EDIT_BROADCAST_PORT, &CWMADHOCTRACKERDlg::OnEnSetfocusEditBroadcastPort)
	ON_EN_UPDATE(IDC_EDIT_BROADCAST_PORT, &CWMADHOCTRACKERDlg::OnEnUpdateEditBroadcastPort)
	ON_EN_CHANGE(IDC_EDIT_BROADCAST_ADDRESS, &CWMADHOCTRACKERDlg::OnEnChangeEditBroadcastAddress)
	ON_EN_SETFOCUS(IDC_EDIT_BROADCAST_ADDRESS, &CWMADHOCTRACKERDlg::OnEnSetfocusEditBroadcastAddress)
	ON_EN_UPDATE(IDC_EDIT_BROADCAST_ADDRESS, &CWMADHOCTRACKERDlg::OnEnUpdateEditBroadcastAddress)
END_MESSAGE_MAP()


// Gestionnaires de messages pour CWMADHOCTRACKERDlg

BOOL CWMADHOCTRACKERDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Définir l'icône de cette boîte de dialogue. L'infrastructure effectue cela automatiquement
	//  lorsque la fenêtre principale de l'application n'est pas une boîte de dialogue
	SetIcon(m_hIcon, TRUE);			// Définir une grande icône
	SetIcon(m_hIcon, FALSE);		// Définir une petite icône

	// TODO : ajoutez ici une initialisation supplémentaire
	// Desactiver le boutton Configure and Start until filling all the parameter
	GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_TRACE_MENU)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_PI)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_PI)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_TRACE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_BROADCAST_PORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_BP)->EnableWindow(TRUE);
	
	// Init the Configuration Dialog
	SetDlgItemInt(IDC_EDIT_BROADCAST_PORT,2618,false);
	SetDlgItemInt(IDC_EDIT_BI,9,false);
	SetDlgItemInt(IDC_EDIT_LP,2617,false);
	//SetDlgItemInt(IDC_EDIT_BLWT,1,false);
	SetDlgItemInt(IDC_EDIT_PI,11,false);
	SetDlgItemText(IDC_EDIT_BROADCAST_ADDRESS,TEXT("255.255.255.255"));
	
	
	return TRUE;  // retourne TRUE, sauf si vous avez défini le focus sur un contrôle
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CWMADHOCTRACKERDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	DRA::RelayoutDialog(
		AfxGetInstanceHandle(), 
		this->m_hWnd, 
		DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_WMADHOCTRACKER_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_WMADHOCTRACKER_DIALOG));
}
#endif

// Cstring convertion

template<typename T,typename S>
bool FromString( const S & Str, T & Dest )
{
#ifdef _UNICODE
    std::wistringstream iss( Str );
#else
    std::istringstream iss( Str );
#endif
    // tenter la conversion vers Dest
    return iss >> Dest != 0;
}

void CWMADHOCTRACKERDlg::OnSysCommand(UINT nID,LPARAM lParam)
{
	if(nID==SC_CLOSE)
	{	BroadCastServerIsRunning=false;
		STOP_SPY=true;
		isRunning=false;
		if(ptr_nm!=NULL)
			delete ptr_nm;
		if(ptr_wmrt!=NULL)
			delete ptr_wmrt;
		if(this->ptr_tm!=NULL)
			delete ptr_tm;
		if(ptr_pm!=NULL)
			delete ptr_pm;
		if(ptr_tcs!=NULL)
			delete ptr_tcs;
		if(TPCThread!=NULL)
			TPCThread=NULL;
		// Close LogFiles
		if(BOOL_CHECK_TRACE)
		{	
			delete announce_lf;
			delete t2t_get_peers_log;
			delete error_in_response_encoding;
		}
	}else
	{
	
		CWMADHOCTRACKERDlg::OnSysCommand(nID,lParam);
	}
}
void CWMADHOCTRACKERDlg::OnBnClickedButtonCas()
{
	// TODO: Add your control notification handler code here
	//Calling The TRACE Dialog
	
	// Getting the tracker port and between hops waiting time
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty()&& !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty())
	{
		FromString(Cstr_LP.GetString(), this->tracker_port );
		//FromString(Cstr_BLWT.GetString(), this->blwt);
		FromString(CString_TRACKER_CLIENT_POLL_INTERVAL.GetString(), this->tracker_client_poll_interval);
		FromString(CString_BROADCAST_PORT.GetString(), this->broadcast_port);
		FromString(CString_BROADCAST_INTERVAL.GetString(), this->broadcast_interval);
		
		this->broadcast_addr.assign(toStlString(CString_BROADCAST_ADDRESS));
		
		
	
		
		// Open Log Files
		 if(BOOL_CHECK_TRACE)
		 {	
			sync_requests=new LogFile(string("sync_requests_log.txt"));
			announce_lf= new LogFile(string("announce_req_log.txt"));
			t2t_get_peers_log=new LogFile(string("t2t-get-peers.txt"));
			error_in_response_encoding= new LogFile("error_in_response_encoding.txt");
		
		 }
		 this->ptr_wmrt->SetBroadcastParam(this->broadcast_addr,this->broadcast_port,this->broadcast_interval,this->tracker_port);
		
		 // Starting the HTTP Server
		ptr_tcs = new TrackerClientServer(this->tracker_port,Get,this,ptr_wmrt,this->blwt);
		
		// Traking the changes in the routing table
		this->ptr_nm->notification_of_changing_in_rt();
		
		if(this->TPCThread==NULL)
		{
			// Unable to create the thread
			this->tracker_state.assign(" Unable to Start The Tracker");	
		}else 
			{	
				tracker_started=true;
				WM_ADHOC_TRACKER_MENU trace_menu(this);
				GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_TRACE)->EnableWindow(FALSE);
							
				CEdit_lp.SetReadOnly(true);
				CEdit_TRACKER_CLIENT_POLL_INTERVAL.SetReadOnly(true);
				CEdit_BROADCAST_PORT.SetReadOnly(true);
				CEdit_BROADCAST_ADDRESS.SetReadOnly(true);
				CEdit_BI.SetReadOnly(true);

				// Starting the broadcast messages server 
				this->ptr_wmrt->start_brodcast_messages_receiver(this->broadcast_port);
				
				// Starting the hello message sender
				this->ptr_wmrt->look_for_1hop_neighbors();
				
				UpdateData(TRUE);
				this->tracker_state.append(" Tracker Running on Port :");
				char  port[20];
				sprintf(port,"%i",tracker_port);
				this->tracker_state.append(port);
				GetDlgItem(IDC_BUTTON_TRACE_MENU)->EnableWindow(TRUE);
				trace_menu.DoModal();
		}
	}
	
}

void CWMADHOCTRACKERDlg::OnEnChangeEditBlwt()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty()  && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty() && !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnChangeEditLp()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() &&!CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started&& !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnUpdateEditLp()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started&& !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnUpdateEditBlwt()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty()  && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnSetfocusEditLp()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnSetfocusEditBlwt()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnBnClickedButtonTraceMenu()
{
	// TODO: Add your control notification handler code here
	WM_ADHOC_TRACKER_MENU trace_menu(this);
	trace_menu.DoModal();
}



string  CWMADHOCTRACKERDlg::get_tracker_state()
{

	this->ptr_tm->DeleteOutOfDatePeers();
	int np=this->ptr_pm->get_number_of_peers();
	int nt=this->ptr_tm->get_number_of_torrents();
	char str_np[100];
	char str_nt[100];
	sprintf(str_np,"%i",np);
	sprintf(str_nt,"%i",nt);
	string res;
	res.append(" Tracker Ip: ");
	res.append(this->ptr_wmrt->get_local_ip_adr());
	res.append("\r\n");
		
	res.append(" Tracker Broadcast @: ");
	res.append(this->broadcast_addr);
	res.append("\r\n");

	res.append(" Tracker is receiving broadcast messages on port: ");
	char str_bp[10];
	sprintf(str_bp,"%i",this->broadcast_port);
	res.append(str_bp);
	res.append("\r\n");

	res.append(tracker_state);
	res.append("\r\n");
	res.append(" Nbr of Peers: ");res.append(str_np);res.append("\r\n");
	res.append(" Nbr of Torrents: ");res.append(str_nt);res.append("\r\n");
	//res.append(" Overlay Trackers: ");res.append(this->ptr_nm->get_overlay_trackers_list());res.append("\r\n");
	return res;
}

TrackerClientServer::request_func TrackerClientServer::request_func_=0;

UINT TrackerClientServer::Request(LPVOID ptr_tp) {
   struct thread_param * tp=reinterpret_cast<struct thread_param *>(ptr_tp);
   Socket s = *tp->sock;
   
  
  std::string line = s.ReceiveLine();
  if (line.empty()) {
	return 1;
  }

  http_request req;
  req.view=tp->view;

  if (line.find("GET") == 0) {
    req.method_="GET";
  }
  else if (line.find("POST") == 0) {
    req.method_="POST";
  }

  std::string path;
  std::map<std::string, std::string> params;

  size_t posStartPath = line.find_first_not_of(" ",3);

  SplitGetReq(line.substr(posStartPath), path, params);

  req.status_ = "202 OK";
  req.s_      = &s;
  req.path_   = path;
  req.params_ = params;

  static const std::string authorization   = "Authorization: Basic ";
  static const std::string accept          = "Accept: "             ;
  static const std::string accept_language = "Accept-Language: "    ;
  static const std::string accept_encoding = "Accept-Encoding: "    ;
  static const std::string user_agent      = "User-Agent: "         ;

  while(1) 
  {
		line=s.ReceiveLine();

		if (line.empty()) break;

		unsigned int pos_cr_lf = line.find_first_of("\x0a\x0d");
		if (pos_cr_lf == 0) break;

		line = line.substr(0,pos_cr_lf);

		if (line.substr(0, authorization.size()) == authorization) {
			req.authentication_given_ = true;
			std::string encoded = line.substr(authorization.size());
			std::string decoded = base64_decode(encoded);

			unsigned int pos_colon = decoded.find(":");

			req.username_ = decoded.substr(0, pos_colon);
			req.password_ = decoded.substr(pos_colon+1 );
		}
		else if (line.substr(0, accept.size()) == accept) {
			req.accept_ = line.substr(accept.size());
		}
		else if (line.substr(0, accept_language.size()) == accept_language) {
			req.accept_language_ = line.substr(accept_language.size());
		}
		else if (line.substr(0, accept_encoding.size()) == accept_encoding) {
			req.accept_encoding_ = line.substr(accept_encoding.size());
		}
		else if (line.substr(0, user_agent.size()) == user_agent) {
			req.user_agent_ = line.substr(user_agent.size());
		}
  }

	request_func_(&req);

	std::stringstream str_str;
	str_str << req.answer_.size();

	if(req.answer_.compare("Dont Respond") == 0)
	{
		s.Close();
		AfxEndThread(0);
		return 0;
	}
	else if(req.path_.compare("/GetListOfPublishedTorrents")==0 || req.path_.compare("/GetTorrentOwner")==0 || req.path_.compare("/GetLocalInfos")==0 )
	{
		if(req.singleLineAnswer)
			s.SendLine(req.answer_);
		else {
			for(int i = 0;i < req.numberOfAnswerLines; i++)
			{
				s.SendLine(req.answers_[i]);
			}
		}

		s.SendLine("");
		AfxEndThread(0);
		return 0;
	}
	else
	{

		static std::string const serverName = "WM-ADHOC-TRACKER";
		s.SendBytes("HTTP/1.1 ");
		if (! req.auth_realm_.empty() ) {
			s.SendLine("401 Unauthorized");
			s.SendBytes("WWW-Authenticate: Basic Realm=\"");
			s.SendBytes(req.auth_realm_);
			s.SendLine("\"");
		}
		else {
			s.SendLine(req.status_);
		}
		s.SendLine(std::string("Server: ") +serverName);
		s.SendLine("Connection: close");
		if(req.path_!="/GetStatus")
			s.SendLine("Content-Type: text/plain; charset=ISO-8859-1");
		else
			s.SendLine("Content-Type:  text/html; charset=ISO-8859-4");
		s.SendLine("Content-Length: " + str_str.str());
		s.SendLine("");

		if(req.singleLineAnswer)
			s.SendLine(req.answer_);
		else
		{
			for(int i = 0;i < req.numberOfAnswerLines; i++)
			{
				s.SendLine(req.answers_[i]);
			}
		}
		
		if(req.path_.compare("/announce") == 0)
			s.Close();
		AfxEndThread(0);
		return 0;
	}

	AfxEndThread(0);
}

// Constructor 

TrackerClientServer::TrackerClientServer(unsigned int port_to_listen, request_func r, CWMADHOCTRACKERDlg* view, WMRoutingTable * wmrt,int bl_waiting_time) 
{
	http_server_port = port_to_listen;
	InitializeCriticalSection(&search_state_cs);
	InitializeCriticalSection(&tf_search_state_cs);


	InitializeCriticalSection(&number_of_thread_searchers_cs);
	InitializeCriticalSection(&number_of_tf_thread_searchers_cs);

	thread_param *__unaligned tp=(thread_param*__unaligned )malloc(sizeof(thread_param));
	
	ptr_wmrt=wmrt;
	blwt=bl_waiting_time;
  
	request_func_ = r;
	tp->view=view;
	tp->port=port_to_listen;
	tp->blwt=bl_waiting_time;
	tp->wmrt=this->ptr_wmrt;
	tp->view=view;
	tp->sock=NULL;
	this->http_in=new SocketServer(port_to_listen,SOMAXCONN);
	tp->http_server_sock=http_in;
	http_server_thread = AfxBeginThread(start_http_server,(LPVOID)tp);
	tp=NULL;
}

// Starting the HTTP Server
UINT TrackerClientServer::start_http_server(LPVOID tp)
{
	thread_param *__unaligned tpt=(thread_param*__unaligned )tp;
	
	SocketServer *in = tpt->http_server_sock;
	while (isRunning) {
		thread_param *__unaligned p=(thread_param*__unaligned )malloc(sizeof(thread_param));
		p->blwt=tpt->blwt;
		p->port=tpt->port;
		p->view=tpt->view;
		p->wmrt=tpt->wmrt;
		Socket* ptr_s = in->Accept();
		p->sock=ptr_s;
		ptr_s=NULL;
		CWinThread * current_thread=AfxBeginThread(Request,(LPVOID)p);
		current_thread = NULL;
		p = NULL;
	}

	in->Close();
	free(tpt);
	tpt=NULL;
	AfxEndThread(0);
	return 0;
}

// Sending Tracker to Tracker request
void TrackerClientServer::send_t2t_request(string info_hash, unsigned int port_to_try,string dest_ip, bool we_know_the_dest, CWMADHOCTRACKERDlg* view,string source_ip, BlackIpList * blackList)
{
	
	t2t_param *__unaligned tp2= (t2t_param * __unaligned )malloc(sizeof(t2t_param));	
	
	tp2->port=port_to_try;
	tp2->blackList = blackList;
	if(dest_ip.length()==0)
		strcpy(tp2->host,"");
	else
		strcpy(tp2->host,dest_ip.c_str());

	if(source_ip.length()==0)
		strcpy(tp2->host,"");
	else
		strcpy(tp2->source_ip,source_ip.c_str());

	strcpy(tp2->info_hash,info_hash.c_str());
	
	tp2->known_host=we_know_the_dest;
	tp2->tcs=this;
	tp2->blwt=view->blwt;
	tp2->view=view;
	
	HANDLE search_event = CreateEvent(NULL,0,0,NULL);
	tp2->se = search_event;
	
	CWinThread * current_thread=AfxBeginThread(T2T_Request,(LPVOID)tp2);
	current_thread=NULL;
	
	// Wait for results
	number_of_thread_searchers = 0;
	WaitForSingleObject(search_event, PEERS_LOOKUP_WAITING_TIME);
	int number_of_remaining_threads;
	this->get_thread_searchers_number(&number_of_remaining_threads);
	if(number_of_remaining_threads>0)
	{
		// Stopping the Search Proccess
		this->set_ss(info_hash,false);
	}
	CloseHandle(search_event);
	
}

t2t_param * TrackerClientServer::get_new_t2t_param(string host,string source_ip, int port,string info_hash,bool known_host,TrackerClientServer *tcs,CWMADHOCTRACKERDlg *view,int blwt, HANDLE se, BlackIpList * blackList)
{
		t2t_param * __unaligned temp=(t2t_param * __unaligned)malloc(sizeof(t2t_param));
		temp->blackList = blackList;
		temp->se = se;
		if(!host.empty())
		{
			strcpy(temp->host,"");
			strcpy(temp->host,host.c_str());
			strcat(temp->host,"\0");
		}
		else strcpy(temp->host,"\0");
		
		if(!source_ip.empty())
		{
			strcpy(temp->source_ip,"");
			strcpy(temp->source_ip,source_ip.c_str());
			strcat(temp->source_ip,"\0");
		}

		else strcpy(temp->host,"\0");
		temp->port=port;
		
		if(info_hash.length()>0)
			{strcpy(temp->info_hash,"");strcpy(temp->info_hash,info_hash.c_str());strcat(temp->info_hash,"\0");}
		else strcpy(temp->info_hash,"\0");
		
		temp->known_host=known_host;
		
		temp->tcs=tcs;
		temp->view=view;
		temp->blwt=blwt;
	
		
	return temp;
}

UINT TrackerClientServer::T2T_Request(LPVOID ptr_tp)
{	
	t2t_param * __unaligned temp1=(t2t_param *__unaligned )(ptr_tp);
	t2t_param * __unaligned tp3=(t2t_param *__unaligned )temp1->tcs->get_new_t2t_param(temp1->host,temp1->source_ip, temp1->port,temp1->info_hash,temp1->known_host,temp1->tcs,temp1->view,temp1->blwt, temp1->se, temp1->blackList);
	
	free(temp1);
	
	
	t2t_param * __unaligned temp2;
	
	if(!tp3->known_host)
	{ // Start the lookup loop ( We are looking for the first interested peer)

		tp3->view->ptr_wmrt->enter_exp_map_cs();
		tp3->view->ptr_wmrt->get_exploration_map();
	
		if(!tp3->view->ptr_wmrt->exploration_map.empty())
		{
			// Iterators for the exploration map
			map<int,list<string>>::iterator exp_map_iter=tp3->view->ptr_wmrt->exploration_map.begin();
			list<string>::iterator ips_iter;
			map<int,list<string>>::iterator test1;
			map<int,list<string>>::iterator test2;
			

			// Creating the future black Ips list
			while((exp_map_iter!=tp3->view->ptr_wmrt->exploration_map.end()))
			{
				ips_iter=exp_map_iter->second.begin();

				while(ips_iter!=exp_map_iter->second.end())
				{
					if((*ips_iter).compare(tp3->source_ip) != 0 && !tp3->blackList->IsIpInTheBlackList(*ips_iter))
					{
						tp3->blackList->AddIpToBlackList(*ips_iter);
					}
					ips_iter++;
				}

				exp_map_iter++;
			}
			
			tp3->tcs->init_thread_searcher();

			while((exp_map_iter!=tp3->view->ptr_wmrt->exploration_map.end()) && (tp3->tcs->get_ss(string(tp3->info_hash))==true))
			{
				ips_iter=exp_map_iter->second.begin();
				int i=0;
				while(ips_iter!=exp_map_iter->second.end()&& tp3->tcs->get_ss(string(tp3->info_hash))== true )
				{
					if((*ips_iter).compare(tp3->source_ip) != 0 && !tp3->blackList->IsIpInTheBlackList(*ips_iter))
					{
						// Creating a new connection
						temp2=(t2t_param *)tp3->tcs->get_new_t2t_param((*ips_iter),tp3->source_ip, tp3->port,string(tp3->info_hash),true,tp3->tcs,tp3->view,tp3->blwt, tp3->se, tp3->blackList);
						// Creating the new thread
						CWinThread * current_thread=AfxBeginThread(T2T_Request_known_host,(LPVOID)temp2);
						i++;
						current_thread=NULL;
						int nt;
						tp3->tcs->add_thread_searcher(&nt);
					}
					
					ips_iter++;
				}
				
				exp_map_iter++;
			}
		
		}
		tp3->view->ptr_wmrt->exit_exp_map_cs();
		
	}else // We know the destination of the request
	{
		if(strcmp(tp3->host, tp3->source_ip) != 0)
		{
			// Creating a new connection
			temp2=(t2t_param  *)tp3->tcs->get_new_t2t_param(tp3->host,tp3->source_ip, tp3->port,string(tp3->info_hash),true,tp3->tcs,tp3->view,tp3->blwt, tp3->se, tp3->blackList);
			// Creating the new thread
			CWinThread * current_thread=AfxBeginThread(T2T_Request_known_host,(LPVOID)temp2);
			current_thread=NULL;
			int nt;
			tp3->tcs->add_thread_searcher(&nt);
		}
	}
	
	// Ending the thread
	int number_of_remaining_threads = 0;
	tp3->tcs->get_thread_searchers_number(&number_of_remaining_threads);
	if(number_of_remaining_threads == 0)
		{
			// Stopping the Search Proccess
			tp3->tcs->set_ss(tp3->info_hash,false);
			SetEvent(tp3->se);
		}
	if(tp3!=NULL)
		free(tp3);
	AfxEndThread(0);
	return 0;
}

UINT TrackerClientServer::T2T_Request_known_host(LPVOID ptr_tp)
{
	t2t_param * __unaligned tp3=(t2t_param *__unaligned)(ptr_tp);

	string l_ip(tp3->view->ptr_wmrt->get_local_ip_adr());
	if(tp3->tcs->get_ss(tp3->info_hash) && l_ip.compare(tp3->host)!=0)
	{
		try{
			SocketClient out(string(tp3->host),tp3->port);
			//! asking for peers interested in a specific torrent.
			std::string http_get_request;
			http_get_request.append("GET /t2t-get-peers?");
			http_get_request.append("info_hash=");
			http_get_request.append(tp3->info_hash);
			http_get_request.append("&ip=");
			http_get_request.append(tp3->tcs->ptr_wmrt->get_local_ip_adr());
			http_get_request.append("&port=");
			char temp_port[10];
			sprintf(temp_port,"%i",tp3->view->get_tracker_port());
			http_get_request.append(temp_port);

			// Add the Ips Black List
			http_get_request.append("&black_list=");
			http_get_request.append(tp3->blackList->GetFormattedList());
		
			http_get_request.append(" HTTP/1.1\n\n");
			
			
			//! Sending the http request.
			out.SendLine(http_get_request);
			out.SendLine(string("Accept: */*\n\n"));
			out.SendLine(string("User-Agent: WM-ADHOC-TRACKER\n\n"));
			
			out.SendLine(string("Host:")+tp3->tcs->ptr_wmrt->get_local_ip_adr()+string(string(":")+string(temp_port)+string("\n\n")));
			out.SendLine(string("Connection: Keep-Alive\n\n"));
			out.SendLine("");
			//! Waiting for the response.
			bool response_recived=false;
			
			int message_end=0;	
			while(!response_recived && tp3->tcs->get_ss(tp3->info_hash))
			{	
				string line=out.ReceiveLine();
				if((line.empty()||line.size() == 1)&& message_end==0) message_end=1;
				
				if((line.substr(0,3).compare("not")==0) || (line.empty()&&message_end==1))
				{
					//! The destination peer is not interested in the torrent.
					//! Updating the trackers overlay.
			
					out.Close();
					int nt;
					tp3->tcs->drop_thread_searcher(&nt);
					if(nt==0)
						SetEvent(tp3->se);
					if(tp3!=NULL)
						free(tp3);
					//! Ending the thread.
					AfxEndThread(0);
				}
				
				if(line.find("peers")!=string::npos) 
				{
					// Reading the list of peers and updating the maps
					//! the list of peers is bencoded list.
					//! Reading the bencoded message.
					BString bs;
					bs.read_buf(line.substr(0,line.find_last_of("e")).c_str(),line.substr(0,line.find_last_of("e")).length());
					
					BTList * lp= dynamic_cast<BTList *>(bs.GetBTObject("peers"));
					if(lp)
					{
						//! Updating the trackers overlay.
						if(iUseTrackerOverlay)
							tp3->view->ptr_nm->add_tracker(tp3->host,tp3->port);
						//! peers found.
						for(btobject_v::iterator it=lp->GetList().begin();it!=lp->GetList().end();it++)
						{
							BTDictionary *p=dynamic_cast<BTDictionary *>(*it);
							string ip;
							string id;
							int port=0;
							int downloaded=0;
							int uploaded=0;
							int left=0;
							string ev;
							if(p)
							{
								// Dictionary found
								for(btmap_t::iterator it=p->GetMap().begin();it!=p->GetMap().end();it++)
								{
									string key=it->first;
									BTObject *p0=it->second;
									if(key=="ip")
									{
										BTString *p=dynamic_cast<BTString *>(p0);
										if(p)
										{
											ip=p->GetValue();
										}
										p=NULL;
									}else if(key=="port")
									{
										BTInteger *p=dynamic_cast<BTInteger*>(p0);
										if(p)
										{
											port=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="peer id")
									{
										BTString *p=dynamic_cast<BTString *>(p0);
										if(p)
										{
											id=p->GetValue();
										}
										p=NULL;
									}else if(key=="downloaded")////////////////
									{
										BTInteger *p=dynamic_cast<BTInteger *>(p0);
										if(p)
										{
											downloaded=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="uploaded")
									{
										BTInteger *p=dynamic_cast<BTInteger *>(p0);
										if(p)
										{
											uploaded=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="left")
									{
										BTInteger *p=dynamic_cast<BTInteger *>(p0);
										if(p)
										{
											left=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="event")
									{
										BTString *p=dynamic_cast<BTString *>(p0);
										if(p)
										{
											ev=p->GetValue();
										}
										p=NULL;
									}
								p0=NULL;	
								}
								
								//! Adding the peer interested in tp->info_hash.
								if(id.size()==20 && port && ip.size()>0)
								{
									//! Adding the peer to torrents manager.
									tp3->view->ptr_tm->add_peer_to_torrent(tp3->info_hash,string(""),(char *) id.c_str());
									//! Adding the peer to the peers manager.
									tp3->view->ptr_pm->add_peer_2(id,ip,port,0,tp3->view->ptr_wmrt->GetNumberOfHops(ip),tp3->view->tracker_client_poll_interval);

									tp3->view->ptr_pm->add_torrent_of_interest(id,string(tp3->info_hash),uploaded,downloaded,left,ev,ip);
									
								}
								
							}

						}
						if(tp3->tcs->get_ss(string(tp3->info_hash)))
							{
								response_recived=true;
								//! End the lookup proccess, because we founded the first interested peer. 
								tp3->tcs->set_ss(string(tp3->info_hash),false);
								lp=NULL;
								out.Close();
								int nt;
								tp3->tcs->drop_thread_searcher(&nt);
								SetEvent(tp3->se);
								if(tp3!=NULL)
									free(tp3);	
								//! Ending the thread.
								AfxEndThread(0);
						}

					}
					out.Close();
					lp=NULL;
					int nt;
					tp3->tcs->drop_thread_searcher(&nt);
					if(tp3!=NULL)
						free(tp3);
			  		//! Ending the thread.
					AfxEndThread(0);
				}
			}
			

		
		}
	catch(string e)
	{	
		int nt;
		tp3->tcs->drop_thread_searcher(&nt);
		if(nt==0 && tp3->tcs->get_ss(tp3->info_hash))
			SetEvent(tp3->se);
		if(tp3!=NULL)
			free(tp3);
		//! Ending the thread.
		AfxEndThread(0);
	}
	}
	//! Ending the thread.
	int nt;
	tp3->tcs->drop_thread_searcher(&nt);
	if(tp3!=NULL)
		free(tp3);
	AfxEndThread(0);
	return 0;
}


TrackerClientServer::~TrackerClientServer()
{
	isRunning = false;
	// Deblock and close the HTTP Server, will be replaced by a non blocking socket call
	SocketClient out(string("127.0.0.1"), http_server_port);
	out.Close();

	// Wait for the server thread
	WaitForSingleObject(http_server_thread->m_hThread, 60000);
	http_server_thread->m_hThread = NULL;
	
	ptr_wmrt=NULL;
	search_status.clear();
	search_status_cs.clear();

	DeleteCriticalSection(&search_state_cs);
	DeleteCriticalSection(&tf_search_state_cs);

	DeleteCriticalSection(&number_of_thread_searchers_cs);
	DeleteCriticalSection(&number_of_tf_thread_searchers_cs);
}

//! Set the search status for a given torrent.
void TrackerClientServer::set_ss(string info_hash, bool state)
{
	EnterCriticalSection(&search_state_cs);
	
		std::map<string,bool>::iterator iter;
		iter=this->search_status.find(info_hash);
		if(iter==this->search_status.end())
		{
			//! Add a new torrent search status.
			this->search_status.insert(make_pair<string,bool>(info_hash,state));
			
		}
		else {
			iter->second=state;
		}
	
	LeaveCriticalSection(&search_state_cs);
}

void TrackerClientServer::set_tf_ss(string file_name, bool state)
{
	EnterCriticalSection(&tf_search_state_cs);
	
		std::map<string,bool>::iterator iter;
		iter=this->tf_search_status.find(file_name);
		if(iter==this->tf_search_status.end())
		{
			//! Add a new torrent search status.
			this->tf_search_status.insert(make_pair<string,bool>(file_name,state));
			
		}
		else {
			iter->second=state;
		}
	
	LeaveCriticalSection(&tf_search_state_cs);
}

//! Return the search status for a given torrent.
bool TrackerClientServer::get_ss(string info_hash)
{
	bool res = false;
	EnterCriticalSection(&search_state_cs);
	
		map<string,bool>::iterator iter;
		iter=this->search_status.find(info_hash);
		if(iter==this->search_status.end())
		{
			res = false;
		}else { res = iter->second;}

	LeaveCriticalSection(&search_state_cs);
	return res;
}

bool TrackerClientServer::get_tf_ss(string file_name)
{
	bool res = false;
	EnterCriticalSection(&tf_search_state_cs);
	
		map<string,bool>::iterator iter;
		iter=this->tf_search_status.find(file_name);
		if(iter==this->tf_search_status.end())
		{
			res = false;
		}else { res =  iter->second;}

	LeaveCriticalSection(&tf_search_state_cs);
	return res;
}

void CWMADHOCTRACKERDlg::OnBnClickedCheckTrace()
{
	// TODO: Add your control notification handler code here
}

void CWMADHOCTRACKERDlg::OnBnKillfocusCheckTrace()
{
	// TODO: Add your control notification handler code here
}

void CWMADHOCTRACKERDlg::OnEnChangeEditPollInterval()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}

void CWMADHOCTRACKERDlg::OnEnUpdateEditPi()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnSetfocusEditPi()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
	
}

void CWMADHOCTRACKERDlg::OnEnChangeEditBroadcastPort()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
		UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}

void CWMADHOCTRACKERDlg::OnEnSetfocusEditBroadcastPort()
{
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}

void CWMADHOCTRACKERDlg::OnEnUpdateEditBroadcastPort()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
		UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}

void CWMADHOCTRACKERDlg::OnEnChangeEditBroadcastAddress()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
			UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}

void CWMADHOCTRACKERDlg::OnEnSetfocusEditBroadcastAddress()
{
	// TODO: Add your control notification handler code here
			UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}

void CWMADHOCTRACKERDlg::OnEnUpdateEditBroadcastAddress()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
			UpdateData(TRUE);
	if(!Cstr_LP.IsEmpty() && !CString_TRACKER_CLIENT_POLL_INTERVAL.IsEmpty()&& !tracker_started && !CString_BROADCAST_PORT.IsEmpty()&& !CString_BROADCAST_ADDRESS.IsEmpty()&&!CString_BROADCAST_INTERVAL.IsEmpty())
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_BUTTON_CAS)->EnableWindow(FALSE);
}



// Sending Tracker to Tracker request for a torrent file
void TrackerClientServer::send_t2t_torrent_file_request(string file_name, unsigned int port_to_try,string dest_ip, bool we_know_the_dest, CWMADHOCTRACKERDlg* view, string source_ip, BlackIpList * bl)
{
	t2t_param_tf *__unaligned tp2= (t2t_param_tf * __unaligned )malloc(sizeof(t2t_param_tf));	
	
	tp2->port = port_to_try;
	
	if(dest_ip.length() == 0)
		strcpy(tp2->host,"");
	else
		strcpy(tp2->host,dest_ip.c_str());

	if(source_ip.length() == 0)
		strcpy(tp2->source_ip,"");
	else
		strcpy(tp2->source_ip,source_ip.c_str());

	strcpy(tp2->file_name, file_name.c_str());
	
	tp2->known_host = we_know_the_dest;
	tp2->tcs=this;
	tp2->blwt=view->blwt;
	tp2->view=view;
	tp2->blackList = bl;
	number_of_tf_thread_searchers = 0;
	HANDLE torrent_files_search_event = CreateEvent(NULL,0,0,NULL);
	tp2->se = torrent_files_search_event;

	CWinThread * current_thread = AfxBeginThread(T2T_tf_Request,(LPVOID)tp2);
	current_thread=NULL;
	
	// Wait for results
	
	WaitForSingleObject(torrent_files_search_event, TORRENT_FILES_LOOKUP_WAITING_TIME);
	CloseHandle(torrent_files_search_event);
}


t2t_param_tf * TrackerClientServer::get_new_t2t_tf_param(string host,string source_ip, int port,string file_name,bool known_host,TrackerClientServer *tcs,CWMADHOCTRACKERDlg *view,int blwt, HANDLE se, BlackIpList * bl)
{
		t2t_param_tf * __unaligned temp = (t2t_param_tf * __unaligned)malloc(sizeof(t2t_param_tf));
		temp->se = se;
		temp->blackList = bl;
		if(host.length()>0)
		{
			strcpy(temp->host, "");
			strcpy(temp->host,host.c_str());
			strcat(temp->host,"\0");
		}
		else strcpy(temp->host, "\0");
		
		if(!source_ip.empty())
		{
			strcpy(temp->source_ip, "");
			strcpy(temp->source_ip, source_ip.c_str());
			strcat(temp->source_ip,"\0");
		}
		else strcpy(temp->source_ip, "\0");
		
		temp->port=port;
		
		if(file_name.length()>0)
		{
			strcpy(temp->file_name, "");
			strcpy(temp->file_name, file_name.c_str());
			strcat(temp->file_name, "\0");
		}
		else strcpy(temp->file_name, "\0");
		
		temp->known_host = known_host;
		
		temp->tcs = tcs;
		temp->view = view;
		temp->blwt = blwt;
	
		return temp;
}


UINT TrackerClientServer::T2T_tf_Request(LPVOID ptr_tp)
{	
	t2t_param_tf * __unaligned temp1=(t2t_param_tf *__unaligned )(ptr_tp);
	t2t_param_tf * __unaligned tp3 = (t2t_param_tf *__unaligned )temp1->tcs->get_new_t2t_tf_param(temp1->host, temp1->source_ip, temp1->port,temp1->file_name,temp1->known_host,temp1->tcs,temp1->view,temp1->blwt, temp1->se, temp1->blackList);
	
	free(temp1);
	
	
	t2t_param_tf * __unaligned temp2;
	
	if(!tp3->known_host)
	{ // Start the lookup loop ( We are looking for the first interested peer)

		tp3->view->ptr_wmrt->enter_exp_map_cs();
		tp3->view->ptr_wmrt->get_exploration_map();
	
		if(!tp3->view->ptr_wmrt->exploration_map.empty())
		{
			// Iterators for the exploration map
			map<int,list<string>>::iterator exp_map_iter=tp3->view->ptr_wmrt->exploration_map.begin();
			list<string>::iterator ips_iter;
			map<int,list<string>>::iterator test1;
			map<int,list<string>>::iterator test2;
			
			
			
			// Creating the future black Ips list
			while((exp_map_iter!=tp3->view->ptr_wmrt->exploration_map.end()))
			{
				ips_iter=exp_map_iter->second.begin();

				while(ips_iter!=exp_map_iter->second.end())
				{
					if((*ips_iter).compare(tp3->source_ip) != 0 && !tp3->blackList->IsIpInTheBlackList(*ips_iter))
					{
						tp3->blackList->AddIpToBlackList(*ips_iter);
					}
					ips_iter++;
				}

				exp_map_iter++;
			}
			
			tp3->tcs->init_tf_thread_searcher();
			exp_map_iter=tp3->view->ptr_wmrt->exploration_map.begin();
			while((exp_map_iter!=tp3->view->ptr_wmrt->exploration_map.end()) && (tp3->tcs->get_tf_ss(string(tp3->file_name)) == true))
			{
				ips_iter=exp_map_iter->second.begin();
				int i = 0;
				while(ips_iter!=exp_map_iter->second.end()&& tp3->tcs->get_tf_ss(string(tp3->file_name))==true )
				{
					if((*ips_iter).compare(tp3->source_ip) != 0 && !tp3->blackList->IsIpInTheBlackList(*ips_iter))
					{
						// Creating a new connection
						temp2=(t2t_param_tf *)tp3->tcs->get_new_t2t_tf_param((*ips_iter),tp3->source_ip, tp3->port,string(tp3->file_name),true,tp3->tcs,tp3->view,tp3->blwt, tp3->se, tp3->blackList);
						// Creating the new thread
						CWinThread * current_thread = AfxBeginThread(T2T_tf_Request_known_host,(LPVOID)temp2);
						i++;
						current_thread=NULL;
						int nt;
						tp3->tcs->add_tf_thread_searcher(&nt);
					}
					ips_iter++;
					Sleep(1);
				}

				exp_map_iter++;
			}
		
		}
		tp3->view->ptr_wmrt->exit_exp_map_cs();
		
	}else // We know the destination of the request
	{
		if(strcmp(tp3->host, tp3->source_ip) != 0 && tp3->blackList->IsIpInTheBlackList(tp3->host))
		{
			// Creating a new connection
			tp3->blackList->AddIpToBlackList(string(tp3->host));
			temp2=(t2t_param_tf  *)tp3->tcs->get_new_t2t_tf_param(tp3->host,tp3->source_ip, tp3->port,string(tp3->file_name),true,tp3->tcs,tp3->view,tp3->blwt, tp3->se, tp3->blackList);
			// Creating the new thread
			CWinThread * current_thread=AfxBeginThread(T2T_tf_Request_known_host,(LPVOID)temp2);
			current_thread=NULL;
			int nt;
			tp3->tcs->add_tf_thread_searcher(&nt);
		}
	}
	
// Ending the thread
int number_of_remaining_threads = 0;
tp3->tcs->get_tf_thread_searchers_number(&number_of_remaining_threads);
if(number_of_remaining_threads == 0)
{
		// Stopping the Search Proccess
		tp3->tcs->set_tf_ss(tp3->file_name,false);
		SetEvent(tp3->se);
}

if(tp3!=NULL)
{

	free(tp3);
}
AfxEndThread(0);
return 0;
}


UINT TrackerClientServer::T2T_tf_Request_known_host(LPVOID ptr_tp)
{
	t2t_param_tf * __unaligned tp3=(t2t_param_tf *__unaligned)(ptr_tp);

	string l_ip(tp3->view->ptr_wmrt->get_local_ip_adr());

	if(tp3->tcs->get_tf_ss(tp3->file_name) && l_ip.compare(tp3->host) !=0 )
	{
		SocketClient *out = NULL;
		try{

			out = new SocketClient(string(tp3->host),tp3->port);
			//! asking for peers interested in a specific torrent.
			std::string http_get_request;
			http_get_request.append("GET /GetTorrentOwner?");
			http_get_request.append("file_name=");
			http_get_request.append(tp3->file_name);
			
			http_get_request.append("&ip=");
			http_get_request.append(tp3->tcs->ptr_wmrt->get_local_ip_adr());
			http_get_request.append("&port=");
			char temp_port[10];
			sprintf(temp_port,"%i",tp3->view->get_tracker_port());
			http_get_request.append(temp_port);
			
			// Add the Ips Black List
			http_get_request.append("&black_list=");
			http_get_request.append(tp3->blackList->GetFormattedList());
			http_get_request.append(" HTTP/1.1\n\n");
			
			
			//! Sending the http request.
			out->SendLine(http_get_request);
			out->SendLine(string("Accept: */*\n\n"));
			out->SendLine(string("User-Agent: WM-ADHOC-TRACKER\n\n"));
			out->SendLine(string("Host:")+tp3->tcs->ptr_wmrt->get_local_ip_adr()+string(string(":")+string(temp_port)+string("\n\n")));
			out->SendLine(string("Connection: Keep-Alive\n\n"));
			out->SendLine("");
			//! Waiting for the response.
			bool getting_data = false;
			int message_end = 0;	

			while(tp3->tcs->get_tf_ss(tp3->file_name) || getting_data)
			{	
				string line = out->ReceiveLine();

				if((line.empty()||line.size() == 1)&& message_end==0) 
					message_end = 1;
				
				if((line.empty()||line.size() == 1) && message_end == 1)
				{
					// End of the message
					if(out != NULL)
					{
						out->Close();
						out = NULL;
					}
					
					int nt;
					tp3->tcs->drop_tf_thread_searcher(&nt);
					if(nt == 0  && tp3->se != NULL)
					{
						tp3->tcs->set_tf_ss(string(tp3->file_name),false);
						SetEvent(tp3->se);
					}
					
					if(tp3 != NULL)
						free(tp3);
					//! Ending the thread.
					AfxEndThread(0);
					return 0;
				}
				
				// The list of the file owners
				if(line.find('#') != string::npos) 
				{
					if(!getting_data)
						getting_data = true;
					
					// Reading the list of peers and updating the database
					// Single file format: node1Ip#file1*file1.size*filedescription \n
					
					// The node ip
					string nodeIp;
					int dz = line.find_first_of('#');
					nodeIp = line.substr(0,dz);
					
					// File Names
					string fileName;
					string tmp = line.substr(dz+1);
					dz = tmp.find_first_of('*');
					fileName = tmp.substr(0, dz);

					//File Size
					tmp = tmp.substr(dz+1);
					dz = tmp.find_first_of('*');
					long fileSize = 0;
					fileSize = atol(tmp.substr(0, dz).c_str());
					
					// File Description
					tmp = tmp.substr(dz+1);
					dz = tmp.find_first_of('\n');
					
					string fileDescription = tmp.substr(0, dz);
					
					
					// Adding the file to the data base
					if(tp3->tcs->get_tf_ss(tp3->file_name))
						tp3->view->ptr_tsm->AddNewTorrentFile(nodeIp, fileName, fileDescription, fileSize);
					

				}

				// The list of peers involved in the torrent sharing session
				if(line.find("peers")!=string::npos) 
				{
					// Extracting the torrent info_hash
					size_t infoPos = line.find_first_of('|');
					string tmp = line.substr(infoPos+1);
					line = line.substr(0, infoPos);
					infoPos = tmp.find_first_of('|');
					string fileName = tmp.substr(0, infoPos);
					string info_hash = tmp.substr(infoPos+1);
					// By default, eleminate extra caracters 
					info_hash = info_hash.substr(0, 20);


					// Reading the list of peers and updating the maps
					//! the list of peers is bencoded list.
					//! Reading the bencoded message.
					BString bs;
					bs.read_buf(line.substr(0,line.find_last_of("e")).c_str(),line.substr(0,line.find_last_of("e")).length());
					
					BTList * lp= dynamic_cast<BTList *>(bs.GetBTObject("peers"));
					if(lp)
					{
						//! Updating the trackers overlay.
						if(iUseTrackerOverlay)
							tp3->view->ptr_nm->add_tracker(tp3->host,tp3->port);
						//! peers found.
						for(btobject_v::iterator it=lp->GetList().begin();it!=lp->GetList().end();it++)
						{
							BTDictionary *p=dynamic_cast<BTDictionary *>(*it);
							string ip;
							string id;
							int port=0;
							int downloaded=0;
							int uploaded=0;
							int left=0;
							string ev;
							if(p)
							{
								// Dictionary found
								for(btmap_t::iterator it=p->GetMap().begin();it!=p->GetMap().end();it++)
								{
									string key=it->first;
									BTObject *p0=it->second;
									if(key=="ip")
									{
										BTString *p=dynamic_cast<BTString *>(p0);
										if(p)
										{
											ip=p->GetValue();
										}
										p=NULL;
									}else if(key=="port")
									{
										BTInteger *p=dynamic_cast<BTInteger*>(p0);
										if(p)
										{
											port=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="peer id")
									{
										BTString *p=dynamic_cast<BTString *>(p0);
										if(p)
										{
											id=p->GetValue();
										}
										p=NULL;
									}else if(key=="downloaded")////////////////
									{
										BTInteger *p=dynamic_cast<BTInteger *>(p0);
										if(p)
										{
											downloaded=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="uploaded")
									{
										BTInteger *p=dynamic_cast<BTInteger *>(p0);
										if(p)
										{
											uploaded=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="left")
									{
										BTInteger *p=dynamic_cast<BTInteger *>(p0);
										if(p)
										{
											left=atoi(p->GetValue().c_str());
										}
										p=NULL;
									}else if(key=="event")
									{
										BTString *p=dynamic_cast<BTString *>(p0);
										if(p)
										{
											ev=p->GetValue();
										}
										p=NULL;
									}
								p0=NULL;	
								}
								
								//! Adding the peer interested in tp->info_hash.
								if(id.size()==20 && port && ip.size()>0)
								{
									//! Adding the peer to torrents manager.
									tp3->view->ptr_tm->add_peer_to_torrent((char*)info_hash.c_str(),fileName,(char *) id.c_str());
									//! Adding the peer to the peers manager.
									tp3->view->ptr_pm->add_peer_2(id,ip,port,0,tp3->view->ptr_wmrt->GetNumberOfHops(ip),tp3->view->tracker_client_poll_interval);

									tp3->view->ptr_pm->add_torrent_of_interest(id,info_hash,uploaded,downloaded,left,ev,ip);
									
								}
								
							}

						}
					}

				}
					
			}
			
			{
			if(out != NULL)
			{
				out->Close();
				out = NULL;
			}
			int nt;
			tp3->tcs->drop_tf_thread_searcher(&nt);
			if(nt == 0 && tp3->se != NULL )
				SetEvent(tp3->se);
			
			if(tp3 != NULL)
				free(tp3);
			//! Ending the thread.
			AfxEndThread(0);
			return 0;
			}

		}

		catch(string e)
		{	
			if(out != NULL)
			{
				out->Close();
				out = NULL;
			}
			int nt;
			tp3->tcs->drop_tf_thread_searcher(&nt);
			if(nt==0 && tp3->se != NULL)
				SetEvent(tp3->se);
		
			if(tp3!=NULL)
				free(tp3);
			//! Ending the thread.
			AfxEndThread(0);
			return 0;
		}
	}

	//! Ending the thread.
	int nt;
	tp3->tcs->drop_tf_thread_searcher(&nt);
	if(nt == 0 && tp3->se != NULL)
		SetEvent(tp3->se);
	
	if(tp3!=NULL)
		free(tp3);
	AfxEndThread(0);
	return 0;
}

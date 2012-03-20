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
* \file WM-ADHOC-TRACKERDlg.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _CWMADHOCTRACKERDlg_H
#define _CWMADHOCTRACKERDlg_H

#pragma once
#include "afxwin.h"
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <list>
using namespace std;
class LogFile;
class TorrentManager;
class NeighborsManager;
class WMRoutingTable;
class PeersManager;
class TrackerClientServer;
class TorrentStorageManager;
class BlackIpList;

// Boîte de dialogue CWMADHOCTRACKERDlg
class CWMADHOCTRACKERDlg : public CDialog
{
// Construction
public:
	CWMADHOCTRACKERDlg(CWnd* pParent = NULL);	// constructeur standard
	~CWMADHOCTRACKERDlg();
	TorrentManager      * ptr_tm;
	PeersManager        * ptr_pm;
	WMRoutingTable      * ptr_wmrt;
	TrackerClientServer * ptr_tcs;
	NeighborsManager	* ptr_nm;
	TorrentStorageManager * ptr_tsm;
	int		   blwt;
	int broadcast_port;
	int broadcast_interval;
	string broadcast_addr;
	string toStlString(const CString& aString);
	static UINT TrackerPeerConnectionThreadLauncher(LPVOID p);
	void show_message(string msg);
	string  get_tracker_state();
	// LogFiles
	LogFile *announce_lf;
	LogFile *t2t_get_peers_log;
	LogFile *error_in_response_encoding;
	LogFile *sync_requests;
	// Données de boîte de dialogue
	enum { IDD = IDD_WMADHOCTRACKER_DIALOG };
	int get_tracker_port(){return tracker_port;};
	typedef struct tracker_peer_connectio_thread_param
	{
	CWMADHOCTRACKERDlg * parent;
	unsigned int server_port;
	unsigned int blwt;
	}tracker_peer_connectio_thread_param;
    tracker_peer_connectio_thread_param tp;
private:
	CWinThread *TPCThread;  
	string	   tracker_state;
	int		   tracker_port;
	bool tracker_started;

public:
	int		   tracker_client_poll_interval;	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// Prise en charge de DDX/DDV

// Implémentation
protected:
	HICON m_hIcon;

	// Fonctions générées de la table des messages
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnBnClickedButtonCas();
public:
	CString Cstr_LP;
public:
	CString Cstr_BLWT;
public:
	CButton Cbutton_Configure_and_Start;
public:
	afx_msg void OnEnChangeEditBlwt();
public:
	afx_msg void OnEnChangeEditLp();
public:
	afx_msg void OnEnUpdateEditLp();
public:
	afx_msg void OnEnUpdateEditBlwt();
public:
	afx_msg void OnEnSetfocusEditLp();
public:
	afx_msg void OnEnSetfocusEditBlwt();
public:
	afx_msg void OnBnClickedButtonTraceMenu();
public:
	CEdit CEdit_blwt;
public:
	CEdit CEdit_lp;
public:
	afx_msg void OnBnClickedCheckTrace();
public:
	afx_msg void OnBnKillfocusCheckTrace();
public:
	CEdit CTRACKER_CLIENT_POLL_INTERVAL;
public:
	afx_msg void OnEnChangeEditPollInterval();
public:
	CString CString_TRACKER_CLIENT_POLL_INTERVAL;
public:
	CEdit CEdit_TRACKER_CLIENT_POLL_INTERVAL;
public:
	afx_msg void OnEnUpdateEditPi();
public:
	afx_msg void OnEnSetfocusEditPi();
public:
	 void OnSysCommand(UINT nID,LPARAM lParam);
public:
	BOOL BOOL_CHECK_TRACE;
public:
	CString CString_BROADCAST_PORT;
public:
	afx_msg void OnEnChangeEditBroadcastPort();
public:
	afx_msg void OnEnSetfocusEditBroadcastPort();
public:
	afx_msg void OnEnUpdateEditBroadcastPort();
public:
	CEdit CEdit_BROADCAST_PORT;
public:
	CEdit CEdit_BROADCAST_ADDRESS;
public:
	CString CString_BROADCAST_ADDRESS;
public:
	afx_msg void OnEnChangeEditBroadcastAddress();
public:
	afx_msg void OnEnSetfocusEditBroadcastAddress();
public:
	afx_msg void OnEnUpdateEditBroadcastAddress();
public:
	CString CString_BROADCAST_INTERVAL;
public:
	CEdit CEdit_BI;
};



class Socket;
class SocketServer;
/*!
* \struct thread_param 
* \brief an object used to pass arguments to a thred.
*/
typedef struct thread_param{
	Socket * sock;
	CWMADHOCTRACKERDlg *view;
	int blwt;
	WMRoutingTable * wmrt;
	int port;
	SocketServer * http_server_sock;
}thread_param;

/*!
* \struct t2t_param 
* \brief an object used to pass arguments to a thred.
*/
typedef struct t2t_param{
	unsigned int port;
	char host[50];
	char source_ip[50];
	// Torrent we are interested in
	char info_hash[30];
	// Searching 
	bool known_host;
	TrackerClientServer * tcs;
	CWMADHOCTRACKERDlg* view;
	HANDLE se;
	int blwt;
	BlackIpList * blackList;
}t2t_param;


/*!
* \struct t2t_param_tf
* \brief an object used to pass arguments to a thred.
*/

typedef struct t2t_param_tf{
	unsigned int port;
	char host[50];
	char source_ip[50];
	// Torrent we are interested in
	char file_name[1024];
	// Searching 
	bool known_host;
	TrackerClientServer * tcs;
	CWMADHOCTRACKERDlg* view;
	int blwt;
	HANDLE se;
	BlackIpList * blackList;
}t2t_param_tf;

#define MAX_MULTI_LINE_ANSWERS 10
/*! 
* \class TrackerClientServer
* \brief used to manage the communication between the tracker and the client.
*/
class TrackerClientServer {
  public:

   	/*! 
	* \fn void send_t2t_request(string info_hash, unsigned int port, string host, bool search,CWMADHOCTRACKERDlg * view)
	* \brief Method used to send tracker to tracker requests about peers for a given torrent.
	*/
	void send_t2t_request(string info_hash, unsigned int port, string dest_ip, bool search,CWMADHOCTRACKERDlg * view, string source_ip,BlackIpList * bl);
	void send_t2t_torrent_file_request(string file_name, unsigned int port, string dest_ip, bool search,CWMADHOCTRACKERDlg * view, string source_ip, BlackIpList * bl);
	int blwt;
	WMRoutingTable * ptr_wmrt;
	
	/*! 
	* \struct http_request 
	* \brief This structer is used to store a http request pqrqmeters.
	*/
	
	struct http_request {
    
      http_request() : authentication_given_(false) 
	  {
		numberOfAnswerLines = 0;
		singleLineAnswer = true;
	  }
	  CWMADHOCTRACKERDlg*				 view;
      Socket*                            s_;
      std::string                        method_;
      std::string                        path_;
      std::map<std::string, std::string> params_;
	  std::string                        accept_;
      std::string                        accept_language_;
      std::string                        accept_encoding_;
      std::string                        user_agent_;
    
      /* status_: used to transmit server's error status, such as
         o  202 OK
         o  404 Not Found 
         and so on */
      std::string                        status_;
    
      /* auth_realm_: allows to set the basic realm for an authentication,
         no need to additionally set status_ if set */
      std::string                        auth_realm_;
    
	  // For a single line answer
	  std::string                        answer_;

	  // For a multiline answer
	  bool	singleLineAnswer;
	  std::string answers_[MAX_MULTI_LINE_ANSWERS];
	  int numberOfAnswerLines;
    
      /*   authentication_given_ is true when the user has entered a username and password.
           These can then be read from username_ and password_ */
      bool authentication_given_;
      std::string username_;
      std::string password_;
    };

    typedef   void (*request_func) (http_request*);
    TrackerClientServer(unsigned int port_to_listen, request_func,CWMADHOCTRACKERDlg* view,WMRoutingTable * wmrt,int blwt);
	~TrackerClientServer();
	
	/*! 
	* \ fn static UINT T2T_Request(LPVOID ptr_tp)
	* \brief Method used by a thread as a start point in order to manage tracker/tracker requests.
	*/
	static UINT T2T_Request(LPVOID ptr_tp);
	static UINT T2T_tf_Request(LPVOID ptr_tp);

	/*! 
	* \fn static UINT TrackerClientServer::start_http_server(LPVOID tp)
	* \brief Method used by a thread as a start point in order to manage tracker/tracker requests.
	*/
	static UINT TrackerClientServer::start_http_server(LPVOID tp);
public:




	/*!
	* \fn void set_ss(string info_hash, bool state)
	* \brief Method used to update the search status of a given torrent.
	*/
	void set_ss(string info_hash, bool state);
	void set_tf_ss(string file_name, bool state);


	/*! 
	* \fn bool get_ss(string info_hash)
	* \brief Method used to get the search status of a given torrent.
	*/
	bool get_ss(string info_hash);
	bool get_tf_ss(string info_hash);
	

	/*! 
	* \fn static UINT T2T_Request_known_host(LPVOID ptr_tp)
	* \brief Function used as an entry point to the thread which will be used to send tracker to tracker request.
	*/
	
	static UINT T2T_Request_known_host(LPVOID ptr_tp);
	static UINT T2T_tf_Request_known_host(LPVOID ptr_tp);

	t2t_param * get_new_t2t_param(string host,string source_ip, int port,string info_hash,bool known_host,TrackerClientServer *tcs,CWMADHOCTRACKERDlg *view,int blwt, HANDLE se,BlackIpList * blackList);
	t2t_param_tf * get_new_t2t_tf_param(string host, string source_ip, int port,string file_name,bool known_host,TrackerClientServer *tcs,CWMADHOCTRACKERDlg *view,int blwt, HANDLE se, BlackIpList * bl);

	

	void add_thread_searcher(int * i){
		EnterCriticalSection(&number_of_thread_searchers_cs);
			number_of_thread_searchers++;
			*i=number_of_thread_searchers;
		LeaveCriticalSection(&number_of_thread_searchers_cs);
	};
	void drop_thread_searcher(int * i){
		EnterCriticalSection(&number_of_thread_searchers_cs);
			number_of_thread_searchers--;
			*i=number_of_thread_searchers;
		LeaveCriticalSection(&number_of_thread_searchers_cs);
	};
	void init_thread_searcher(){
		EnterCriticalSection(&number_of_thread_searchers_cs);
			number_of_thread_searchers = 0;
		LeaveCriticalSection(&number_of_thread_searchers_cs);
	};

	void get_thread_searchers_number(int *i){
		EnterCriticalSection(&number_of_thread_searchers_cs);
			*i = number_of_thread_searchers;
		LeaveCriticalSection(&number_of_thread_searchers_cs);
	};

	void add_tf_thread_searcher(int * i){
		EnterCriticalSection(&number_of_tf_thread_searchers_cs);
			number_of_tf_thread_searchers++;
			*i=number_of_tf_thread_searchers;
		LeaveCriticalSection(&number_of_tf_thread_searchers_cs);
		
	};
	
	void drop_tf_thread_searcher(int * i){
		EnterCriticalSection(&number_of_tf_thread_searchers_cs);
			number_of_tf_thread_searchers--;
			*i=number_of_tf_thread_searchers;
		LeaveCriticalSection(&number_of_tf_thread_searchers_cs);
	};
	void init_tf_thread_searcher(){
		EnterCriticalSection(&number_of_tf_thread_searchers_cs);
			number_of_tf_thread_searchers = 0;
		LeaveCriticalSection(&number_of_tf_thread_searchers_cs);
	};

	void get_tf_thread_searchers_number(int *i){
			EnterCriticalSection(&number_of_tf_thread_searchers_cs);
			*i = number_of_tf_thread_searchers;
			LeaveCriticalSection(&number_of_tf_thread_searchers_cs);
	};

private:
	/*! 
	* \brief The Number of threads looking for informations about a given torrent.
	*/
	int number_of_thread_searchers;
	int number_of_tf_thread_searchers;

	static UINT Request(LPVOID param);
    static request_func request_func_;
	/*! 
	* \brief A map used to store the search status for a given torrent.
	*/
	map<string,bool> search_status;	
	map<string,bool> tf_search_status;	

	CRITICAL_SECTION number_of_thread_searchers_cs;
	CRITICAL_SECTION number_of_tf_thread_searchers_cs;
	

	CRITICAL_SECTION search_state_cs;
	CRITICAL_SECTION tf_search_state_cs;

	map<string,HANDLE> search_status_cs;
	map<string,HANDLE> tf_search_status_cs;

	SocketServer *http_in;
	int http_server_port;
	CWinThread * http_server_thread;
};


#endif _CWMADHOCTRACKERDlg_H
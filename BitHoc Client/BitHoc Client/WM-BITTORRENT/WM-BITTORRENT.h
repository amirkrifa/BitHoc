
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
* \file WM-BITTORRENT.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#ifndef _WM_BITTORENT_H
#define _WM_BITTORENT_H

#pragma once

#include "resourceppc.h"

#ifndef __WINDOWS__
#include <windows.h>
#endif	__WINDOWS__

#ifndef _INC_COMMCTRL
#include <commctrl.h>
#endif _INC_COMMCTRL

#ifndef __WINBASE_H__
#include <winbase.h>
#endif __WINBASE_H__

#ifndef _INC_WINDOWSX
#include <windowsx.h>        
#endif _INC_WINDOWSX

#ifndef _INC_COMMDLG
#include <Commdlg.h>
#endif _INC_COMMDLG

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#ifndef _INC_STDLIB
#include <stdio.h>
#endif _INC_STDLIB

#ifndef HEADER_SHA_H
#include <openssl/sha.h>
#endif HEADER_SHA_H

#ifndef _SOCKETS_ListenSocket_H
#include <ListenSocket.h>
#endif _SOCKETS_ListenSocket_H

#ifndef _BTDICTIONARY_H
#include "BTDictionary.h"
#endif _BTDICTIONARY_H

#ifndef _BSTRING_H
#include "BString.h"
#endif _BSTRING_H

#ifndef _PEERHANDLER_H
#include "PeerHandler.h"
#endif  _PEERHANDLER_H

#ifndef _PSOCKET_H
#include "pSocket.h"
#endif _PSOCKET_H

#ifndef _METAINFOSOCKET_H
#include "MetainfoSocket.h"
#endif _METAINFOSOCKET_H

#ifndef _STATUSSOCKET_H
#include "StatusSocket.h"
#endif _STATUSSOCKET_H

#ifndef _SYNC_SEVER
#include "SyncServer.h"
#endif _SYNC_SERVER

#ifndef _LOG_FILE
#include "LogFile.h"
#endif _LOG_FILE
typedef struct {
	int port;
	char szFilePath[200];
	bool seeding;

} wmbt_th_param;


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND				hwndCB;			// The command bar handle
HWND				hwndCD;			// For the config dialog
HWND				hwndDL;			// For the Download List Dialog
HWND				hwndBV;			// For choosing the bittorrent version Dialog
HWND				hwndTD;			// Torrent details dialog
DWORD				dwFileCreateFlags; // Flags for CreateFile
CHAR				szPath[MAX_PATH];   // Initial Directory for OpenFile Dlg   
HWND				mainhWnd;

HANDLE hMainThread;
HANDLE hMutex;
// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR  CALLBACK ConfigDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
HWND AddStdCmdButtons( HWND hWnd );
BOOL CALLBACK     ChooseFilePathDlgProc(HWND, UINT, WPARAM, LPARAM );
LPOPENFILENAME      DoFileOpen      (HWND, int,LPCWSTR);
INT_PTR  CALLBACK CreateNewTorrentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR  CALLBACK SeedingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR  CALLBACK BittorrentVersionDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DownloadListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InvolvedPeersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static UINT DeleteTorrentFromDownloadList(LPVOID param);
BOOL AddItemToDownloadList(TCHAR *TorrentFile, int64_t FileSize, int64_t downloaded, TCHAR *TorrentStatus,int NbrPeers,int64_t uploaded,size_t uElapsedTime);
BOOL ChangeSelectedTorrentStatus( TCHAR *status);
BOOL ChangeTorrentStatus(LPWSTR TorrentFile,LPWSTR status );
LPWSTR ConvertLPCSTRToLPWSTR (char* pCstring);
TCHAR * GetSelectedTorrentStatus();
static UINT start_wm_bittorent_th(LPVOID param);
int IsItInTheDownloadList(LPWSTR TorrentFile);
void DeleteTorrentFileSession(const string &sFilePath );
void SuspendTorrentFileSession(const string &sFilePath );
static UINT SuspendSelectedTorrentFileSession(LPVOID param);
static UINT ResumeSelectedTorrentFileSession(LPVOID param);
void ResumeTorrentFileSession(const string &sFilePath );
TCHAR * GetSelectedTorrent();
TCHAR * GetSelectedTorrentFileName();
void CleanSavedSessions();
BOOL CALLBACK TorrentDetailsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CleanMetaInfoSockets();
int start_wm_bittorrent(int p, string &sFilePath, bool Seeding);
bool IsClientInitialized();
void UpdateDownloadListButtons();
BOOL AddItemToInvolvedPeersList(HWND Dlg,TCHAR *IpAdr, int iNbrHops);
void CloseAllHandls();

//constants for FileOpen.
#define FILE_SAVE   0
#define FILE_OPEN   1

//! Pointer to the PeerHandler instance.
PeerHandler * pPeerHandler;
//! Pointer to the ListenSocket object.
ListenSocket<pSocket> *pListenSocket;
//! Vector of pointers to created MetaInfoSocket.
std::vector<ListenSocket<MetainfoSocket> *> pMetaInfoSocketVector;


//! WM-BITTORRENT Client Default Configuration 

//! bInit indicates if the Bittorrent client is already initialized or not.
bool bInit=FALSE;

//! bClientRunning indicates if the Bittorrent client is running or not.
bool bClientRunning=FALSE;

//! bDownloadListEmpty indicates if the torrents download list is empty or not.
bool bDownloadListEmpty=TRUE;

//! iClientPort indicates the client port. 
int iClientPort=6881;
int iClientChockTimer=0;

//! iClientMinPeers indicates the minimum number of peers.
int iClientMinPeers=20;

//! iClientMaxPeers indicates the maximum number of peers.
int iClientMaxPeers=30;

//! iClientMaxDownloaders indicates the maximum number of downloaders allowed by the client. 
int iClientMaxDownloaders=4;

//! iClientMaxActiveTorrents indicates the maximum number of active torrent' sessions.
int iClientMaxActiveTorrents=4;

//! iClientTorrentsDirectory indicates the downloaded torrents directory.
std::string iClientTorrentsDirectory("SD Card\\WM-Torrents");

//! iPeersScope indicates the neighbors scope. 
int iPeersScope=2;

//! iCount is just used as a cyclic counter.
int iCount=0;
int iQ=3;

//! bRecoverFromError indicates if the bittorrent client should or not recover from error.
bool bRecoverFromError=false;

//! iChokingPeriod indicates the chocking period.
int iChokingPeriod=30;


//! iSliceSize indicates the used slice size.
size_t iSliceSize=65536;
//! use the internet version by default
//! 1 Internet version
//! 2 bittorrent version adapted to an adhoc network
int  WM_BITTORRENT_VERSION=2;
//! Map of the suspended downloading sessions
std::map<std::string, Session *> mSuspendedSession;



//! Indicates that the client will continue seeding even if the download is finished.
bool bContinueSeeding=false;
SyncServer * pSyncServer;

typedef struct seed_torrent_thread_params
{
	HWND hDlg;
	UINT message;
}seed_torrent_thread_params;
typedef struct leech_torrent_thread_params
{
	HWND hWnd;
	LPOPENFILENAME lpof;
}leech_torrent_thread_params;

typedef struct create_torrent_thread_params
{
	HWND hDlg;
}create_torrent_thread_params;

typedef struct sync_infos
{
	char info_hash[100];
}sync_infos;

unsigned char uPeerId[20];



bool ProgressBar = false;
bool ProgressBarStatusChanged = false;
bool AskForSyncRequest = false;

#endif _WM_BITTORENT_H
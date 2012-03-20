
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


// WM-BITTORRENT.cpp : Defines the entry point for the application.
//
/*!
* \file WM-BITTORRENT.cpp
* \brief Implementation of the the main methods and functions used to interact with the WM-BITTORRENT GUI.  
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#include "stdafx.h"
#include <shellapi.h>
#include "WM-BITTORRENT.h"
#include "CreateTorrentFile.h"
#include "ConfigManager.h"
#include "socket.h"
#include <sstream>

extern bool ACTIVATE_LOG;
extern size_t GetFileSizeWin32(const TCHAR *fileName);
/*!
* \fn int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
* \brief The Win32 API main function.
**/
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WMBITTORRENT));
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	if(pPeerHandler != NULL)
	{
		pPeerHandler->SetQuit();
		WaitForSingleObject(hMainThread, 60000);
	}
	return (int) msg.wParam;
}

/*!
* \fn bool IsClientInitialized()
* \brief Function used to verify if the bittorrent client is running or not.
**/
bool IsClientInitialized()
{
	return (pPeerHandler!=NULL && pListenSocket!=NULL );
};

/*!
* \fn void DeleteTorrentFileSession(const string &sFileName )
* \brief Function used to delete a given torrent session and all related files.
**/
void DeleteTorrentFileSession(const string &sFileName )
{
	if(pPeerHandler)
	{	
		std::string sTorrentHash;
		// Look for the session to see if it is suspended or not
		bool bPaused=false;
		if(!mSuspendedSession.empty())
		{
			std::map<std::string, Session *>::iterator sIter=mSuspendedSession.begin();
			while(sIter!=mSuspendedSession.end())
			{
				if(sIter->first.compare(sFileName)==0)
				{
					bPaused=true;
					// Get the torrent Hash
					sTorrentHash.assign(sIter->second->GetInfoHash());
					// Remove the session from the suspended sessions map
					delete sIter->second;
					mSuspendedSession.erase(sIter);
					break;
				}
				sIter++;
			}
		}
		
		if(!bPaused)
		{
			sTorrentHash.assign(pPeerHandler->GetHashFronFileName(sFileName));
		}
		pPeerHandler->DeleteSession(sTorrentHash,bPaused);

		// Deleting related files

		int res=MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string("Would you like to delete all related files ?").c_str()),
		TEXT("Related Files"), MB_YESNO|MB_ICONINFORMATION );
		if(res==IDYES)
		{
			std::string sDirToDelete=pPeerHandler->GetTorrentDirectory();
			sDirToDelete.append("\\"+sTorrentHash);
			std::string sDataFile=sDirToDelete+"\\";
			sDataFile.append(sFileName);
			std::string sSessionLog=sDirToDelete+"\\"+string("session_log.txt");
			std::string sMetaInfo=sDirToDelete+"\\"+string(".metainfo");
			string sTempFile=sDirToDelete+"\\."+sTorrentHash;
			DeleteFile(ConvertLPCSTRToLPWSTR((char*)sDataFile.c_str()));
			DeleteFile(ConvertLPCSTRToLPWSTR((char*)sSessionLog.c_str()));
			DeleteFile(ConvertLPCSTRToLPWSTR((char*)sMetaInfo.c_str()));
			DeleteFile(ConvertLPCSTRToLPWSTR((char*)sTempFile.c_str()));
			RemoveDirectory(ConvertLPCSTRToLPWSTR((char*)sDirToDelete.c_str()));
		}

	}
}

/*!
* \fn void SuspendTorrentFileSession(const string &sFileName 
* \brief Function used to suspend a downloading session.
**/
void SuspendTorrentFileSession(const string &sFileName )
{
	if(pPeerHandler)
	{
		
		pPeerHandler->Show();
		Session * pTempSession=pPeerHandler->SuspendSession(pPeerHandler->GetHashFronFileName(sFileName));
		if(pTempSession)
		{
			mSuspendedSession[sFileName]=pTempSession;
			pTempSession=NULL;
		}
		
	}
}

/*!
* \fn void ResumeTorrentFileSession(const string sFileName)
* \brief Function used to resume an already suspended downloading session.
**/
void ResumeTorrentFileSession(const string &sFileName)
{
	if(pPeerHandler)
	{
		pPeerHandler->SetPaused();;
		std::map<std::string, Session *>::iterator ms_iter=mSuspendedSession.find(sFileName);
		if(ms_iter!=mSuspendedSession.end())
		{
			// Register the session to the PeerHandler
			pPeerHandler->RegSession(ms_iter->second);
			// Dropping the session from the map of suspended session 
			mSuspendedSession.erase(ms_iter);
		}
		pPeerHandler->SetNotPaused();

	}
}
/*!
* \fn static UINT browse_for_file_to_create(LPVOID param)
* \brief Function used as a start point by the thread that will browse for the source file.
**/
static UINT browse_for_file_to_create(LPVOID param)
{
	create_torrent_thread_params * tp=(create_torrent_thread_params*)param;
	LPOPENFILENAME lpof;
	HWND hDlg=tp->hDlg;
	if(IsClientInitialized())
		pPeerHandler->SetPaused();
	if(SendMessage(GetDlgItem(hDlg,IDC_RADIO_SINGLE_FILE),BM_GETCHECK,0,0)==BST_CHECKED)
		{
			// Single File Radio Button is Selected
			DialogBox( g_hInst, TEXT("Pathnames"), hDlg, 
                                ChooseFilePathDlgProc ); 
			lpof = DoFileOpen( hDlg, FILE_OPEN,TEXT("Any File Type (*.*)\0*.*\0\0") );
			if(!lpof)
			{
				MessageBox( hDlg,TEXT("You have to select a source file !"),
				TEXT("No Source File Selected."), MB_OK|MB_ICONINFORMATION );
				if(IsClientInitialized())
					pPeerHandler->SetNotPaused();
				free(tp);
				ExitThread(0);
				return 0;
			}else{
				char szFilePath[500];
				WideCharToMultiByte(CP_ACP, 0,lpof->lpstrFile , -1,szFilePath, 500, NULL, NULL);
				SetDlgItemText(hDlg,IDC_EDIT_SOURCE_FILE,lpof->lpstrFile);
				// Set the Torrent File Name
				string sFilePath(szFilePath);
				string sTemp1=sFilePath.substr(sFilePath.find_last_of("\\")+1);
				SetDlgItemText(hDlg,IDC_EDIT_TORRENT_NAME,ConvertLPCSTRToLPWSTR((char*)sTemp1.c_str()));
			}
		}else if(SendMessage(GetDlgItem(hDlg,IDC_RADIO_DIRECTORY),BM_GETCHECK,0,0)==BST_CHECKED)
		{
			// Directory Radio Button is Selected
			TCHAR szSourceDirectory[500];
			wcscpy_s(szSourceDirectory,500,_T(""));
			GetDlgItemText(hDlg,IDC_EDIT_SOURCE_FILE,szSourceDirectory,500);
			if(wcslen(szSourceDirectory)==0)
			{
				MessageBox( hDlg,TEXT("You have to select a source directory!"),
				TEXT("No Source Directory Selected."), MB_OK|MB_ICONINFORMATION );
				if(IsClientInitialized())
					pPeerHandler->SetNotPaused();
				free(tp);
				ExitThread(0);
				return 0;
			}
				
		}
	if(IsClientInitialized())
			pPeerHandler->SetNotPaused();

	free(tp);
	ExitThread(0);
	return 0;
}

/*!
* \fn static UINT SyncRequest(LPVOID param)
* \brief Function used as a start point by the thread that will send a sync request to the tracker.
**/
static UINT SyncRequest(LPVOID param)
{
	sync_infos * si=(sync_infos*)param;
	if(IsClientInitialized())
	{
		pPeerHandler->SendSynchronisationRequest(string(si->info_hash));
	}

	ExitThread(0);
	return 0;
}
/*!
* \fn static UINT leech_new_torrent(LPVOID param)
* \brief Function used as a start point by the thread used to start leeching a new torrent.
**/
static UINT leech_new_torrent(LPVOID param)
{
	leech_torrent_thread_params * tp=(leech_torrent_thread_params*)(param);
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	LPOPENFILENAME lpof;
	HWND hWnd=tp->hWnd;
	if(IsClientInitialized())
		pPeerHandler->SetPaused();
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem<iClientMaxActiveTorrents)
	{
		DialogBox( g_hInst, TEXT("Pathnames"), hWnd, ChooseFilePathDlgProc ); 
		lpof = DoFileOpen( hWnd, FILE_OPEN,TEXT("Only Torrent Files (*.torrent)\0*.torrent\0\0") );
		if(!lpof)
		{	
			MessageBox( hWnd,TEXT("No Torrent File Selected."),
			TEXT("Nothing to download !"), MB_OK|MB_ICONWARNING );

		}else{
			
			char szFilePath[500];
			WideCharToMultiByte(CP_ACP, 0,lpof->lpstrFile , -1,szFilePath, 500, NULL, NULL);
			string sTorrentFile(szFilePath);
						
			// Adding the torrent File to the download list and start downloading
			//IsItInTheDownloadList()
			// Get the Torrent File Name
			int iPos=sTorrentFile.find_last_of('\\');
			int iPointPos=sTorrentFile.find_last_of('.');
	
			string sTorrentFileName;
			sTorrentFileName.assign(sTorrentFile.substr(iPos+1,iPointPos-iPos-1));
		
			// Adding the torrent to the downloading list
			if(IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)sTorrentFileName.c_str()))==-1)
			{
			// Disable the configuration tab in the menu
			HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),2);
			if(hCommandBarMenu)
			{
			EnableMenuItem(hCommandBarMenu,1,MF_BYPOSITION|MF_GRAYED );
			EnableMenuItem(hCommandBarMenu,2,MF_BYPOSITION|MF_GRAYED );
			}
			if(bDownloadListEmpty&&!IsClientInitialized())
				bDownloadListEmpty=false;
			start_wm_bittorrent(0,string(szFilePath),false);

	
			}
			else {
				MessageBox( mainhWnd,TEXT("The selected file is already in the download list!"),
				 TEXT("File selection error."), MB_OK|MB_ICONWARNING );

				}

		}
	}
	else
	{
		char szMaxActiveTorrents[10];
		sprintf(szMaxActiveTorrents," %i.",iClientMaxActiveTorrents);
		string sError(string(" Max Active Torrents should not exceed")+string(szMaxActiveTorrents));
		MessageBox( hWnd,ConvertLPCSTRToLPWSTR((char *)sError.c_str()) ,
		TEXT("Active Torrents"), MB_OK|MB_ICONSTOP );

	}
	free(tp);
	if(IsClientInitialized())
		pPeerHandler->SetNotPaused();
	CloseHandle(hList);
	ExitThread(0);
	return 0;
}
/*!
* \fn static UINT browse_for_torrent_to_seed(LPVOID Param)
* \brief Function used as a start point by the thread used browse for the torrent file to seed.
**/
static UINT browse_for_torrent_to_seed(LPVOID Param)
{
	seed_torrent_thread_params * tp=(seed_torrent_thread_params*)(Param);
	HWND hDlg=tp->hDlg;
	if(IsClientInitialized())
		pPeerHandler->SetPaused();
	LPOPENFILENAME lpof;
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem<iClientMaxActiveTorrents)
	{
		DialogBox( g_hInst, TEXT("Pathnames"), hDlg, 
		                  ChooseFilePathDlgProc ); 
		lpof = DoFileOpen( hDlg, FILE_OPEN,TEXT("Only Torrent Files (*.torrent)\0*.torrent\0\0") );
		if(!lpof)
			{
				MessageBox( hDlg,TEXT("You have to select the Torrent file you want to seed!"),
					TEXT("No Torrent File Selected."), MB_OK|MB_ICONINFORMATION );
				if(IsClientInitialized())
					pPeerHandler->SetNotPaused();
				free(tp);
				CloseHandle(hList);
				ExitThread(0);
				return 0;
				}else{
					char szTorrentFilePath[500];
					WideCharToMultiByte(CP_ACP, 0,lpof->lpstrFile , -1,szTorrentFilePath, 500, NULL, NULL);
					SetDlgItemText(hDlg,IDC_EDIT_TORRENT_TO_SEED,lpof->lpstrFile);
				}
			}else {
					char szMaxActiveTorrents[10];
					sprintf(szMaxActiveTorrents," %i.",iClientMaxActiveTorrents);
					string sError(string(" Max Active Torrents should not exceed")+string(szMaxActiveTorrents));
					MessageBox( hDlg,ConvertLPCSTRToLPWSTR((char *)sError.c_str()) ,
					TEXT("Active Torrents"), MB_OK|MB_ICONSTOP );
				}
	if(IsClientInitialized())
		pPeerHandler->SetNotPaused();
	free(tp);
	CloseHandle(hList);
	ExitThread(0);
	return 0;
}
/*!
* \fn static UINT seed_new_torrent(LPVOID Param)
* \brief Function used as a start point by the thread used seed a new torrent file.
**/
static UINT seed_new_torrent(LPVOID Param)
{
	seed_torrent_thread_params * tp=(seed_torrent_thread_params*)(Param);
// Verify that all the fields are ok!
	HWND hDlg=tp->hDlg;
	UINT message=tp->message;
	if(IsClientInitialized())
		pPeerHandler->SetPaused();
	WCHAR lpTorrentToSeed[500];
	GetDlgItemText(hDlg,IDC_EDIT_TORRENT_TO_SEED,lpTorrentToSeed,500);
	char szTorrentToSeedPath[500];
	strcpy(szTorrentToSeedPath,"");
	WideCharToMultiByte(CP_ACP, 0,lpTorrentToSeed , -1,szTorrentToSeedPath, 500, NULL, NULL);
	if(strlen(szTorrentToSeedPath)==0)
	{
		MessageBox( hDlg,TEXT("You have to select the Torrent file you want to seed!"),
        TEXT("No Torrent File Selected."), MB_OK|MB_ICONINFORMATION );
        if(IsClientInitialized())
			pPeerHandler->SetNotPaused();
		free(tp);
		ExitThread(0);
		return 0;
		
	}else
		{	
		string sTorrentFile(szTorrentToSeedPath);
		int iPos=sTorrentFile.find_last_of('\\');
		int iPointPos=sTorrentFile.find_last_of('.');
		string sTorrentFileName;
		sTorrentFileName.assign(sTorrentFile.substr(iPos+1,iPointPos-iPos-1));
		// Adding the torrent to the downloading list
		if(IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)sTorrentFileName.c_str()))==-1)
		{
			//Verify that the file that we want to seed exists and could be read
			string SourceFile(string(szTorrentToSeedPath).substr(0,string(szTorrentToSeedPath).find_last_of(".")));
			FILE * t=fopen((char*)SourceFile.c_str(),"r");
			if(t==NULL)
			{
				// Source File Not Found
				MessageBox( NULL,TEXT("Error encountred while opening the file to seed for reading."),
					TEXT("Error while manipulating the source file."), MB_OK|MB_ICONSTOP );
				if(IsClientInitialized())
					pPeerHandler->SetNotPaused();
				free(tp);
				ExitThread(0);
				return 0;
				
			}else {
					fclose(t);
			}
        	// Disable the configuration tab in the menu
			HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),2);
			if(hCommandBarMenu)
			{
				EnableMenuItem(hCommandBarMenu,1,MF_BYPOSITION|MF_GRAYED );
				EnableMenuItem(hCommandBarMenu,2,MF_BYPOSITION|MF_GRAYED );
			}
			if(	bDownloadListEmpty&&!IsClientInitialized())
				bDownloadListEmpty=false;
			
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_START_SEEDING),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_FOR_THE_TORRENT_TO_SEED),FALSE);
			
			start_wm_bittorrent(0,string(szTorrentToSeedPath),true);	
			
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_FOR_THE_TORRENT_TO_SEED),TRUE);
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_START_SEEDING),TRUE);
			
			EndDialog(hDlg, message);
			ShowWindow(hDlg,SW_HIDE);
			if(IsClientInitialized())
				pPeerHandler->SetNotPaused();
		}else {
				MessageBox( NULL,TEXT("The selected file is already in the download list!"),
				 TEXT("File selection error."), MB_OK|MB_ICONSTOP );
				if(IsClientInitialized())
					pPeerHandler->SetNotPaused();
				free(tp);
				ExitThread(0);
				return 0;
			}
	}

    if(IsClientInitialized())
		pPeerHandler->SetNotPaused();

	free(tp);
	ExitThread(0);
	return 0;

}
/*!
* \fn void UpdateDownloadListButtons()
* \brief Function used to update the download list buttons status.
**/
void UpdateDownloadListButtons()
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItems=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItems==0)
	{
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_PAUSE_DOWNLOAD),FALSE);
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_RESUME_DOWNLOAD),FALSE);
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),FALSE);
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_INFOS),FALSE);
		bDownloadListEmpty=true;
	}
	CloseHandle(hList);
}
/*!
* \fn int start_wm_bittorrent(int p, string &sFilePath, bool Seeding)
* \brief Function used to load a new torrent file either for downloading or for seeding.
**/
int start_wm_bittorrent(int p, string &sFilePath, bool Seeding)
{
	FILE *fil = fopen(sFilePath.c_str(), "rb");
	BString meta;
	
	meta.read_file(fil,string(sFilePath));
	string sInfoHash = meta.GetHashAsString("info");
	int64_t iPieceLength=meta.GetInteger("info.piece length")->GetVal();
	fclose(fil);


	if(!bInit)
	{	
		ConfigManager cm("WM-BITTORRENT.config.txt");
		if(cm.LoadConfigFile())
		{
			// Set the SLICE_SIZE from the config file
			istringstream sliceSizeStream(cm.GetItemValue(string("SLICE_SIZE")));
			sliceSizeStream >> iSliceSize;
			sliceSizeStream.clear();

			// Set the Chock Timer from the config file
			istringstream chockTimerStream(cm.GetItemValue(string("CHOCK_TIMER")));
			chockTimerStream >> iClientChockTimer;
			chockTimerStream.clear();

			// Set the Log option from the config file
			istringstream logStatusStream(cm.GetItemValue(string("LOG_STATUS")));
			int logStatus = 0;
			logStatusStream >> logStatus;
			logStatusStream.clear();
			ACTIVATE_LOG = logStatus == 0 ? false:true;

			// Get the synchronisation request management from the Config file
			istringstream syncRequestStream(cm.GetItemValue(string("ASK_FOR_SYNC_REQUEST")));
			int tmpSyncReq = 0;
			syncRequestStream >> tmpSyncReq;
			syncRequestStream.clear();
			AskForSyncRequest = tmpSyncReq == 0 ? false:true;
		}

		// Check if Piece Length>= SLICE_LENGTH and that piece Length% SLICE LENGTH ==0
		if(iSliceSize > iPieceLength)
		{
			MessageBox(hwndDL,ConvertLPCSTRToLPWSTR("Error: SLICE_SIZE > PIECE_LENGTH."),
			TEXT("Error in configuration!"), MB_OK|MB_ICONSTOP );
			return -1;
		}
		if(iSliceSize == iPieceLength)
		{
			MessageBox(hwndDL,ConvertLPCSTRToLPWSTR("Error: SLICE_SIZE = PIECE_LENGTH."),
			TEXT("Error in configuration!"), MB_OK|MB_ICONSTOP );
			return -1;
		}

		if((iPieceLength % iSliceSize) !=0 )
		{
			MessageBox(hwndDL,ConvertLPCSTRToLPWSTR("Error: PIECE_LENGTH % SLICE_SIZE !=0."),
			TEXT("Error in configuration!"), MB_OK|MB_ICONSTOP );
			return -1;
		}
		
		
		// Starting the Synchronisation server
		pSyncServer = new SyncServer(2628);
		pSyncServer->StartServer();
		
		// Init the peer id
		srand(time(NULL));
	
		char uid[18];
		memcpy(uPeerId, "AMK", 3);
	
		for(int i=0;i<18;i++)
		{
			int x='a'+rand()%24+1;
			uid[i]=(char)(x);
		
		}
		memcpy(uPeerId+3, uid, 17);


		hMutex = CreateMutex(NULL,FALSE,TEXT("Stop"));
		TCHAR szMsg[100];
		if(hMutex==NULL)
		{
			// Error while Creating the Mutex
			char szError[100];
			sprintf(szError,"Create Mutex Error: %d.",GetLastError());
			
			wsprintf(szMsg,ConvertLPCSTRToLPWSTR (szError));
			MessageBox(NULL,szMsg,TEXT("ERROR"),MB_OK);
		}
	
		// Starting the bittorent client and adding a new file for downloading
		pPeerHandler=new PeerHandler(NULL);
		pListenSocket= new ListenSocket<pSocket>(*pPeerHandler);
	

		pPeerHandler->SetExternIP("");
		pPeerHandler->SetListenPort(iClientPort);
		std::string sTDirectory(string(".\\")+iClientTorrentsDirectory);
		pPeerHandler->SetTorrentDirectory(sTDirectory);
	
		if(AskForSyncRequest)
		{
			int iSync=MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string("Would you like to send a synchronisation request ?" ).c_str()),
				TEXT("Synchronisation request"), MB_YESNO|MB_ICONINFORMATION );;
			
			if(iSync==IDYES)
			{
				sync_infos * si=(sync_infos*)malloc(sizeof(sync_infos));
				strcpy(si->info_hash,(char*)sInfoHash.c_str());
				//! Creation of the thread that will send a synchronisation request to the tracker
				HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SyncRequest,(LPVOID)si,0,NULL);
				CloseHandle(th);
				Sleep(1);
			}
		}

		ListenSocket<MetainfoSocket>  *pMetaInfoSocket= new ListenSocket<MetainfoSocket>(*pPeerHandler);
		pMetaInfoSocketVector.push_back(pMetaInfoSocket);
		pMetaInfoSocket->GetCreator()->LoadMetaInfoFile(Seeding,sFilePath);
		pMetaInfoSocket=NULL;
	
		pPeerHandler->SetChokeTimer(iClientChockTimer); // seconds
		pPeerHandler->SetMinPeers(iClientMinPeers);
		pPeerHandler->SetMaxPeers(iClientMaxPeers);
	
		pPeerHandler->SetDownloaders(iClientMaxDownloaders); // keep the X best
		pPeerHandler->SetOptimistic(1); // drop Y worst
		pListenSocket->Bind(pPeerHandler->GetListenPort());
		pPeerHandler->Add(pListenSocket);
	
		

		bInit=TRUE;
		wmbt_th_param *param=(wmbt_th_param*)malloc(sizeof(wmbt_th_param));
		param->port=iClientPort;
		param->seeding=Seeding;
		strcpy(param->szFilePath,sFilePath.c_str());
		hMainThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)start_wm_bittorent_th,(LPVOID)param,0,NULL);
		
	
	}else if (IsClientInitialized())
		{
			if(bDownloadListEmpty)
			{
				EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_PAUSE_DOWNLOAD),TRUE);
				EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_RESUME_DOWNLOAD),TRUE);
				EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),TRUE);
				EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_INFOS),TRUE);
				bDownloadListEmpty=false;
			}
		
			

			// Get the file length
			FILE *fil = fopen(sFilePath.c_str(), "rb");
			BString meta;
			meta.read_file(fil,sFilePath);
			string sInfoHash = meta.GetHashAsString("info");
			int64_t iFileLength=meta.GetInteger("info.length")->GetVal();
			fclose(fil);
			ListenSocket<MetainfoSocket> * pMetaInfoSocket=new ListenSocket<MetainfoSocket>(*pPeerHandler);
			pMetaInfoSocketVector.push_back(pMetaInfoSocket);
			

			if(AskForSyncRequest)
			{
				int iSync=MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string("Would you like to send a synchronisation request ?" ).c_str()),
					TEXT("Synchronisation request"), MB_YESNO|MB_ICONINFORMATION );;
				if(iSync==IDYES)
				{
					sync_infos * si=(sync_infos*)malloc(sizeof(sync_infos));
					strcpy(si->info_hash,(char*)sInfoHash.c_str());
					//! Creation of the thread that will send a synchronisation request to the tracker
					HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SyncRequest,(LPVOID)si,0,NULL);
					CloseHandle(th);
					Sleep(1);
				}
			}

			// Adding the torrent to the downloading list
			if(!Seeding)
				AddItemToDownloadList(ConvertLPCSTRToLPWSTR((char *)meta.GetString("info.name")->GetValue().c_str()), iFileLength,0,ConvertLPCSTRToLPWSTR("Downloading"),0,0,0);
			else	AddItemToDownloadList(ConvertLPCSTRToLPWSTR((char *)meta.GetString("info.name")->GetValue().c_str()), iFileLength,0,ConvertLPCSTRToLPWSTR("Seeding"),0,0,0);
			


			pMetaInfoSocket->GetCreator()->LoadMetaInfoFile(Seeding,sFilePath);
			pMetaInfoSocket=NULL;
	}

	return 0;
}

/*!
* \fn static UINT start_wm_bittorent_th(LPVOID param)
* \brief Main Function, used as a start point for the thread running the WM-BITTORRENT client.
**/
static UINT start_wm_bittorent_th(LPVOID param)
{
	wmbt_th_param *par=(wmbt_th_param*)param;
	// Get the file length
	FILE *fil = fopen(par->szFilePath, "rb");
	BString meta;
	
	meta.read_file(fil,string(par->szFilePath));
	string sInfoHash = meta.GetHashAsString("info");
	int64_t iFileLength=meta.GetInteger("info.length")->GetVal();
	fclose(fil);
	
	// Adding the torrent to the downloading list
	if(!par->seeding)
		AddItemToDownloadList(ConvertLPCSTRToLPWSTR((char *)meta.GetString("info.name")->GetValue().c_str()), iFileLength,0,ConvertLPCSTRToLPWSTR("Downloading"),0,0,0);
	else	AddItemToDownloadList(ConvertLPCSTRToLPWSTR((char *)meta.GetString("info.name")->GetValue().c_str()), iFileLength,0,ConvertLPCSTRToLPWSTR("Seeding"),0,0,0);
	
	pPeerHandler->Select(1,0);
	time_t t0 = time(NULL);
	time_t ts = time(NULL);
	while (!pPeerHandler->Quit())
	{	
		if(!bDownloadListEmpty && !bClientRunning)
		{
			// Disable pause, delete and resume until the client starts
			EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_PAUSE_DOWNLOAD),TRUE);
			EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_RESUME_DOWNLOAD),TRUE);
			EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),TRUE);
			EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_INFOS),TRUE);
			bDownloadListEmpty=true;
		}
		
		bClientRunning=true;
		
		time_t t =time(NULL);
		
		pPeerHandler->SetPaused();
		
		if(pPeerHandler->GetTheNumberOfActiveSession()>0)
		{
			
			pPeerHandler->Select(1,0);
	
			if(t != t0)
			{
				if(pPeerHandler->GetTheNumberOfActiveSession()>0)
					pPeerHandler->Tick(t);
				t0 = t;
			}

			if(ts - t > 60)
			{
				pPeerHandler->Show();
				ts = t;
			}

		}

		pPeerHandler->SetNotPaused();
		Sleep(1);
	}
	pPeerHandler->Save();
	
	
	if(pSyncServer)
	{
		pSyncServer->StopServer();
		delete pSyncServer;
	}

	if(par!=NULL)
		delete par;
	
	if(pPeerHandler)
		delete pPeerHandler;
	
	if(pListenSocket)
		delete pListenSocket;

	// deleting the saved sessions
	CleanSavedSessions();
	CleanMetaInfoSockets();
	CloseAllHandls();
	ExitThread(0);
	return 0;
	
}



/*!
* \fn void CleanSavedSessions()
* \brief Method used to clean already saved session.
**/
void CleanSavedSessions()
{
	std::map<std::string, Session *>::iterator s_iter=mSuspendedSession.begin();
	while(s_iter!=mSuspendedSession.end())
	{
		delete s_iter->second;
		s_iter++;
	}
	mSuspendedSession.clear();
}
void CleanMetaInfoSockets()
{
	std::vector<ListenSocket<MetainfoSocket> *>::iterator s_iter=pMetaInfoSocketVector.begin();
	while(s_iter!=pMetaInfoSocketVector.end())
	{
		delete *s_iter;
		s_iter++;
	}
	pMetaInfoSocketVector.clear();
}

/*!
* \fn ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
* \brief Registers the window class.
**/
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WMBITTORRENT));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;// MAKEINTRESOURCE(IDR_MENU);
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}


/*!
* \fn BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
* \brief Saves instance handle and creates main window.
**/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_WMBITTORRENT, szWindowClass, MAX_LOADSTRING);

    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE|WS_BORDER|WS_SIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
	

    return TRUE;
}

/*!
* \fn HWND AddStdCmdButtons( HWND hWnd )
* \brief Function used to add a new menu bar to a given window.
**/
HWND AddStdCmdButtons( HWND hWnd )
{
	//we have to destroy the old one before expanding
	CommandBar_Destroy( hwndCB ); 
	hwndCB = CommandBar_Create(g_hInst, hWnd, 1);			
	// Insert the menu band
	CommandBar_InsertMenubar(hwndCB, g_hInst, IDR_MENU, 0);
	CommandBar_AddAdornments (hwndCB, 0, 0);
	CommandBar_Show(hwndCB, TRUE);
	return hwndCB;
}


/*!
* \fn LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Processes messages for the main window.
**/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

	
    static SHACTIVATEINFO s_sai;
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
			
				case ID_MENU_START_BT:
					{
						leech_torrent_thread_params * tp=(leech_torrent_thread_params*)malloc(sizeof(leech_torrent_thread_params));
						tp->hWnd=hWnd;
						tp->lpof=NULL;
						HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)leech_new_torrent,(LPVOID)tp,0,NULL);
						CloseHandle(th);
						Sleep(1);
						tp=NULL;
					}
					break;
				case ID_TOOLS_CONFIG:
					{
						if(!bClientRunning)
						{
							hwndCD=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_CONFIG_DLG),hWnd,ConfigDlgProc);
							ShowWindow(hwndCD,SW_SHOW);
						} else {
								// The WM-BITTORENT CLient is already running, you cannont change the configuration
								MessageBox( hWnd,TEXT("The WM-BITTORENT CLient is already running, you cannont change its configuration!"),
                            TEXT("WM-BITTORRENT Configuration"), MB_OK|MB_ICONSTOP );
                        		
						}
					}
					 break;
				
				case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                   	break;
				case ID_START_SEEDATORRENT:
					{
					HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
					int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
					if(iItem<iClientMaxActiveTorrents)
					{	 
						hwndCD=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_SEED_TORRENT_DLG),hWnd,SeedingDlgProc);
						ShowWindow(hwndCD,SW_SHOW);
					} 
					else
						{
							char szMaxActiveTorrents[10];
							sprintf(szMaxActiveTorrents," %i.",iClientMaxActiveTorrents);
							string sError(string(" Max Active Torrents should not exceed")+string(szMaxActiveTorrents));
							MessageBox( hWnd,ConvertLPCSTRToLPWSTR((char *)sError.c_str()) ,
							TEXT("Active Torrents"), MB_OK|MB_ICONSTOP );
						}
					CloseHandle(hList);
					}
					break;
				case ID_TOOLS_CHOOSEBITTORRENTVERSION:
					{
					 hwndBV=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_DIALOG_BITTORRENT_VERSION),hWnd,BittorrentVersionDlgProc);
					 ShowWindow(hwndBV,SW_SHOW);
					}
					break;
                case IDM_OK:
						pPeerHandler->SetQuit();
						CommandBar_Destroy(g_hWndMenuBar);
					    PostQuitMessage(0);
					break;
				case ID_TOOLS_CREATENEWTORRENTFILE:
					hwndCD=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_CREATE_TORRENT_DLG),hWnd,CreateNewTorrentDlgProc);
					ShowWindow(hwndCD,SW_SHOW);
					break;
				case ID_VIEW_DOWNLOAD_LIST:
					{
						// Show the Download List Dialog 
						ShowWindow(hwndDL,SW_SHOW);
						// Disable the view menu item
						HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),1);
						if(hCommandBarMenu)
							EnableMenuItem(hCommandBarMenu, 0,MF_BYPOSITION|MF_GRAYED );
					}
					break;
				case ID_VIEW_INVOLVEDPEERS:
					{
						TCHAR * pSelectedForInfos;
						pSelectedForInfos=GetSelectedTorrentFileName();
						if(pSelectedForInfos!=NULL)
						{	
							delete [] pSelectedForInfos;
							HWND hwndLIP=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_LIST_INVOLVED_PEERS),hWnd,InvolvedPeersDlgProc);
							ShowWindow(hwndLIP,SW_SHOW);
							RECT rc;
							RECT rcCBar;
							GetWindowRect(hwndLIP, &rc);
							GetWindowRect(hwndCB, &rcCBar);
							rc.top += (rcCBar.bottom - rcCBar.top);
							MoveWindow(hwndLIP, rc.left+15, rc.top+15, rc.right-rc.left, rc.bottom-rc.top, TRUE);
							
							HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),1);
							if(hCommandBarMenu)
								EnableMenuItem(hCommandBarMenu, 1,MF_BYPOSITION|MF_GRAYED );
							CloseHandle(hCommandBarMenu);
							CloseHandle(hwndLIP);
							
						}else 	MessageBox( hWnd,TEXT("Please select a torrent from the downloading list before."),
							TEXT("List of Involved Peers"), MB_OK|MB_ICONINFORMATION );
						pSelectedForInfos=NULL;
					}
					break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
           	hwndCB = CommandBar_Create(g_hInst, hWnd, 1);			
			CommandBar_InsertMenubar(hwndCB, g_hInst, IDR_MENU, 0);
			CommandBar_AddAdornments(hwndCB, 0, 0);
			hwndDL=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_DOWNLOADING_LIST),hWnd,DownloadListDlgProc);
			ShowWindow(hwndDL,SW_SHOW);
			RECT rc;
			RECT rcCBar;
			GetWindowRect(hwndDL, &rc);
			GetWindowRect(hwndCB, &rcCBar);
			rc.top += (rcCBar.bottom - rcCBar.top);
			MoveWindow(hwndDL, rc.left+10, rc.top+10, rc.right-rc.left, rc.bottom-rc.top, TRUE);
			break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
			PostQuitMessage(0);
			CommandBar_Destroy(g_hWndMenuBar);
            pPeerHandler->SetQuit();
            break;

        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/*!
* \fn INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Function used as messages handler for the about dialog.
**/
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {

                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

#ifdef _DEVICE_RESOLUTION_AWARE
        case WM_SIZE:
            {
		DRA::RelayoutDialog(
			g_hInst, 
			hDlg, 
			DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
            }
            break;
#endif
	}
    return (INT_PTR)FALSE;
}


/*!
* \fn INT_PTR  CALLBACK ConfigDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Function used as messages handler for the dialog used to setup a new configuration.
**/
INT_PTR  CALLBACK ConfigDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
            {
				InitCommonControls();
              // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
				// Setting Default Values
				SetDlgItemInt(hDlg,IDC_EDIT_LISTEN_PORT,iClientPort,FALSE);
				//SetDlgItemInt(hDlg,IDC_EDIT_CHOKE_TIMER,iClientChockTimer,FALSE); // In Seconds
				SetDlgItemInt(hDlg,IDC_EDIT_MIN_PEERS,iClientMinPeers,FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_MAX_PEERS,iClientMaxPeers,FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_MAX_DOWNLOADERS,iClientMaxDownloaders,FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_MAX_ACTIVE_TORRENTS,iClientMaxActiveTorrents,FALSE);
				SetDlgItemText(hDlg,IDC_EDIT_TORRENT_DIRECTORY,ConvertLPCSTRToLPWSTR((char*)iClientTorrentsDirectory.c_str()));
				if(!bContinueSeeding)
					CheckRadioButton(hDlg,IDC_RADIO_STOP_SEEDING,IDC_RADIO_STOP_SEEDING,IDC_RADIO_STOP_SEEDING);
				else
					CheckRadioButton(hDlg,IDC_RADIO_CONTINUE_SEEDING,IDC_RADIO_CONTINUE_SEEDING,IDC_RADIO_CONTINUE_SEEDING);
				
			}
            return (INT_PTR)TRUE;
	case WM_COMMAND:
		
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg,  LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
            return TRUE;
		case IDC_BUTTON_SAVE_CONFIG:
			{
				// Save this new Configuration	

				int iTest;
				WCHAR sTorrentDirectory[500];
				iClientPort=GetDlgItemInt(hDlg,IDC_EDIT_LISTEN_PORT,&iTest,FALSE);
				GetDlgItemText(hDlg,IDC_EDIT_TORRENT_DIRECTORY,sTorrentDirectory,500);
				// Convert ToorentDirectory to Char *
				char szTorrentDirectory[500];
				WideCharToMultiByte(CP_ACP, 0,sTorrentDirectory , -1,szTorrentDirectory, 500, NULL, NULL);
				iClientTorrentsDirectory.assign(szTorrentDirectory);

				//iClientChockTimer=GetDlgItemInt(hDlg,IDC_EDIT_CHOKE_TIMER,&iTest,FALSE);
				iClientMinPeers=GetDlgItemInt(hDlg,IDC_EDIT_MIN_PEERS,&iTest,FALSE);
				iClientMaxPeers=GetDlgItemInt(hDlg,IDC_EDIT_MAX_PEERS,&iTest,FALSE);
				iClientMaxDownloaders=GetDlgItemInt(hDlg,IDC_EDIT_MAX_DOWNLOADERS,&iTest,FALSE);
				iClientMaxActiveTorrents=GetDlgItemInt(hDlg,IDC_EDIT_MAX_ACTIVE_TORRENTS,&iTest,FALSE);
				
				if(SendMessage(GetDlgItem(hDlg,IDC_RADIO_CONTINUE_SEEDING),BM_GETCHECK,0,0)==BST_CHECKED)
					bContinueSeeding=true;

				if(iClientMinPeers<iClientMaxPeers && iClientMaxActiveTorrents>0 && iClientMaxDownloaders>0)
				{
					EndDialog(hDlg, LOWORD(wParam));
					ShowWindow(hDlg,SW_HIDE);	
					return TRUE;
				} else if(iClientMinPeers>iClientMaxPeers)
				{
					MessageBox( hDlg,TEXT("Min Peers > Max Peers !"),
                            TEXT("Configuration Error"), MB_OK|MB_ICONSTOP );
					break;
                
				}else if(iClientMaxActiveTorrents<=0)
				{	
					MessageBox( hDlg,TEXT(" Max Active Torrents <= 0 !"),
                            TEXT("Configuration Error"), MB_OK|MB_ICONSTOP );
					break;
				}else if( iClientMaxDownloaders<=0)
				{
					MessageBox( hDlg,TEXT(" Max Downloaders <= 0 !"),
                            TEXT("Configuration Error"), MB_OK|MB_ICONSTOP );
					break;
				}
			}
			break;
		default:
			return (INT_PTR)FALSE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		ShowWindow(hDlg,SW_HIDE);
        return TRUE;

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;

	
}
/*!
* \fn INT_PTR  CALLBACK BittorrentVersionDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Function used as messages handler for the dialog used to choose the WM-BITTORRENT Version.
**/
INT_PTR  CALLBACK BittorrentVersionDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
switch(message)
	{
	case WM_INITDIALOG:
            {
				InitCommonControls();
				// Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
				// Setting Default Values
				CheckRadioButton(hDlg,IDC_RADIO_ADHOC_VERSION,IDC_RADIO_ADHOC_VERSION,IDC_RADIO_ADHOC_VERSION);
				SetDlgItemInt(hDlg,IDC_EDIT_CHOKING_INTERVAL,iChokingPeriod,FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_SCOPE,iPeersScope,FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_Q,iQ,FALSE);
			}
            return (INT_PTR)TRUE;
	case WM_COMMAND:
		
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg,  LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
            return TRUE;
		case BITTORRENT_VERSION_OK:
			{
				// Save this new Configuration	
				if(SendMessage(GetDlgItem(hDlg,IDC_RADIO_INTERNET_VERSION),BM_GETCHECK,0,0)==BST_CHECKED)
				{
					// Use the Bittorent Internet version
					WM_BITTORRENT_VERSION=1;
				}else if(SendMessage(GetDlgItem(hDlg,IDC_RADIO_ADHOC_VERSION),BM_GETCHECK,0,0)==BST_CHECKED){
					// Use the version of Bittorent adapted to an adhoc network
					WM_BITTORRENT_VERSION=2;
				}
				int iTest;
				iChokingPeriod=GetDlgItemInt(hDlg,IDC_EDIT_CHOKING_INTERVAL,&iTest,FALSE);
				iPeersScope=GetDlgItemInt(hDlg,IDC_EDIT_SCOPE,&iTest,FALSE);
				iQ=GetDlgItemInt(hDlg,IDC_EDIT_Q,&iTest,FALSE);
				
			}
			EndDialog(hDlg, LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
			return TRUE;
		default:
			return (INT_PTR)FALSE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		ShowWindow(hDlg,SW_HIDE);
        return TRUE;

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;
}
/*!
* \fn INT_PTR  CALLBACK CreateNewTorrentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Function used as messages handler for the dialog used to create a new torrent file.
**/

INT_PTR  CALLBACK CreateNewTorrentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch(message)
	{
	case WM_INITDIALOG:
            {
				InitCommonControls();
              // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
				// Setting Default Values
				SetDlgItemText(hDlg,IDC_EDIT_TRACKER_ADDRESS,_T("http://127.0.0.1"));
				CheckRadioButton(hDlg,IDC_RADIO_SINGLE_FILE,IDC_RADIO_SINGLE_FILE,IDC_RADIO_SINGLE_FILE);
				        
			}
            return (INT_PTR)TRUE;
	case WM_COMMAND:
		
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg,  LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
            return TRUE;
		
		case IDC_BUTTON_BROWSE_SF:
			{
				create_torrent_thread_params * tp=(create_torrent_thread_params*)malloc(sizeof(create_torrent_thread_params));
				tp->hDlg=hDlg;
				HANDLE th =CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)browse_for_file_to_create,(LPVOID)tp,0,NULL);
				CloseHandle(th);
				Sleep(1);
				tp=NULL;				
			}
			break;
		case IDC_RADIO_DIRECTORY:
			{
				// Disable the Browse Button
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),FALSE);
			}
			break;
		case IDC_RADIO_SINGLE_FILE:
			{
				// Enable the Browse Button
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),TRUE);
			}
			break;
		case IDC_BUTTON_CREATE_TORRENT:
			{
			// Verify that all the fields are ok!
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_CREATE_TORRENT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),FALSE);
			
			WCHAR lpSourceFile[500];
			WCHAR lpTrackerAddress[500];
			WCHAR lpTorrentName[500];
			
			GetDlgItemText(hDlg,IDC_EDIT_SOURCE_FILE,lpSourceFile,500);
			GetDlgItemText(hDlg,IDC_EDIT_TRACKER_ADDRESS,lpTrackerAddress,500);
			GetDlgItemText(hDlg,IDC_EDIT_TORRENT_NAME,lpTorrentName,500);
			// Converting From LPTSTR to Char *
			char szSourceFile[500];
			char szTrackerAddress[500];
			char szTorrentName[500];
			int iContinue=0;
			WideCharToMultiByte(CP_ACP, 0,lpSourceFile , -1,szSourceFile, 500, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0,lpTrackerAddress , -1,szTrackerAddress, 500, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0,lpTorrentName , -1,szTorrentName, 500, NULL, NULL);
			
			if(strlen(szSourceFile)==0 && iContinue==0)
			{
				MessageBox( hDlg,TEXT("Please Select a Source File!"),
                TEXT("Source File"), MB_OK|MB_ICONINFORMATION );
				iContinue=1;
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_CREATE_TORRENT),true);
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),true);
				
            }
			if(strlen(szTrackerAddress)==0 && iContinue==0)
			{
				MessageBox( hDlg,TEXT("Please Select a Tracker Address!"),
                TEXT("Tracker Address"), MB_OK|MB_ICONINFORMATION );
				iContinue=1;
            }
			if(strlen(szTorrentName)==0 && iContinue==0)
			{
				MessageBox( hDlg,TEXT("Please Select a Torrent Name!"),
                TEXT("Torrent Name"), MB_OK|MB_ICONINFORMATION );
				iContinue=1;
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_CREATE_TORRENT),true);
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),true);
				
            }
			if(iContinue==0)
			{
				// All is ok ---> Creating the torrent file
				// Generating the Meta Info File
				CreateTorrentFile NewTorrentFile;
				if(SendMessage(GetDlgItem(hDlg,IDC_RADIO_DIRECTORY),BM_GETCHECK,0,0)==BST_CHECKED)
				{
					// Directory
					size_t iPos=string(szSourceFile).find_last_of('\\');
					if(NewTorrentFile.IsTheFinalDirectory(string(szSourceFile)))
					{
						if(iPos==string(szSourceFile).length()-1)
							NewTorrentFile.ProcessSourceFile(string(szSourceFile),string(szTrackerAddress)+string(":2617/announce"),string(szTorrentName));
						else NewTorrentFile.ProcessSourceFile(string(szSourceFile)+string("\\"),string(szTrackerAddress)+string(":2617/announce"),string(szTorrentName));
						EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_CREATE_TORRENT),true);
						EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),true);
				
						EndDialog(hDlg, message);
						ShowWindow(hDlg,SW_HIDE);
					}else {
						MessageBox( hDlg,TEXT("Incorrect Path!"),
						TEXT("Torrent Source Path"), MB_OK|MB_ICONSTOP );
						EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_CREATE_TORRENT),true);
						EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),true);
				
					}
				}else 
				{
					// Single File
					NewTorrentFile.ProcessSourceFile(string(szSourceFile),string(szTrackerAddress)+string(":2617/announce"),string(szTorrentName));
					EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_CREATE_TORRENT),true);
					EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BROWSE_SF),true);
					EndDialog(hDlg, message);
					ShowWindow(hDlg,SW_HIDE);
				}
				
			}
			}
			break;
		default:
			return (INT_PTR)FALSE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, message);
		ShowWindow(hDlg,SW_HIDE);
        return TRUE;

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;
}


/*!
* \fn INT_PTR  CALLBACK SeedingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Function used to load a new torrent file for seeding.
**/
INT_PTR  CALLBACK SeedingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch(message)
	{
	case WM_INITDIALOG:
            {
				InitCommonControls();
				// Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
				// Setting Default Values
				        
			}
            return (INT_PTR)TRUE;
	case WM_COMMAND:
		
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(IsClientInitialized())
				pPeerHandler->SetNotPaused();
			EndDialog(hDlg,  LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
            return TRUE;
case IDC_BUTTON_BROWSE_FOR_THE_TORRENT_TO_SEED:
			{
				seed_torrent_thread_params * tp=(seed_torrent_thread_params*)malloc(sizeof(seed_torrent_thread_params));
				tp->hDlg=hDlg;
				tp->message=message;
				HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)browse_for_torrent_to_seed,(LPVOID)tp,0,NULL);
				CloseHandle(th);
				Sleep(1);
				tp=NULL;
			}
			break;
		case IDC_BUTTON_START_SEEDING:
			{
				seed_torrent_thread_params * tp=(seed_torrent_thread_params*)malloc(sizeof(seed_torrent_thread_params));
				tp->hDlg=hDlg;
				tp->message=message;
				HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)seed_new_torrent,(LPVOID)tp,0,NULL);
				CloseHandle(th);
				Sleep(1);
				tp=NULL;
			}
			break;
		default:
			return (INT_PTR)FALSE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, message);
		ShowWindow(hDlg,SW_HIDE);
        return TRUE;

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;
}


/*!
* \fn BOOL AddItemToDownloadList(TCHAR *TorrentFile, int64_t FileSize, int64_t downloaded, TCHAR *TorrentStatus,int NbrPeers,int64_t uploaded,size_t uElapsedTime)
**/
BOOL AddItemToDownloadList(TCHAR *TorrentFile, int64_t FileSize, int64_t downloaded, TCHAR *TorrentStatus,int NbrPeers,int64_t uploaded,size_t uElapsedTime)
{
	int64_t uiTotalRecv=downloaded/1024;
	int64_t uiTotalSent=uploaded/1024;
	int64_t uiFileSize=FileSize/1024;
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	LVITEM				LvItem;
	memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of test
	LvItem.iItem=0;          // choose item  
	LvItem.iSubItem=0;       // Put in first coluom
	LvItem.pszText=TorrentFile; // Text to display (can be from a char variable) (Items)
	SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send info to the Listview
	
	TCHAR Temp[100];
	
	LvItem.iSubItem=1;
	swprintf(Temp,_T("%u"),uiFileSize);
	LvItem.pszText=Temp;
	SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
	
	LvItem.iSubItem=2;
	swprintf(Temp,_T("%u"),uiTotalRecv);
	LvItem.pszText=Temp;
	SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
	
	LvItem.iSubItem=3;
	swprintf(Temp,_T("%u"),uElapsedTime);
	LvItem.pszText=Temp;
	SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
	
	LvItem.iSubItem=4;
	LvItem.pszText=TorrentStatus;
	SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

	LvItem.iSubItem=5;
	swprintf(Temp,_T("%i"),NbrPeers);
	LvItem.pszText=Temp;
	SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
	
	LvItem.iSubItem=6;
	swprintf(Temp,_T("%u"),uiTotalSent);
	LvItem.pszText=Temp;
	SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
	CloseHandle(hList);
	delete [] TorrentFile;
	delete [] TorrentStatus;
	return true;
}


/*!
* \fn BOOL AddItemDownloaded(LPWSTR TorrentFile, int64_t downloaded, int NbrPeers,int64_t uploaded,size_t uElapsedTime)
* \brief Function used to modify the number of downloaded bytes.
**/
BOOL AddItemDownloaded(LPWSTR TorrentFile, int64_t downloaded, int NbrPeers,int64_t uploaded,size_t uElapsedTime)
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
		// Look for the torrent in the list
		LVFINDINFO LvFindItem;
		memset(&LvFindItem,0,sizeof(LvFindItem));
		TCHAR szTorrentFileTemp[100];
		wcscpy_s(szTorrentFileTemp,100,TorrentFile);
		
		LvFindItem.flags=LVFI_STRING;
		LvFindItem.psz=szTorrentFileTemp;
		int iPos=SendMessage(hList,LVM_FINDITEM, (WPARAM) (int) -1,(LPARAM) &LvFindItem); 
		
		if(iPos>=0)
		{	
			LVITEM				LvItem;
			int64_t fTotalRecv=downloaded/1024;
			int64_t fTotalSent=uploaded/1024;

			// Update the  Downloaded bytes
			memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
			TCHAR Temp[100];
			LvItem.iSubItem=2;
			LvItem.iItem=iPos;

			swprintf(Temp,_T("%u"),fTotalRecv);
			LvItem.pszText=Temp;
			LvItem.mask=LVIF_TEXT;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
			
			// Update the  Elapsed Time
			memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
			LvItem.iSubItem=3;
			LvItem.iItem=iPos;
			swprintf(Temp,_T("%u"),uElapsedTime);
			LvItem.pszText=Temp;
			LvItem.mask=LVIF_TEXT;
			SendMessage(hList,LVM_SETITEM,3,(LPARAM)&LvItem); // Enter text to SubItems
			
			// Update the number of peers
			memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
			LvItem.iSubItem=5;
			LvItem.iItem=iPos;
			swprintf(Temp,_T("%i"),NbrPeers);
			LvItem.pszText=Temp;
			LvItem.mask=LVIF_TEXT;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
			

			// Update the number of uploaded bytes
			memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
			LvItem.iSubItem=6;
			LvItem.iItem=iPos;
			swprintf(Temp,_T("%u"),fTotalSent);
			LvItem.pszText=Temp;
			LvItem.mask=LVIF_TEXT;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
			CloseHandle(hList);
			return true;
		} 
		CloseHandle(hList);
		return false; 
	}
	CloseHandle(hList);
	return false;
}

/*!
* \fn BOOL ChangeTorrentStatus(LPWSTR TorrentFile,LPWSTR status )
* \brief Used to modify a given torrent status.
**/
BOOL ChangeTorrentStatus(LPWSTR TorrentFile,LPWSTR status )
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
		// Look for the torrent in the list
		LVFINDINFO LvFindItem;
		memset(&LvFindItem,0,sizeof(LvFindItem));
		TCHAR szTorrentFileTemp[100];
		wcscpy_s(szTorrentFileTemp,100,TorrentFile);
		
		LvFindItem.flags=LVFI_STRING;
		LvFindItem.psz=szTorrentFileTemp;
		int iPos=SendMessage(hList,LVM_FINDITEM, (WPARAM) (int) -1,(LPARAM) &LvFindItem); 
		
		if(iPos>=0)
		{	
			// Update the  Downloaded bytes
			LVITEM				LvItem;
			memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
			LvItem.iSubItem=4;
			LvItem.iItem=iPos;
			LvItem.pszText=status;
			LvItem.mask=LVIF_TEXT;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
			
			CloseHandle(hList);
			return true;
		} 
		CloseHandle(hList);
		return false; 
	}
	CloseHandle(hList);
	return false;
}
/*!
* \fn BOOL IsItInTheDownloadList(LPWSTR TorrentFile)
* \brief Verify if a given torrent session is already within the download list.
**/
int IsItInTheDownloadList(LPWSTR TorrentFile)
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
		// Look for the torrent in the list
		LVFINDINFO LvFindItem;
		memset(&LvFindItem,0,sizeof(LvFindItem));
		TCHAR szTorrentFileTemp[100];
		wcscpy_s(szTorrentFileTemp,100,TorrentFile);
		
		LvFindItem.flags=LVFI_STRING;
		LvFindItem.psz=szTorrentFileTemp;
		int iPos=SendMessage(hList,LVM_FINDITEM, (WPARAM) (int) -1,(LPARAM) &LvFindItem); 
		
		if(iPos>=0)
		{	
			CloseHandle(hList);
			return iPos;
		} 
		CloseHandle(hList);
		return -1; 
	}
	CloseHandle(hList);
	return -1;
}

/*!
* \fn BOOL DeleteTorrentFromDownloadList()
* \brief Delete selected torrent session from the download list.
**/

static UINT DeleteTorrentFromDownloadList(LPVOID param)
{
	// Disable the configuration tab in the menu
	HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),0);
	if(hCommandBarMenu)
	{
		EnableMenuItem(hCommandBarMenu,0,MF_BYPOSITION|MF_GRAYED );
		EnableMenuItem(hCommandBarMenu,1,MF_BYPOSITION|MF_GRAYED );
	}
	
	// Disable the delete Button
	EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),FALSE);
	// If the downloading list is empty ----> disable buttons
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItems=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 

	if(iItems==1)
	{
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_PAUSE_DOWNLOAD),FALSE);
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_RESUME_DOWNLOAD),FALSE);
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),FALSE);
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_INFOS),FALSE);
		bDownloadListEmpty=true;
	}
	// Change the statue to deleting 
	ChangeSelectedTorrentStatus(_T("Deleting"));
	pPeerHandler->SetPaused();
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
		int iPos=ListView_GetSelectionMark(hList);
		
		if(iPos>=0)
		{
			char szTempFileName[100];
			TCHAR * szTemp2FileName=GetSelectedTorrentFileName();
			wcstombs(szTempFileName,szTemp2FileName,100);
			TCHAR * szSelectedTorrentStatus=GetSelectedTorrentStatus();
			

			SuspendTorrentFileSession(string(szTempFileName));
			DeleteTorrentFileSession(string(szTempFileName));
			
			if(szTemp2FileName!=NULL)
				delete [] szTemp2FileName;
			if(szSelectedTorrentStatus!=NULL)
				delete [] szSelectedTorrentStatus;

			BOOL bDeleted=ListView_DeleteItem(hList,iPos);
			
			pPeerHandler->SetNotPaused();
			if(hCommandBarMenu)
			{
				EnableMenuItem(hCommandBarMenu,0,MF_BYPOSITION|MF_ENABLED );
				EnableMenuItem(hCommandBarMenu,1,MF_BYPOSITION|MF_ENABLED );
			}
			if(SendMessage(hList,LVM_GETITEMCOUNT,0,0)>0)
				EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),TRUE);
			CloseHandle(hCommandBarMenu);
			CloseHandle(hList);
			ExitThread(0);
			return bDeleted;
		} 

		pPeerHandler->SetNotPaused();
		if(hCommandBarMenu)
		{
			EnableMenuItem(hCommandBarMenu,0,MF_BYPOSITION|MF_ENABLED );
			EnableMenuItem(hCommandBarMenu,1,MF_BYPOSITION|MF_ENABLED );
		}
		if(SendMessage(hList,LVM_GETITEMCOUNT,0,0)>0)
			EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),TRUE);
		CloseHandle(hCommandBarMenu);
		CloseHandle(hList);
		ExitThread(0);
		return false; 
	}
	
	pPeerHandler->SetNotPaused();
	if(hCommandBarMenu)
	{
		EnableMenuItem(hCommandBarMenu,0,MF_BYPOSITION|MF_ENABLED );
		EnableMenuItem(hCommandBarMenu,1,MF_BYPOSITION|MF_ENABLED );
	}
	if(SendMessage(hList,LVM_GETITEMCOUNT,0,0)>0)
		EnableWindow(GetDlgItem(hwndDL,IDC_BUTTON_DELETE),TRUE);
	CloseHandle(hCommandBarMenu);
	CloseHandle(hList);
	ExitThread(0);
	return false;
}
/*!
* \fn static UINT SuspendSelectedTorrentFileSession(LPVOID param)
* \brief Suspend selected torrent session.
**/
static UINT SuspendSelectedTorrentFileSession(LPVOID param)
{
	pPeerHandler->SetPaused();
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
		int iPos=ListView_GetSelectionMark(hList);
		
		if(iPos>=0)
		{	
			char szTempFileName[100];
			TCHAR * szTemp2FileName=GetSelectedTorrentFileName();
			wcstombs(szTempFileName,szTemp2FileName,100);
			SuspendTorrentFileSession(string(szTempFileName));
			if(szTemp2FileName!=NULL)
				delete [] szTemp2FileName;
			pPeerHandler->SetNotPaused();
			CloseHandle(hList);
			ExitThread(0);
			return true;
		} 
		pPeerHandler->SetNotPaused();
		CloseHandle(hList);
		ExitThread(0);
		return false; 
	}
	pPeerHandler->SetNotPaused();
	CloseHandle(hList);
	ExitThread(0);
	return false;
}
/*!
* \fn static UINT ResumeSelectedTorrentFileSession(LPVOID param)
* \brief Resume the selected torrent session.
**/
static UINT ResumeSelectedTorrentFileSession(LPVOID param)
{

	pPeerHandler->SetPaused();
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
		int iPos=ListView_GetSelectionMark(hList);
		
		if(iPos>=0)
		{
			char szTempFileName[100];
			TCHAR * szTemp2FileName=GetSelectedTorrentFileName();
			wcstombs(szTempFileName,szTemp2FileName,100);
			ResumeTorrentFileSession(string(szTempFileName));
			if(szTemp2FileName)
				delete [] szTemp2FileName;
			pPeerHandler->SetNotPaused();
			CloseHandle(hList);
			ExitThread(0);
			return true;
		} 
		pPeerHandler->SetNotPaused();
		CloseHandle(hList);
		ExitThread(0);
		return false; 
	}
	pPeerHandler->SetNotPaused();
	CloseHandle(hList);
	ExitThread(0);
	return false;
}

/*!
* \fn TCHAR * GetSelectedTorrentStatus()
* \brief Return the selected torrent status within the download list.
**/
TCHAR * GetSelectedTorrentStatus()
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItems=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItems>0)
	{	
		LVITEM				LvItem;
		int iPos=ListView_GetSelectionMark(hList);
		memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
		
		if(iPos>=0)
		{	
			TCHAR * szStatus=new TCHAR[100];
			ListView_GetItemText(hList,iPos,4,szStatus,100)
			CloseHandle(hList);
			return W2T(szStatus);
		} 
		CloseHandle(hList);
		return NULL; 
	}
	CloseHandle(hList);
	return NULL;
}


/*!
* \fn TCHAR * GetSelectedTorrentFileName()
* \brief Return the selected torrent file name within the download list.
**/
TCHAR * GetSelectedTorrentFileName()
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
int iItems=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItems>0)
	{	
		int iPos=ListView_GetSelectionMark(hList);
		
		if(iPos>=0)
		{	
			TCHAR * szFileName=new TCHAR[100];
			ListView_GetItemText(hList,iPos,0,szFileName,100);
			CloseHandle(hList);
			return W2T(szFileName);
		} 
		CloseHandle(hList);
		return NULL; 
	}
	CloseHandle(hList);
	return NULL;
}
/*!
* \fn BOOL ChangeSelectedTorrentStatus( TCHAR *status)
* \brief Function used to change the status of a torrent in the download list.
**/
BOOL ChangeSelectedTorrentStatus( TCHAR *status)
{
	HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
	int iItem=SendMessage(hList,LVM_GETITEMCOUNT,0,0); 
	if(iItem>0)
	{	
	
		LVITEM				LvItem;	
		int iPos=ListView_GetSelectionMark(hList);
		memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
		if(iPos>=0)
		{
			LvItem.pszText=(LPWSTR)status;
			LvItem.iItem=iPos;
			LvItem.iSubItem=4;
			LvItem.mask=LVIF_TEXT ;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
			CloseHandle(hList);
			return true;
		} 
		delete [] status;
		CloseHandle(hList);
		return false; 
	}
	delete [] status;
	CloseHandle(hList);
	return false;
}

/*!
* \fn BOOL CALLBACK DownloadListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Callback function for the torrents download list dialog.
**/
BOOL CALLBACK DownloadListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
        case WM_INITDIALOG:
            {
				InitCommonControls();
				
				// Disabling the close button
				//EnableMenuItem (GetSystemMenu(hDlg, FALSE), SC_CLOSE,MF_BYCOMMAND | MF_GRAYED);
				// initializing the Download List
				HWND hList=GetDlgItem(hDlg,IDC_LIST_TORRENTS);
				LVCOLUMN			LvCol;
				memset(&LvCol,0,sizeof(LvCol)); 
				
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.pszText=TEXT("File Name"); // First Header Text
				LvCol.cx=0x40;    // Width of a coloum
				SendMessage(hList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT); // Set style
				SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol); // Insert/Show the coloum
				
				memset(&LvCol,0,sizeof(LvCol)); 
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.cx=0x40;    // Width of a coloum
				LvCol.pszText=TEXT("Size (Kb)");                            // Next coloum
				SendMessage(hList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol); // ...
				
				memset(&LvCol,0,sizeof(LvCol)); 
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.cx=0x40;    // Width of a coloum
				LvCol.pszText=TEXT("Downloaded(Kb)");                            //
				SendMessage(hList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol); //
				
				memset(&LvCol,0,sizeof(LvCol)); 
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.cx=0x40;    // Width of a coloum
				LvCol.pszText=TEXT("Elapsed Time(s)");                            //
				SendMessage(hList,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol); //
				
				memset(&LvCol,0,sizeof(LvCol)); 
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.cx=0x40;    // Width of a coloum
				LvCol.pszText=TEXT("Status");                            //
				SendMessage(hList,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol); //
				
				memset(&LvCol,0,sizeof(LvCol)); 
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.cx=0x40;    // Width of a coloum
				LvCol.pszText=TEXT("Nbr of Peers");                            //
				SendMessage(hList,LVM_INSERTCOLUMN,5,(LPARAM)&LvCol); //

				memset(&LvCol,0,sizeof(LvCol)); 
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  
				LvCol.cx=0x40;    // Width of a coloum
				LvCol.pszText=TEXT("Uploaded(Kb)");                            //
				SendMessage(hList,LVM_INSERTCOLUMN,6,(LPARAM)&LvCol); //

				// Disable the view menu item
				HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),1);
				if(hCommandBarMenu)
					EnableMenuItem(hCommandBarMenu, 0,MF_BYPOSITION|MF_GRAYED );
				// Disable pause, delete, resume and details until the client starts
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_PAUSE_DOWNLOAD),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_RESUME_DOWNLOAD),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_DELETE),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_INFOS),FALSE);

				
				
				CloseHandle(hCommandBarMenu);
				CloseHandle(hList);
			}
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, LOWORD(wParam));
				ShowWindow(hDlg,SW_HIDE);
            
                return TRUE;
            }else if(LOWORD(wParam) == IDC_BUTTON_PAUSE_DOWNLOAD)
			{
				TCHAR *szCurrentStatus;
				szCurrentStatus=GetSelectedTorrentStatus();
				if(wcscmp(szCurrentStatus,_T("Seeding"))!=0)
				{
					if(wcscmp(szCurrentStatus,_T("Downloading"))==0)
					{
						HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SuspendSelectedTorrentFileSession,(LPVOID)NULL,0,NULL);
						Sleep(1);
						CloseHandle(th);
						//SuspendSelectedTorrentFileSession();
						ChangeSelectedTorrentStatus(_T("Paused"));
					}else 
					{
						MessageBox( hDlg,TEXT("The downloading proccess is already paused!"),
					TEXT("Changing the Torrent Status"), MB_OK|MB_ICONSTOP );
                
					}
				}else{
						MessageBox( hDlg,TEXT("The selected torrent is not in the downloading status!"),
					TEXT("Changing the Torrent Status"), MB_OK|MB_ICONSTOP);
                }
				if(szCurrentStatus!=NULL)
					delete [] szCurrentStatus;
			}else if(LOWORD(wParam) == IDC_BUTTON_RESUME_DOWNLOAD)
			{
				TCHAR *szTest=GetSelectedTorrentStatus();
				if(wcscmp(szTest,_T("Paused"))==0)
				{
					HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ResumeSelectedTorrentFileSession,(LPVOID)NULL,0,NULL);
					CloseHandle(th);
					Sleep(1);
					ChangeSelectedTorrentStatus(_T("Downloading"));
				}else
				{
					MessageBox( hDlg,TEXT("The downloading proccess is already resumed!"),
                TEXT("Changing the Torrent Status"), MB_OK|MB_ICONSTOP);
                
				}
				if(szTest!=NULL)
					delete []szTest;
			}else if(LOWORD(wParam) == IDC_BUTTON_DELETE)
			{
				HANDLE th = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DeleteTorrentFromDownloadList,(LPVOID)NULL,0,NULL);
				CloseHandle(th);
				Sleep(1000);
				
				
			}else if(LOWORD(wParam) == IDC_BUTTON_INFOS)
			{
				TCHAR * pSelectedForInfos;
				pSelectedForInfos=GetSelectedTorrentFileName();
				if(pSelectedForInfos!=NULL)
				{	
					hwndTD = CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_DIALOG_DETAILS),hDlg,TorrentDetailsDlgProc);
					ShowWindow(hwndTD,SW_SHOW);
					delete [] pSelectedForInfos;

				}else 	MessageBox( hDlg,TEXT("Please select a torrent from the downloading list before."),
                TEXT("Torrent Details"), MB_OK|MB_ICONINFORMATION );
                
			}
		    break;
		case WM_CTLCOLORBTN ://Dessin des boutons
			{
				HDC hdcButton = (HDC) wParam; // handle to the button display context 
				HWND hwndButton = (HWND) lParam; // handle to the button 
				HWND BPause=GetDlgItem(hDlg,IDC_BUTTON_PAUSE_DOWNLOAD);
				//if (hwndButton==BPause)
                //{
					return (LRESULT)CreateSolidBrush(RGB(157,255,42));
				//}
				break;
			}
			case WM_CTLCOLORDLG://Couleur de fond de la boite de dialogue
				{
					static HBRUSH hbDialog = CreateSolidBrush(RGB(76,202,234));
					return (INT_PTR)hbDialog;
				}
			
			case WM_CTLCOLORSCROLLBAR:
				{
					return (LRESULT)CreateSolidBrush(RGB(247,47,94));

					break;
				}
		case WM_CLOSE:
			{
				// Enable the view Menu Item
				HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),1);
				if(hCommandBarMenu)
					EnableMenuItem(hCommandBarMenu, 0,MF_BYPOSITION|MF_ENABLED );
				CloseHandle(hCommandBarMenu);
				ShowWindow(hDlg,SW_HIDE);
				EndDialog(hDlg, message);
				
			}
			return TRUE;

	}
    return (INT_PTR)FALSE;
}
/*!
* \fn BOOL CALLBACK ChooseFilePathDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Callback function for the dialog used to choose the file path.
**/
BOOL CALLBACK ChooseFilePathDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    TCHAR      tszPlatType[12];
    int        i, rc, iBaseStringID, iMaxStringID;
    int        iSel;
    short int  wCharCnt;
    LPCTSTR    lpszPathString;
    LPTSTR    lpszPath;
    LPBYTE     lpbStrlen;
    HWND       hWndList;

	switch (message)
	{
		case WM_INITDIALOG:
         hWndList = GetDlgItem(hDlg,IDC_PATHNAMES );
        //Init paths list based on platform type
	    memset( &tszPlatType, 0x0, sizeof(tszPlatType));
	    rc = SystemParametersInfo( SPI_GETPLATFORMTYPE, 
                                    sizeof(tszPlatType),
								    &tszPlatType, 0);
	    if( (lstrcmp( tszPlatType, TEXT("Jupiter") ) == 0 )
	                        ||
	       ( lstrcmp( tszPlatType, TEXT("HPC") ) == 0 ) )
        { 
            iBaseStringID = IDS_HPC_PATH1;
            iMaxStringID  = IDS_HPC_PATH11;
        }
	    
	    if( lstrcmp( tszPlatType, TEXT("Palm PC") ) == 0 )
	    { 
            iBaseStringID = IDS_PPC_PATH1;
            iMaxStringID  = IDS_PPC_PATH7;
        }

        for( i = iBaseStringID; i < iMaxStringID; i++ )
        {
            lpszPathString = 
                (LPCTSTR)LoadString(g_hInst, i, NULL, NULL);
            lpbStrlen = (LPBYTE)lpszPathString;
            lpbStrlen -= 2 * sizeof( BYTE );
            wCharCnt = (short int)(*lpbStrlen);
            lpszPath = (LPTSTR)LocalAlloc( LPTR, (wCharCnt + 1) * sizeof(TCHAR));
            LoadString(g_hInst, i, lpszPath, (int)wCharCnt + 1);
            ListBox_InsertString(hWndList, -1, lpszPath);
            LocalFree(lpszPath);
        }
			return TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK) 
			{
               //set path from ctrl input
               //get the sel item
               hWndList = GetDlgItem(hDlg,IDC_PATHNAMES );
               iSel = SendMessage(hWndList, LB_GETCURSEL, 0, 0 );
  
               //copy sel item text
               memset( szPath, 0x0, sizeof(szPath) );
               SendMessage(hWndList,LB_GETTEXT, iSel, 
                           (LPARAM)(LPCTSTR) &szPath );
               EndDialog(hDlg, LOWORD(wParam));
			   ShowWindow(hDlg,SW_HIDE);
		       return TRUE;
			}

             if (LOWORD(wParam) == IDCANCEL)
             {
                //set path NULL
               memset( szPath, 0x0, sizeof(szPath) );
              EndDialog(hDlg, LOWORD(wParam));
			  ShowWindow(hDlg,SW_HIDE);
		     return TRUE;
            }
			break;
	}
    return FALSE;


	MessageBox( hDlg, TEXT( "Couldn't Allocate Memory"), 
                      TEXT( "Choose Path Failed" ), MB_OK|MB_ICONSTOP);
    return FALSE;

}





/*!
* \fn LPOPENFILENAME DoFileOpen(HWND hWnd, int iOpenSaveFlag,LPCWSTR lpFilter)
* \brief Function used to open a . Torrent file either for seeding or downloading.
**/
LPOPENFILENAME DoFileOpen(HWND hWnd, int iOpenSaveFlag,LPCWSTR lpFilter)
{
    TCHAR*  pszFileName;
	LPCWSTR pszOpenFilter = lpFilter;
	LPOPENFILENAME lpof;

	// Allocate space for the OPENFILENAME struct
	lpof = (LPOPENFILENAME)LocalAlloc(LPTR,sizeof (OPENFILENAME));
    if (!lpof) 
		{ goto FAIL;}

	// Allocate space for the FILENAME string
	pszFileName = (TCHAR*)LocalAlloc(LPTR, MAX_PATH);
    if (!pszFileName) 
		{ goto FAIL;}

    // Make the first char null, as we don't want the 
	// dialog to init the  filename edit ctl.
  //  pszFileName[0] = '\0';

    // Initialize File Open structure.
    lpof->lStructSize = sizeof (OPENFILENAME);
    lpof->hwndOwner = hWnd;
    lpof->lpstrFile = pszFileName;
    lpof->nMaxFile  = MAX_PATH;
    lpof->lpstrFilter = pszOpenFilter;
    lpof->lpstrInitialDir = (LPCWSTR)&szPath[0];
 
	if (iOpenSaveFlag ==  FILE_SAVE)
 {
	//prompt for overwrite, send files to sync directory
	lpof->Flags = OFN_OVERWRITEPROMPT;


	 if( !GetSaveFileName (lpof))
     {
			lpof = NULL;
	 }
 }
else
{

	 if( !GetOpenFileName (lpof))
		 {
			lpof = NULL;
		 }
}
 //it's all good  
	return lpof;

//fail and bail
FAIL:
	MessageBox( hWnd, TEXT( "Couldn't Allocate Memory"), 
                      TEXT( "OpenFile Failed" ), MB_OK|MB_ICONSTOP);
        return 0;
}
 /*!
* \fn BOOL CALLBACK TorrentDetailsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Callback function for the torrents details dialog.
**/
BOOL CALLBACK TorrentDetailsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	  switch(message)
	{
	case WM_INITDIALOG:
            {
				InitCommonControls();
              // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
				// Setting Default Values
				char szFileName[100];
				TCHAR * pSelectedForInfos=GetSelectedTorrentFileName();
				wcstombs(szFileName,pSelectedForInfos,100);
				if(pSelectedForInfos)
					delete [] pSelectedForInfos;
				std::string sSelectedHash=pPeerHandler->GetHashFronFileName(string(szFileName));
				Session * pSelectedTorrentSession=pPeerHandler->GetSession(sSelectedHash);
				
				SetDlgItemText(hDlg,IDC_EDIT_SAVE_IN,ConvertLPCSTRToLPWSTR((char*)pSelectedTorrentSession->GetLogDirectory().c_str()));
				SetDlgItemInt(hDlg,IDC_EDIT_NUMBER_PIECES,pSelectedTorrentSession->GetNumberOfPieces(),FALSE);
				SetDlgItemText(hDlg,IDC_EDIT_TRACKER_URL,ConvertLPCSTRToLPWSTR((char*)pSelectedTorrentSession->GetAnnounce().c_str()));
				SetDlgItemInt(hDlg,IDC_EDIT_PIECE_SIZE,(UINT)pSelectedTorrentSession->GetPieceLength(),FALSE);
				SetDlgItemInt(hDlg,IDC_EDIT_SLICE_SIZE,iSliceSize,FALSE);
				SetDlgItemText(hDlg,IDC_EDIT_DOWNLOAD_SPEED,ConvertLPCSTRToLPWSTR((char*)pSelectedTorrentSession->GetDownloadSpeed().c_str()));
				
				
				// Initializing the progress bar
				HWND hProgress=GetDlgItem(hDlg,IDC_PROGRESS_DOWNLOAD);
				int progressBarLength;
				int tmp1 = pSelectedTorrentSession->GetLength()/iSliceSize;
				int tmp2 = pSelectedTorrentSession->GetLength() - (tmp1*iSliceSize);
				
				if(tmp2 > 0 && tmp2 < iSliceSize)
					progressBarLength = tmp1 + 1;
				else progressBarLength = tmp1;

				SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, progressBarLength)); 
				SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, 0);      

				
				ProgressBar=true;
				ProgressBarStatusChanged = true;
				pSelectedTorrentSession=NULL;
				CloseHandle(hProgress);
			}
            return (INT_PTR)TRUE;
	case WM_COMMAND:
		
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg,  LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
			ProgressBar=false;
            ProgressBarStatusChanged = true;
			return TRUE;
		
		default:
			return (INT_PTR)FALSE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		ShowWindow(hDlg,SW_HIDE);
		ProgressBar=false;
        ProgressBarStatusChanged = true;
		return TRUE;

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;

}



/*!
* \fn BOOL CALLBACK InvolvedPeersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
* \brief Callback function for the list of involved peers.
**/
BOOL CALLBACK InvolvedPeersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	  switch(message)
	{
	case WM_INITDIALOG:
            {
				InitCommonControls();
				LVCOLUMN			sLvCol;
				
				Session * pSelectedTorrentSession;
				TCHAR * pSelectedForInfos;
				pSelectedForInfos=GetSelectedTorrentFileName();
				if(pSelectedForInfos!=NULL)
				{	
					char szFileName[100];
					wcstombs(szFileName,pSelectedForInfos,100);
					std::string sSelectedHash=pPeerHandler->GetHashFronFileName(string(szFileName));
					pSelectedTorrentSession=pPeerHandler->GetSession(sSelectedHash);
					delete [] pSelectedForInfos;
				}	
				pSelectedForInfos=NULL;

				// initializing the List of peers
				HWND hListPeers=GetDlgItem(hDlg,IDC_LIST_INVOLVED_PEERS);
				memset(&sLvCol,0,sizeof(sLvCol)); 
				sLvCol.mask=LVCF_TEXT|LVCF_WIDTH;  
				sLvCol.pszText=TEXT("Ip @"); // First Header Text
				sLvCol.cx=0x50;;    // Width of a coloum
				SendMessage(hListPeers,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT); // Set style
				SendMessage(hListPeers,LVM_INSERTCOLUMN,0,(LPARAM)&sLvCol); // Insert/Show the coloum
				
				memset(&sLvCol,0,sizeof(sLvCol)); 
				sLvCol.mask=LVCF_TEXT|LVCF_WIDTH;  
				sLvCol.cx=0x30;;    // Width of a coloum
				sLvCol.pszText=TEXT("Hop(s)");                            // Next coloum
				SendMessage(hListPeers,LVM_INSERTCOLUMN,1,(LPARAM)&sLvCol); // ...
				

				// Inserting items to the list
				int iNbrItems=pSelectedTorrentSession->GetNumberOfInvolvedPeers();
				if(iNbrItems>0)
				{
					for(int i=0;i<iNbrItems;i++)
					{
						AddItemToInvolvedPeersList(hDlg,ConvertLPCSTRToLPWSTR((char *)pSelectedTorrentSession->GetPeerNumberIp(i).c_str()),pSelectedTorrentSession->GetPeerNumberNbrHops(i));
					}
				}
				CloseHandle(hListPeers);
				pSelectedTorrentSession=NULL;
				
			}
            return (INT_PTR)TRUE;
	case WM_COMMAND:
		
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),1);
					if(hCommandBarMenu)
						EnableMenuItem(hCommandBarMenu, 1,MF_BYPOSITION|MF_ENABLED );
				CloseHandle(hCommandBarMenu);
				EndDialog(hDlg,  LOWORD(wParam));
				ShowWindow(hDlg,SW_HIDE);
			}
            return TRUE;
		
		default:
			return (INT_PTR)FALSE;
		}
		break;
	case WM_CLOSE:
		{
			HMENU hCommandBarMenu=GetSubMenu(CommandBar_GetMenu(hwndCB,0),1);
			if(hCommandBarMenu)
				EnableMenuItem(hCommandBarMenu, 1,MF_BYPOSITION|MF_ENABLED );
			CloseHandle(hCommandBarMenu);
			EndDialog(hDlg, LOWORD(wParam));
			ShowWindow(hDlg,SW_HIDE);
		}
        return TRUE;

	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;

}


/*!
* \fn BOOL AddItemToInvolvedPeersList(TCHAR *TorrentFile, int64_t FileSize, int64_t downloaded, TCHAR *TorrentStatus,int NbrPeers,int64_t uploaded,size_t uElapsedTime)
**/
BOOL AddItemToInvolvedPeersList(HWND Dlg,TCHAR *IpAdr, int iNbrHops)
{
	HWND hListPeers=GetDlgItem(Dlg,IDC_LIST_INVOLVED_PEERS);
	LVITEM				sLvItem;
	memset(&sLvItem,0,sizeof(sLvItem)); // Zero struct's Members
	sLvItem.mask=LVIF_TEXT;   // Text Style
	sLvItem.cchTextMax = 256; // Max size of test
	sLvItem.iItem=0;          // choose item  
	sLvItem.iSubItem=0;       // Put in first coluom
	sLvItem.pszText=IpAdr;    // Text to display (can be from a char variable) (Items)
	SendMessage(hListPeers,LVM_INSERTITEM,0,(LPARAM)&sLvItem); // Send info to the Listview
	
	TCHAR Temp[100];
	
	sLvItem.iSubItem=1;
	if(iNbrHops<0)
		swprintf(Temp,_T("%i"),0); // In the case of local ip @ or not assigned one
	else swprintf(Temp,_T("%i"),iNbrHops);
	sLvItem.pszText=Temp;
	SendMessage(hListPeers,LVM_SETITEM,0,(LPARAM)&sLvItem); // Enter text to SubItems
	
	CloseHandle(hListPeers);
	delete [] IpAdr;
	return true;
}



void CloseAllHandls()
{
	CloseHandle(g_hWndMenuBar);				
	CloseHandle(hwndCB);
	CloseHandle(hwndCD);		
	CloseHandle(hwndDL);
	CloseHandle(hwndBV);
	CloseHandle(hwndTD);
	CloseHandle(mainhWnd);
	CloseHandle(hMainThread);
	CloseHandle(hMutex);
}
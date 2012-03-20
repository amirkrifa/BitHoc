
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
* \file SyncServer.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _SYNC_SERVER
#define _SYNC_SERVER
#pragma once
#include "socket.h"
extern bool bStopTheServer;
/*!
* \class SyncServer
* \brief This Class is used by Bittorrent Client in order to manage the synchronisation messages.
*/
class SyncServer
{
public:
	SyncServer(int port);
	/*!
	* \fn static UINT StartSyncThreads(LPVOID tp)
	* \brief Start point of the thread which will run the synchronisation server.
	*/	
	static UINT StartSyncThreads(LPVOID tp);
	/*!
	* \fn static UINT StartReceiverThread(LPVOID tp)
	* \brief It starts the thread that will receive the synchronisation message.
	*/	
	static UINT StartReceiverThread(LPVOID tp);
	/*!
	* \fn void StartServer()
	* \brief It starts the main synchronisation server.
	*/	
	void StartServer();
	/*!
	* \fn void StopServer()
	* \brief Stops the synchronisation server.
	*/
	void StopServer()
	{
		bStopTheServer=true;
	};
	~SyncServer(void);
private:
	/*!
	* \struct SyncThreadParam
	* \brief Used as parameter for the main server thread.
	*/
	typedef struct SyncThreadParam
	{
		AmirSocketLib::SocketServer *SyncSocket;
		AmirSocketLib::Socket *SyncGeneratedSocket;
		int iServerPort;
		SyncServer * pSyncServer;
	}SyncThreadParam;
	//! Server's socket.
	AmirSocketLib::SocketServer * SyncSocket;
	//! Server's port.
	int iPort;
	HANDLE hSyncServ;
};
#endif _SYNC_SERVER

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
* \file SyncServer.cpp
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "StdAfx.h"
#include "SyncServer.h"
#include "PeerHandler.h"
#include "socket.h"

//! Pointer to the PeerHandler instance.
extern PeerHandler * pPeerHandler;
bool bStopTheServer;
SyncServer::SyncServer(int port)
{
	SyncSocket=new AmirSocketLib::SocketServer(port, 100);
	iPort=port;
	bStopTheServer=false;
}

SyncServer::~SyncServer(void)
{
	// Close the waiting socket
	AmirSocketLib::SocketClient out(std::string("127.0.0.1"),iPort);
	out.Close();
	WaitForSingleObject(hSyncServ, 60000);
	if(SyncSocket)
		delete SyncSocket;
	
}

void SyncServer::StartServer()
{
	SyncThreadParam* p=(SyncThreadParam*)malloc(sizeof(SyncThreadParam));
	p->pSyncServer=this;
	p->iServerPort=iPort;
	p->SyncSocket=this->SyncSocket;
	p->SyncGeneratedSocket=NULL;
	hSyncServ = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartSyncThreads,(LPVOID)p,0,NULL);
}

UINT SyncServer::StartSyncThreads(LPVOID tp)
{
	SyncThreadParam* tpt=(SyncThreadParam* )tp;
	
	AmirSocketLib::SocketServer *in=tpt->SyncSocket;
	while (!bStopTheServer) {
		SyncThreadParam * p=(SyncThreadParam* )malloc(sizeof(SyncThreadParam));
		p->SyncGeneratedSocket=in->Accept();
		if(p->SyncGeneratedSocket != NULL)
		{
			p->pSyncServer=tpt->pSyncServer;
			p->SyncSocket = NULL;
			p->iServerPort=tpt->iServerPort;
			CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartReceiverThread,(LPVOID)p,0,NULL);
		}
	}

	in->Close();
	free(tpt);
	tpt=NULL;
	return 0;
}

UINT SyncServer::StartReceiverThread(LPVOID tp)
{
	SyncThreadParam* tpt=(SyncThreadParam* )tp;
	AmirSocketLib::Socket * s=tpt->SyncGeneratedSocket;
	std::string line = s->ReceiveLine();
	if(line.empty())
		return 0;
	// hash received
	pPeerHandler->SetPaused();
	pPeerHandler->SynchronizeSession(line.substr(0,40));
	pPeerHandler->SetNotPaused();

	s->Close();
	free(tpt);
	return 0;
}
	
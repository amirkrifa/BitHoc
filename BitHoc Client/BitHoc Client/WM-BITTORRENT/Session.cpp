
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

/*
Copyright (C) 2005  Anders Hedstrom

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
* \file Session.cpp
* \brief Status of a Bittorent downloading session.
**/

#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#define random rand
typedef unsigned __int32 uint32_t;
#endif

#include <windows.h>
#include <vector>
#include <Utility.h>
#include "PeerHandler.h"
#include "Peer.h"
#include "bitmap_t.h"
#include "Piece.h"
#include "pSocket.h"
#include "FileManager.h"
#include "Clock.h"
#include "DownloadListFunctions.h"
#include "Session.h"
#include <openssl/crypto.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sstream>
#include "LogFile.h"
#include "resourceppc.h"
#include "UsefulFunctions.h"

//! Added by Amir Krifa.
extern void IntToString(uint32_t i, std::string & o);
extern BOOL ChangeTorrentStatus(LPWSTR TorrentFile,LPWSTR status );
extern LPWSTR ConvertLPCSTRToLPWSTR (char* pCstring);
extern int iPeersScope;
extern int WM_BITTORRENT_VERSION;
extern HWND				hwndDL;	
extern int iQ;
extern int iCount;
extern bool bRecoverFromError;
extern int iChokingPeriod;
extern size_t iSliceSize;
extern unsigned char uPeerId[20];
extern HWND				hwndDL;
extern int IsItInTheDownloadList(LPWSTR TorrentFile);
extern void UpdateDownloadListButtons();
extern bool bContinueSeeding;
extern HWND hwndTD;
extern bool ProgressBar;
extern bool ProgressBarStatusChanged;
extern bool ACTIVATE_LOG;

//! Modified by Amir Krifa.
Session::Session(SocketHandler& h,const std::string& hash,bool seeding,string l)
:m_handler(h)
,m_info_hash(hash)
,m_piece_length(0)
,m_length(0)
,m_number_of_pieces(0)
,m_last_length(0)
,m_interval(15)
,m_t_tracker(0)
,m_bitmap(NULL)
,m_prev_offset(0)
,m_prev_length(0)
,m_length_one(0)
,m_filemanager(NULL)
,m_b_check_complete(false)
,m_demon(false)
,bSeeding(seeding)
,uiTotalR(0)
,uiTotalS(0)
,sLogDirectory(l)
,iNbrPieceComplete(0)
,pLastChoosedPeer(NULL)
,uElapsedTime(0)
,uElapsedTimeLastUpdate(time(NULL))
,bAlreadyDownloaded(FALSE)
,uLastTR(0)
,bToDelete(false)
,iPaused(0)
,bDeleteRelatedFiles(FALSE)
,uLastTotalRecievedBytes(0)
{
	InitializeCriticalSection(&csStop);
	srand(time(NULL)*(time_t)this);
	memcpy(m_peer_id, uPeerId, 20);
	if(ACTIVATE_LOG)
	{
		session_log = new LogFile(string(l+"session_log.txt"));
		downloaded_files_log = new LogFile(string(l+"downloaded_files.txt"));
		matrix_log = new LogFile(string(l+"trafic_matrix.txt"));
	}else
	{
		session_log = NULL;
		downloaded_files_log = NULL;
		matrix_log = NULL;
	}
}


Session::~Session()
{
	// Calculate the Sharing Ratio
	if(ACTIVATE_LOG)
	{
		CalculateTheSharingRatioMatrix();
		// Write the Sharing Ratio matrix to a log file
		WriteTheSharingRatioMatrixToLogFile();
	}

	u_traffic_matrix_map.clear();
	d_traffic_matrix_map.clear();
	sharing_ratio_matrix.clear();

	DeleteCriticalSection(&csStop);
	while (!m_files.empty())
	{
		file_v::iterator it = m_files.begin();
		file_t *p = *it;
		delete p;
		m_files.erase(it);
	}
	while (!m_peers.empty())
	{
		peer_v::iterator it = m_peers.begin();
		Peer *p = *it;
		delete p;
		m_peers.erase(it);
	}
	while (!m_complete.empty())
	{
		piece_v::iterator it = m_complete.begin();
		Piece *p = *it;
		delete p;
		m_complete.erase(it);
	}
	while (!m_incomplete.empty())
	{
		piece_v::iterator it = m_incomplete.begin();
		Piece *p = *it;
		delete p;
		m_incomplete.erase(it);
	}

	if (m_filemanager)
		delete m_filemanager;
	if(session_log)
		delete session_log;
	if(downloaded_files_log)
		delete downloaded_files_log;
	if(matrix_log)
		delete matrix_log;
}


void Session::AddFile(int64_t length)
{
	AddFile(m_name, length);
	m_length_one = length;
}


void Session::AddFile(const std::string& path,int64_t length)
{
	file_t *p = new file_t(path, length);
	m_length += length;
	p -> offset = m_prev_offset + m_prev_length;
	m_files.push_back(p);
	m_prev_offset = p -> offset;
	m_prev_length = p -> length;
	p=NULL;
}


// load peers, complete, incomplete
void Session::Load()
{
	std::string filename = GetBitmapFilename();
	FILE *fil = fopen(filename.c_str(), "r+b");
	if (fil &&!this->bSeeding)
	{
		
		//paused or not
		fread(&this->iPaused, sizeof(int), 1, fil); 
		
		//Get the number of bytes already Uploaded.
		fread(&this->uiTotalS, sizeof(int64_t), 1, fil); 
		//Get the Elapsed time.
		fread(&this->uElapsedTime, sizeof(size_t), 1, fil); 

		size_t q;
		fread(&q, sizeof(size_t), 1, fil); //!Number of peers.
		while (q--)
		{
			char c;
			char ip[100];
			int iNumberOfHops=-1;
			fread(&c, 1, 1, fil); // length of ip
			fread(ip, 1, c, fil);
			ip[ (int)c] = 0;
			char id[20];
			fread(id, 1, 20, fil);
			port_t port;
			fread(&port, sizeof(port_t), 1, fil);
			fread(&iNumberOfHops, sizeof(int), 1, fil);
			if (!GetPeer(ip) && port)
			{
				Peer *p = new Peer(Handler(),m_info_hash,ip,static_cast<std::string>(id).substr(0,20),port,iNumberOfHops);
				AddPeer(p);
				p=NULL;
			}
		}
		// Cleaning m_complete 
		if(!m_complete.empty())
		{
			piece_v::iterator cp;
			while(!m_complete.empty())
			{
				cp=m_complete.begin();
				delete *cp;
				m_complete.erase(cp);
			}
		}
		// Cleaning m_incomplete 
		if(!m_incomplete.empty())
		{
			piece_v::iterator cp;
			while(!m_incomplete.empty())
			{
				cp=m_incomplete.begin();
				delete *cp;
				m_incomplete.erase(cp);
			}
		}

		load_piece_v(fil, m_complete);
		load_piece_v(fil, m_incomplete);
		fclose(fil);
		// ...
		for (piece_v::iterator it = m_complete.begin(); it != m_complete.end(); it++)
		{
			Piece *p = *it;
			p -> ClearRequested();
		}
		for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
		{
			Piece *p = *it;
			p -> ClearRequested();
		}
		this->uElapsedTimeLastUpdate=time(NULL);
		if((this->m_length==this->GetTotalBytesR()) &&this->m_length>0)
		{
			// The file was already downloaded
			this->bAlreadyDownloaded=true;
		}
		return;
	}
	// no file to load
	if(!this->bSeeding)
	{
		for (size_t i = 0; i < m_number_of_pieces; i++)
		{
			Piece *p;
			if (i < m_number_of_pieces - 1)
			{
				p = new Piece(i, m_piece_length);
			}
			else
			{
				p = new Piece(i, m_last_length);
			}
			m_incomplete.push_back(p);
			p=NULL;
		}
		Verify();
	//!Added by Amir Krifa for Seeding capabilities.
	}else
	{
		if(!this->bAlreadyDownloaded)
		{
			for (size_t i = 0; i < m_number_of_pieces; i++)
			{
				Piece *p;
				if (i < m_number_of_pieces - 1)
				{
					p = new Piece(i, m_piece_length);
				}
				else
				{
					p = new Piece(i, m_last_length);
				}
				m_complete.push_back(p);
				p=NULL;
			}
			this->SetCheckComplete();
			Verify();
		}

	}
}


// save peers, complete, incomplete
//! Modified by Amir Krifa.
	
void Session::Save()
{
	std::string filename = GetBitmapFilename();
	static_cast<PeerHandler&>(Handler()).mkpath( filename );
	FILE *fil = fopen(filename.c_str(), "w+b");
	if (fil)
	{
		
		fwrite(&this->iPaused,sizeof(int),1,fil);
		
		//!saving the number of bytes uploaded.
		fwrite(&this->uiTotalS,sizeof(int64_t),1,fil);
		//!Saving the Elapsed Time
		fwrite(&this->uElapsedTime,sizeof(size_t),1,fil);
		
		size_t q = m_peers.size();
		fwrite(&q, sizeof(size_t), 1, fil);
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *peer = *it;
			char c = peer -> GetIP().size();
			fwrite(&c, 1, 1, fil);
			fwrite(peer -> GetIP().c_str(), 1, c, fil);
			fwrite(peer -> GetID().c_str(), 1, 20, fil);
			port_t p = peer -> GetPort();
			fwrite(&p, sizeof(port_t), 1, fil);
			//!Save the number of hops.
			int iNbrHops=peer->GetNumberOfHops();
			fwrite(&iNbrHops, sizeof(int), 1, fil);
		}
		save_piece_v(fil, m_complete);
		save_piece_v(fil, m_incomplete);
		fclose(fil);
	}
}


std::string Session::GetBitmapFilename()
{
	std::string torrentdir = static_cast<PeerHandler&>(Handler()).GetTorrentDirectory();
	return torrentdir + "\\" + m_info_hash + "\\." + m_info_hash;
	
}


Peer *Session::GetPeer(const std::string& ip)
{
	if(!m_peers.empty())
	{
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *p = *it;
			if (p -> GetIP() == ip)
				return p;
		}
	}
	return NULL;
}


//! Modified by Amir Krifa.

std::string Session::GetAnnounceUrl(const std::string& event)
{
	if(!bSeeding)
	{
		std::string url;
		char slask[200];

		url = GetAnnounce();
		url += "?info_hash=";
		for (size_t i = 0; i < 40; i += 2)
		{
			url += '%';
			url += m_info_hash.substr(i,2);
		}
		// Add the file Name
		url += "&file_name=";
		string tmpName;
		tmpName.append(m_name); tmpName.append(".torrent");
		strcpy(slask, "");
		strncpy(slask,tmpName.c_str(),tmpName.length());
		slask[tmpName.length()] = '\0';
		url += slask;
	
		strcpy(slask,"");
		strncpy(slask,reinterpret_cast<char *>(m_peer_id),20);
		slask[20]='\0';
		url += "&peer_id="; // peer_id
		url +=slask;
		url += "&ip=" + static_cast<PeerHandler&>(Handler()).GetExternIP();
		sprintf(slask,"&port=%d", static_cast<PeerHandler&>(Handler()).GetListenPort() );
		url += slask; // port
		//! Total sent bytes.
		sprintf(slask ,"%u", this->uiTotalS);
		url += "&uploaded=";
		url+=slask;
		//! Total received bytes.
		int64_t uTotalRecv=GetTotalBytesR();
		sprintf(slask ,"%u", uTotalRecv);
		url += "&downloaded=";
		url+=slask;
		sprintf(slask ,"%u", this->GetLength()-uTotalRecv);
		url += "&left=";
		url+=slask;
		if (event.size())
			url += "&event=" + event;
		url += "&numwant=" + Utility::l2string(static_cast<PeerHandler&>(Handler()).GetMinPeers() * 2);

		return url;
	}
	else
	{
		std::string url;
		char slask[200];

		url = GetAnnounce();
		url += "?info_hash=";
		for (size_t i = 0; i < 40; i += 2)
		{
			url += '%';
			url += m_info_hash.substr(i,2);
		}
		// Add the file Name
		url += "&file_name=";
		string tmpName;
		tmpName.append(m_name); tmpName.append(".torrent");
		strcpy(slask, "");
		strncpy(slask,tmpName.c_str(),tmpName.length());
		slask[tmpName.length()] = '\0';
		url += slask;
	
		strcpy(slask,"");
		strncpy(slask,reinterpret_cast<char *>(m_peer_id),20);
		slask[20]='\0';
		url += "&peer_id="; // peer_id
		url +=slask;
		url += "&ip=" + static_cast<PeerHandler&>(Handler()).GetExternIP();
		sprintf(slask,"&port=%d", static_cast<PeerHandler&>(Handler()).GetListenPort() );
		url += slask; // port
		sprintf(slask ,"%u", this->uiTotalS);
		url += "&uploaded=";
		url+=slask;
		sprintf(slask ,"%u", this->uiTotalR);
		url += "&downloaded=";
		url+=slask;
		url += "&left=0";
		url += "&event=completed" ;
		url += "&numwant=" + Utility::l2string(static_cast<PeerHandler&>(Handler()).GetMinPeers() * 2);

		return url;
	}
}


void Session::SetPieceLength(int64_t x)
{
	m_piece_length = x;
	if (m_piece_length % iSliceSize)
	{

	}
}


void Session::SetPieces(const std::string& x)
{
	m_pieces = x;
	m_number_of_pieces = x.size() / 20;
	m_last_length = m_length % m_piece_length;
	if (!m_last_length)
		m_last_length = m_piece_length;
}


void Session::AddConnect()
{
	PeerHandler& ref = static_cast<PeerHandler&>(Handler());
	std::vector<Peer *> available;
	time_t now = time(NULL);
	for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
	{
		Peer *peer = *it;
		if (peer -> Failed())
		{
			m_peers.erase(it);
			break;
		}
	}
	{
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *peer = *it;
			bool bTest=peer -> Connected();
			if (!bTest && 
				now > peer -> GetChokeTime() + ref.GetChokeTimer() && 
				!peer -> Tried() &&
				!peer -> Failed())
			{
				available.push_back(peer);
			}
		}
	}
	size_t q;
	if ((q = available.size()) > 0)
	{
		Peer *peer = available[random() % q];
		pSocket *p = new pSocket(Handler(), m_info_hash, (unsigned char *)peer -> GetID().c_str());
		if(ACTIVATE_LOG)
			p->InitLogFile(this->GetLogDirectory(),true);
		p -> Open(peer -> GetIP(), peer -> GetPort());
		p -> SetDeleteByHandler();
		Handler().Add(p);
		peer -> SetTried();
		p=NULL;
	}
	else
	{
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *peer = *it;
			peer -> SetTried(false);
		}
	}
}


bool Session::GetRandomNotRequested(Peer *peer,size_t& piece,size_t& offset,size_t& length)
{
	std::map<size_t,std::vector<Piece *> > mmap;
	size_t q[1000];
	size_t max = 0;
	memset(q, 0, sizeof(size_t[1000]));
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested())
		{
			size_t n = p -> NumberComplete();
			n = p -> NumberRequested();
			if (n < 1000)
			{
				q[n]++;
				mmap[n].push_back(p);
				max = MAX(max,n);
			}
		}
	}
	while (max > 0)
	{
		if (q[max] && (!max || (random() % 13 > 0)) )
		{
			std::vector<Piece *>& ref = mmap[max];
			Piece *p = ref[random() % ref.size()];
			piece = p -> GetNumber();
			return p -> GetRandomNotRequested(offset, length);
		}
		max--;
	}
	//
	{
		if (q[max])
		{
			std::vector<Piece *>& ref = mmap[max];
			Piece *p = ref[random() % ref.size()];
			piece = p -> GetNumber();
			return p -> GetRandomNotRequested(offset, length);
		}
	}
	// find rarest piece to begin with
	return false;
}


Request *Session::AddRequest(Peer *peer,size_t piece,size_t offset,size_t length)
{
	pSocket *sock = peer -> PeerSocket();
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (p -> GetNumber() == piece)
		{
			// TODO: check status
			p -> SetRequested(offset, length);
			if (sock)
			{
				sock -> SendRequest(piece, offset, length);
			}
			return new Request(piece, offset, length);
		}
	}
	return NULL;
}


void Session::RemoveRequest(Peer *peer,size_t piece,size_t offset,size_t length)
{
	pSocket *sock = peer -> PeerSocket();
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (p -> GetNumber() == piece)
		{
			// TODO: check status
			p->SetNotRequested(offset, length);
			if (sock)
			{
				sock -> SendCancel(piece, offset, length);
			}
			return;
		}
	}
}

//! Modified by Amir Krifa.

void Session::RetryRequest(Peer *peer,size_t piece,size_t offset,size_t length)
{
	pSocket *sock = peer -> PeerSocket();
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (p -> GetNumber() == piece)
		{
			p->SetRequested(offset, length);
			if (sock)
			{
				sock -> SendRequest(piece, offset, length);
			}
			return;
		}
	}
}

//! Modified by Amir Krifa.

void Session::SaveSlice(size_t piece,size_t offset,size_t length,unsigned char *buf)
{
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (p -> GetNumber() == piece)
		{
			if(!p->Complete(offset))
			{
				p -> SetComplete(offset, length);
				if (m_filemanager)
				{
						//uiTotalR+=length;
						m_filemanager -> Write(piece, buf, length, offset);
				}
				break;
			} else {
				// slice already received :)
				int alr = 0;
				alr++;
			}
			break;
		}
	}
	
}


void Session::CreateFileManager()
{
	m_filemanager = new FileManager(static_cast<PeerHandler&>(Handler()), m_info_hash, bSeeding);
}


Piece *Session::GetIncomplete(size_t piece)
{
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (p -> GetNumber() == piece)
			return p;
	}
	return NULL;
}


Piece *Session::GetComplete(size_t piece)
{
	for (piece_v::iterator it = m_complete.begin(); it != m_complete.end(); it++)
	{
		Piece *p = *it;
		if (p -> GetNumber() == piece)
			return p;
	}
	return NULL;
}


void Session::Verify()
{
	bool repeat;
	for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
	{
		Piece *p = *it;
		if (m_filemanager -> Verify(p -> GetNumber(),p -> PieceLength()) )
		{
			for (size_t i = 0; i < p -> PieceLength(); i += p -> SliceSize())
			{
				p -> SetComplete(i, MIN(p -> SliceSize(),p -> PieceLength() - i) );
			}
		}
	}
	do {
		repeat = false;
		for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
		{
			Piece *p = *it;
			if (p -> Complete())
			{
				m_complete.push_back(p);
				p -> ClearComplete();
				p -> ClearRequested();
				m_incomplete.erase(it);
				repeat = true;
				break;
			}
		}
	} while (repeat);
}


void Session::SendSlice(pSocket *sock,size_t piece,size_t offset,size_t length)
{
	Piece *p = GetComplete(piece);
	if (p)
	{	uiTotalS+=length;
		unsigned char *buf = new unsigned char[m_piece_length];
		m_filemanager -> ReadPiece(piece, buf, m_piece_length );
		

		uint32_t l = htonl(length + 9);
		sock -> SendBuf( (char *)&l,4);
		sock -> Send("\07");
		
		l = htonl(piece);
		sock -> SendBuf( (char *)&l, 4);
		
		l = htonl(offset);
		sock -> SendBuf( (char *)&l,4);
		
		sock -> SendBuf( (char*)&buf[offset], length);
		
		delete buf;
		p -> SetRequested(offset, length);
	}
}


bool Session::SliceSent(size_t piece,size_t offset)
{
	Piece *p = GetComplete(piece);
	if (p)
	{
		if (p -> Requested(offset))
			return true;
	}
	return false;
}


void Session::save_piece_v(FILE *fil,piece_v& ref)
{
	size_t q = ref.size();
	fwrite(&q, sizeof(size_t), 1, fil);
	for (piece_v::iterator it = ref.begin(); it != ref.end(); it++)
	{
		Piece *p = *it;
		size_t nr = p -> GetNumber();
		fwrite(&nr, sizeof(size_t), 1, fil);
		nr = p -> PieceLength();
		fwrite(&nr, sizeof(size_t), 1, fil);
		p -> save_slice_m(fil, p -> MapComplete());
		p -> save_slice_m(fil, p -> MapRequested());
	}
}


void Session::load_piece_v(FILE *fil,piece_v& ref)
{
	
	size_t q;
	fread(&q, sizeof(size_t), 1, fil);
	while (q--)
	{
		size_t nr, piece_length;
		fread(&nr, sizeof(size_t), 1, fil);
		fread(&piece_length, sizeof(size_t), 1, fil);
		Piece *p = new Piece(nr, piece_length);
		p -> load_slice_m(fil, p -> MapComplete());
		p -> load_slice_m(fil, p -> MapRequested());
		ref.push_back(p);
		p=NULL;
	}
}


bool Session::GenerateRequest(Peer *peer)
{
	request_v& reqs = peer -> Requests();
	size_t piece;
	size_t offset;
	size_t length;
	//TOVERIFY
	if (GetRandomNotRequested(peer, piece, offset, length))
	{
		Request *r = AddRequest(peer, piece, offset, length);
		if (r)
		{
			reqs.push_back(r);
		}
		return true;
	}else
	{
		if(!this->m_incomplete.empty() && reqs.empty())
		{
			// No requests qnd there is some incomplete pieces
			for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
			{
				Piece *p = *it;
				if (!p -> Complete())
				{
					p->GetRandomRequestedButIncomplete(offset, length);
					Request *r = AddRequest(peer, p->GetNumber(), offset, length);
					if (r)
					{
						reqs.push_back(r);
					}
				}	
			}
						
		}
		
		return true;
	}
}


bool Session::PieceUnique(size_t piece)
{
	for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
	{
		Peer *peer = *it;
		if (peer -> IsSet(piece))
		{
			return false;
		}
	}
	return true;
}


void Session::PeerStatus() 
{
	this->UpdateElapsedTime();
	__int64 totalReceived = this->GetTotalBytesR();
	AddItemDownloaded(ConvertLPCSTRToLPWSTR((char *)this->m_name.c_str()),totalReceived ,this->m_peers.size(),this->GetTotalBytesS(),this->GetElapsedTime());
	
	if(ProgressBar && !this->isSeeding() && !this->bAlreadyDownloaded )
	{
		if(ProgressBarStatusChanged)
		{
			// The details windows was closed and know it is opened
			int64_t iStep;
			iStep = totalReceived;
			uLastTotalRecievedBytes = iStep;
			if(iStep > 0)
			{
   				HWND hProgress = GetDlgItem(hwndTD,IDC_PROGRESS_DOWNLOAD);
				int i;
				int max = iStep/abs(iSliceSize);
				for(i=0;i<max;i++)
				{
					SendMessage(hProgress, PBM_STEPIT, 0, 0); 
				}
				
				// Downloading Speed:
				SetDlgItemText(hwndTD,IDC_EDIT_DOWNLOAD_SPEED,ConvertLPCSTRToLPWSTR((char*)this->GetDownloadSpeed().c_str()));
				CloseHandle(hProgress);
				ProgressBarStatusChanged = false;
			}
		}else 
		{
			// The status of the Details windows does not changed (open)
			int64_t iStep;
			iStep = totalReceived - uLastTotalRecievedBytes;
			uLastTotalRecievedBytes = totalReceived;
			if(iStep > 0)
			{
     			HWND hProgress = GetDlgItem(hwndTD,IDC_PROGRESS_DOWNLOAD);
				int i;
				int max = iStep/abs(iSliceSize);
				for(i = 0; i < max;i++)
				{
					SendMessage(hProgress, PBM_STEPIT, 0, 0); 
				}
				// Downloading Speed:
				SetDlgItemText(hwndTD,IDC_EDIT_DOWNLOAD_SPEED,ConvertLPCSTRToLPWSTR((char*)this->GetDownloadSpeed().c_str()));
				CloseHandle(hProgress);
				ProgressBarStatusChanged = false;
			}

		}
	}
}



//! Modified by Amir Krifa.
void Session::Update()
{
	this->UpdateElapsedTime();
	
	PeerHandler& ref = static_cast<PeerHandler&>(Handler());
	
	// add connections
	size_t q = ref.PeerCount(m_info_hash);

	if(bRecoverFromError)
	{
		
		piece_v::iterator iter = this->m_incomplete.begin();
		while(iter!=this->m_incomplete.end())
		{
			(*iter)->ReChek();
			iter++;
		}

		peer_v::iterator pIter = m_peers.begin();
		while(pIter!=this->m_peers.end())
		{
			(*pIter)->RemoveRequests();
			pIter++;
		}
		AddConnect();
		
		bRecoverFromError=false;
		
	}

	if (q < ref.GetMinPeers() || this->iPaused==1)
	{
		AddConnect();
		bRecoverFromError = false;
	}

	// update interested
	if (m_b_update_interested)
	{
		Peer *peer = NULL;
		pSocket *sock = NULL;
			
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			peer = *it;
			sock = peer -> PeerSocket();
			
			if (sock && sock -> CTS())
			{

				bool interested = false;
				Piece *p = NULL;
					
				for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
				{
					p = *it;
					if (peer -> IsSet(p -> GetNumber()))
					{
						interested = true;
						break;
					}
				}
				p = NULL;

				if (interested != sock -> Interested())
				{
					sock -> SendInterest(interested);
				}
			} // Connected
		}

		peer = NULL;
		sock = NULL;
		m_b_update_interested = false;
	}

	if(!this->bSeeding && !this->bAlreadyDownloaded)
	{
		Peer *peer = NULL;
		pSocket *sock = NULL;
		int64_t ant = 0;
		// make sure all unchoked pSocket's has at least 1 piece OR 256 kB requested data
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			peer = *it;
			request_v& reqs = peer -> Requests();
			// TODO: ignore choke
			sock = peer -> PeerSocket();
			// peer -> IsChoked - Choke received from remote end
			// sock -> Choked - we sent Choke to remote end
			if ((sock && sock -> CTS() && !peer -> IsChoked()) )
			{
				ant = GetPieceLength() / iSliceSize;
				// min number of requests to queue
				if (reqs.size() < 2)
				{
					RequestAvailable(peer);	
				}
				iPaused=0;
			}
			else
			{

			}
		}
		peer = NULL;
		sock = NULL;
	}


	// check for completed pieces
	{
		if (m_b_check_complete)
		{
			
			Piece *p = NULL;
			char szPc[100];
			piece_v::iterator it = m_incomplete.begin();
			while(it != m_incomplete.end())
			{
				p = *it;
				if (p -> Complete())
				{
					this->iNbrPieceComplete++;
					memset(szPc,'\0', 100);
					sprintf(szPc,"%i",iNbrPieceComplete);
					if(ACTIVATE_LOG)
						this->session_log->add_line(string("Number of pieces downloaded : ")+string(szPc),true);
					if (m_filemanager -> Verify(p -> GetNumber(),p -> PieceLength()) )
					{
						for (size_t i = 0; i < p -> PieceLength(); i += p -> SliceSize())
						{
							p -> SetComplete(i, MIN(p -> SliceSize(),p -> PieceLength() - i) );
						}
						
						m_complete.push_back(p);
						ref.SendHave(m_info_hash, p -> GetNumber());
						p -> ClearComplete();
						p -> ClearRequested();
						// Delete the piece
						it = m_incomplete.erase(it);
						//p = NULL;
						continue;
					}
					else
					{
						p -> ClearComplete();
						p -> ClearRequested();
					}
				}
				it++;
			}
		
			m_b_check_complete = false;
		}
	}	
	
	//! check age of requests
	if (time(NULL) % 5 == 0 && !bAlreadyDownloaded && !isSeeding())
	{
		CheckRequests();
	}
	

	// Check if the file was already downloaded or not
	if (m_incomplete.empty() && this->bAlreadyDownloaded && !this->isSeeding())
	{
		if(this->m_filemanager->IsComplete())
		{
			MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string(this->GetName()+" was already downloaded.").c_str()),
			TEXT("Downloading Completed!"), MB_OK|MB_ICONINFORMATION );
			int res=MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string("Would you like to seed the file "+this->GetName()+" to other peers?" ).c_str()),
			TEXT("Seeding!"), MB_YESNO|MB_ICONINFORMATION );
			if(res == IDYES)
			{
				this->bSeeding = true;
				this->bAlreadyDownloaded = true;
				ChangeTorrentStatus(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()),_T("Seeding"));
				this->SetCheckComplete();
			}else 
			{
				//! Delete the torrent from the downloading list
				this->bToDelete = true;
				int pos = IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()));
				if(pos>=0)
				{
					HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
					ListView_DeleteItem(hList,pos);
					CloseHandle(hList);
				}
				
				UpdateDownloadListButtons();
			}
		}else
		{
			//! The saved state files indicates that the data file was already downloaded however the size of the data file is not correct
			MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string(string("Error in the size of ")+this->GetName()+"."+string(" All related files in the downloading directory will be deleted.")).c_str()),
			TEXT("Error in the data file size."), MB_OK|MB_ICONSTOP );
			//! Delete the torrent from the downloading list
			this->bToDelete=true;
			this->bDeleteRelatedFiles=true;
			//! The peerHandler will delete this session
			int pos=IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()));
			if(pos>=0)
			{
				HWND hList = GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
				ListView_DeleteItem(hList,pos);
				CloseHandle(hList);
			}
			UpdateDownloadListButtons();
		}
	}

	//! check for all complete and switch to seeding mode 
	if (m_incomplete.empty() && !this->isSeeding() && !bToDelete && !this->bAlreadyDownloaded)
	{
		if(!bContinueSeeding)
		{
			if(this->m_filemanager->IsComplete())
			{
				std::string sEndLog;
				//! Local ip @
				sEndLog.append(this->GetLocalIpAdr()+"  ");
				//! File Name
				sEndLog.append(this->GetName()+"  ");
				//! Number of received bytes
				std::ostringstream oss;
				oss<<this->GetTotalBytesR();
				sEndLog.append(oss.str()+"  ");
				//! Elapsed Time
				std::ostringstream oss2;
				oss2<<this->GetElapsedTime();
				sEndLog.append(oss2.str());
				sEndLog.append("\n");
				if(ACTIVATE_LOG)
					downloaded_files_log->add_line(sEndLog,true);
		
			
				MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string(this->GetName()+" was successfully downloaded.").c_str()),
				TEXT("Downloading Completed!"), MB_OK|MB_ICONINFORMATION );
				//!All Complete
		
				int res=MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string("Would you like to seed the file "+this->GetName()+" to other peers?" ).c_str()),
				TEXT("Seeding!"), MB_YESNO|MB_ICONINFORMATION );
				if(res==IDYES)
				{
					this->bSeeding = true;
					this->bAlreadyDownloaded = true;
					this->SuspendDownloading(false);
					//! Change the torrent status to seeding
					ChangeTorrentStatus(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()),_T("Seeding"));
					this->SetCheckComplete();
					
				}else {
					//! Delete the torrent from the downloading list
					this->bToDelete=true;
					this->SuspendDownloading(true);
					int pos = IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()));
					if(pos>=0)
					{
						HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
						ListView_DeleteItem(hList,pos);
						CloseHandle(hList);
					}
					UpdateDownloadListButtons();
				}
		
			}else
			{
				//! The saved state files indicates that the data file was already downloaded however the size of the data file is not correct
				MessageBox(hwndDL,ConvertLPCSTRToLPWSTR((char*)std::string(string("Error in the size of ")+this->GetName()+"."+string(" All related files in the downloading directory will be deleted.")).c_str()),
				TEXT("Error in the data file size."), MB_OK|MB_ICONSTOP );
				this->bToDelete=true;
				this->SuspendDownloading(true);
				int pos=IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()));
				if(pos>=0)
				{
					HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
					ListView_DeleteItem(hList,pos);
					CloseHandle(hList);
				}
				UpdateDownloadListButtons();
			}
		}else
		{
			if(this->m_filemanager->IsComplete())
			{
				std::string sEndLog;
				//! Local ip @
				sEndLog.append(this->GetLocalIpAdr()+"  ");
				//! File Name
				sEndLog.append(this->GetName()+"  ");
				//! Number of received bytes
				std::ostringstream oss;
				oss<<this->GetTotalBytesR();
				sEndLog.append(oss.str()+"  ");
				//! Elapsed Time
				std::ostringstream oss2;
				oss2<<this->GetElapsedTime();
				sEndLog.append(oss2.str());
				sEndLog.append("\n");
				if(ACTIVATE_LOG)
					downloaded_files_log->add_line(sEndLog,true);
				this->SuspendDownloading(false);
				this->bSeeding=true;
				this->bAlreadyDownloaded = true;
				//! Change the torrent status to seeding
				ChangeTorrentStatus(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()),_T("Seeding"));
			
			}else
			{
				//! The saved state files indicates that the data file was already downloaded however the size of the data file is not correct
				//! Delete the torrent from the downloading list
				this->bToDelete=true;
				this->SuspendDownloading(true);
				//! The peerHandler will delete this session
				int pos=IsItInTheDownloadList(ConvertLPCSTRToLPWSTR((char *)this->GetName().c_str()));
				if(pos>=0)
				{
					HWND hList=GetDlgItem(hwndDL,IDC_LIST_TORRENTS);
					ListView_DeleteItem(hList,pos);
					CloseHandle(hList);
				}
				UpdateDownloadListButtons();
			}
		}
	
	}

	//! choke downloader with lowest d/l rate
	if ((time(NULL) % iChokingPeriod == 0 ) && this->m_peers.size() > 4)
	{
		ref.CheckDownloadRate();
	}

	//! get number of downloaders
	if (time(NULL) % 4 == 0 && !m_peers.empty())	
	{
			size_t qd = 0;
			std::vector<Peer *> interested_choked;

			{
				pSocket *sock = NULL;
				Peer *peer = NULL;
				for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
				{
					peer = *it;
					sock = peer -> PeerSocket();
					if (sock)
					{
						//if (peer -> IsInterested() && sock -> Choked())
						if (sock -> Choked())
						{
							interested_choked.push_back(peer);
						}
						if (!sock -> Choked())
						{
							qd++;
						}
					}
				}
				peer = NULL;
				sock = NULL;
			}
		
			q = interested_choked.size();

			if (qd < ref.GetDownloaders() + ref.GetOptimistic() && q > 0)
			{
				Peer *peer = NULL;;

				if(WM_BITTORRENT_VERSION == 2)
				{
					//! Choose iQ times a node within the iPeersScope and after another node out of iPeersScope
					iCount++;
					if(iCount == iQ+1)
					{
						//! Choose a far node
						peer = ChooseAPeer(interested_choked,pLastChoosedPeer,iPeersScope+1);
						iCount = 0;
					}else{
						//! Choose a node within iPeersScope
						peer = ChooseAPeer(interested_choked,pLastChoosedPeer,iPeersScope-1);
					}
					pLastChoosedPeer = peer;
				}else{
					//! Choose a peer Randomly
					peer = ChooseAPeer(interested_choked,pLastChoosedPeer,iPeersScope+1);
				}

				
				if(peer != NULL)
				{
					pSocket *sock = peer -> PeerSocket();
					if (sock)
					{
						sock -> SendChoke(false);
					}
					peer = NULL;
					sock = NULL;
				}
			}
		}
	
	}



Peer* Session::ChooseAPeer(std::vector<Peer *> & pv,Peer * diff,int scope)
{
	if(WM_BITTORRENT_VERSION==2)
	{
		//! Version adapted to an adhoc network
		//! Choose iQ times from nodes within iPeersScope and 1 time from nodes out of iPeersScope
		std::vector<Peer *>::iterator iter=pv.begin();
	
		while(iter!=pv.end())
		{
			if(pv.size()>1)
			{
				if( scope>iPeersScope && *iter!=diff && (*iter)->GetNumberOfHops()>iPeersScope)
					return *iter;
				if( scope<iPeersScope && *iter!=diff && (*iter)->GetNumberOfHops()<=iPeersScope)
					return *iter;
			}else
			{
				if( scope>iPeersScope && (*iter)->GetNumberOfHops()>iPeersScope)
					return *iter;
				if( scope<iPeersScope &&(*iter)->GetNumberOfHops()<=iPeersScope)
					return *iter;
			}
			iter++;
		}
		return NULL;
	}else
		{	//! Internet version
			//! Choose a random peer
			return pv[random()%pv.size()];
		}
}



void Session::Status(TcpSocket *p) 
{
	
}


size_t Session::Available(int piece)
{
	size_t q = 0;
	if(WM_BITTORRENT_VERSION==1)
	{
		
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *peer = *it;
			if (peer -> IsSet(piece) && !peer -> IsChoked())
				q++;
		}
		return q;
	}else if(WM_BITTORRENT_VERSION==2)
	{
		
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *peer = *it;
			if(peer->GetNumberOfHops()<=iPeersScope)
			{
				if (peer -> IsSet(piece) && !peer -> IsChoked())
					q++;
			}
		}
		return q;
	}
	return q;
}

bool Session::IsAbsent(int piece)
{
	if(WM_BITTORRENT_VERSION==2)
	{
		size_t q = 0;
		for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
		{
			Peer *peer = *it;
			if(peer->GetNumberOfHops()<=iPeersScope)
			{
				if (peer -> IsSet(piece) && !peer -> IsChoked())
					return false;
			}
		}
		return true;
	}
	return true;
}

//! Modified by Amir Krifa.

void Session::RequestAvailable(Peer *peer)
{
	request_v& reqs = peer -> Requests();
	int64_t ant = GetPieceLength() / iSliceSize;
	int64_t q = 2; // requests to add
	
		
	if(reqs.size() < q)
	{
		if(WM_BITTORRENT_VERSION==1)
		{	// add requests for least available (Raerest Firts)
			if (q > 0)
			{
				std::map<size_t,piece_v> mmap;
				for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end() && q > 0; it++)
				{
					Piece *p = *it;
					if (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested())
					{
						mmap[Available(p -> GetNumber())].push_back(p);
					}
				}
			
				piece_v tmp;
				{
					for (std::map<size_t,piece_v>::iterator it = mmap.begin(); it != mmap.end(); it++)
					{
						piece_v& ref = (*it).second;
						for (piece_v::iterator it2 = ref.begin(); it2 != ref.end(); it2++)
						{
							Piece *p = *it2;
							tmp.push_back(p);
						}
					}
				}	
				{
					for (piece_v::iterator it = tmp.begin(); it != tmp.end() && q > 0; it++)
					{
						Piece *p = *it;
						int max = 20;
						while (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested() && max-- && q--)
						{
							size_t offset;
							size_t length;
							if (p -> GetRandomNotRequested(offset, length))
							{
								Request *r = AddRequest(peer, p -> GetNumber(), offset, length);
								if (r)
								{
									reqs.push_back(r);
								}
							}
							else
							{
								//GetRandomNotRequested failed
								if(ACTIVATE_LOG)
									this->session_log->add_line(string("GetRandomNotRequested failed."),true);
							}
						}
					}
				}
			}
		}
		else
		{
			//! Rearest First within iPeersScope if the hops(peer) <= iPeersScope and Absent first if hops(peer)>iPeersScope
			if (q > 0)
			{
				if(peer->GetNumberOfHops() <= iPeersScope)
				{   //! A peer within the scope
					//! Policy Raerest First
					std::map<size_t,piece_v> mmap;
					for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end(); it++)
					{
						Piece *p = *it;
						if (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested())
						{
							mmap[Available(p -> GetNumber())].push_back(p);
						}
					}

					piece_v tmp;
					{
						for (std::map<size_t,piece_v>::iterator it = mmap.begin(); it != mmap.end(); it++)
						{
							piece_v& ref = (*it).second;
							for (piece_v::iterator it2 = ref.begin(); it2 != ref.end(); it2++)
							{
								Piece *p = *it2;
								tmp.push_back(p);
							}
						}
					}

					{
						for (piece_v::iterator it = tmp.begin(); it != tmp.end() && q > 0; it++)
						{
							Piece *p = *it;
							while (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested() && q--)
							{
								size_t offset;
								size_t length;
								if (p -> GetRandomNotRequested(offset, length))
								{
									Request *r = AddRequest(peer, p -> GetNumber(), offset, length);
									if (r)
									{
										reqs.push_back(r);
									}
									r = NULL;
								}
								else
								{
									//GetRandomNotRequested failed
									if(ACTIVATE_LOG)
										this->session_log->add_line(string("GetRandomNotRequested failed."),true);
								}
							}
						}
					}
				}else{
					//! Peer Out of the Scope 
					//! Policy== Absent Piece First

					piece_v tmp;
					for (piece_v::iterator it = m_incomplete.begin(); it != m_incomplete.end() && q > 0; it++)
					{
						Piece *p = *it;
						if (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested() && IsAbsent(p->GetNumber()))
						{
							tmp.push_back(p);
						}
					}
				
						
					{
						for (piece_v::iterator it = tmp.begin(); it != tmp.end() && q > 0; it++)
						{
							Piece *p = *it;
							while (peer -> IsSet(p -> GetNumber()) && !p -> AllRequested() && q--)
								{
									size_t offset;
									size_t length;
										if (p -> GetRandomNotRequested(offset, length))
										{
											Request *r = AddRequest(peer, p -> GetNumber(), offset, length);
											if (r)
											{
												reqs.push_back(r);
											}
											r = NULL;
										}
										else
										{
											//GetRandomNotRequested failed
											if(ACTIVATE_LOG)
												this->session_log->add_line(string("GetRandomNotRequested failed."),true);
										}
							}
						}
					}
				}
			}
		}
		}
}


void Session::RemoveRequests(Peer *peer)
{
	request_v& reqs = peer -> Requests();
	while (!reqs.empty())
	{
		request_v::iterator it = reqs.begin();
		Request *r = *it;
		size_t piece = r -> GetPiece();
		size_t offset = r -> GetOffset();
		size_t length = r -> GetLength();
		RemoveRequest(peer, piece, offset, length);
		delete r;
		reqs.erase(it);
	}
}


void Session::CheckRequests()
{
	for (peer_v::iterator it = m_peers.begin(); it != m_peers.end(); it++)
	{
		Peer *peer = *it;
		request_v& reqs = peer -> Requests();
		request_v::iterator itr = reqs.begin();
		while(itr != reqs.end())
		{
			Request *r = *itr;
			int age = abs(r -> Age());
			int max = static_cast<PeerHandler&>(Handler()).MaxRequestAge();
			if (age > max)
			{
				size_t piece = r -> GetPiece();
				size_t offset = r -> GetOffset();
				size_t length = r -> GetLength();
				RetryRequest(peer, piece, offset, length);
				
			}
			itr++;
		}
	}
}

// Added by Amir Krifa
// Returns the total received bytes.
int64_t Session::GetTotalBytesR()
{
	piece_v::iterator iInComp=this->m_incomplete.begin();
	piece_v::iterator iComp=this->m_complete.begin();
	
	this->uiTotalR=0;
	while(iComp!=this->m_complete.end())
	{
		this->uiTotalR+=(*iComp)->GetPieceLength();
		iComp++;
	}
	while(iInComp!=this->m_incomplete.end())
	{
		this->uiTotalR+=(*iInComp)->GetCompletedBytes();
		iInComp++;
	}
	if(uiTotalR < uLastTR)
	{
		return uiTotalR;
	}
	this->uLastTR=uiTotalR;
	return this->uiTotalR;
}

//! Added by Amir krifa.
//! Returns the total bytes sent.
int64_t Session::GetTotalBytesS()
{
	return this->uiTotalS;
}


//! Added by Amir Krifa.
//! Suspend for a while the downloading process.
void Session::SuspendDownloading(bool deleting)
{
	if(!this->m_incomplete.empty())
	{
			piece_v::iterator iter=this->m_incomplete.begin();
			while(iter!=this->m_incomplete.end())
			{
				(*iter)->ClearRequested();
				iter++;
			}
	}

	if(!this->m_peers.empty())
	{
			peer_v::iterator iter = m_peers.begin();
			while(iter!=this->m_peers.end())
			{
				if((*iter))
				{	
					(*iter)->RemoveRequests();
					if((*iter)->PeerSocket())
					{
						if(!deleting)
						{
							(*iter)->PeerSocket()->SendChoke(false);
							(*iter)->PeerSocket()->SendInterest(true);
						}
						(*iter)->PeerSocket()->SetCloseAndDelete();
						delete (*iter)->PeerSocket();
					}
				}
				iter++;
			}
			Sleep(2);
	}
	this->iPaused=1;
}

string Session::GetDownloadSpeed()
{
	if(this->uElapsedTime==0)
	{
		return string("0");
	}else
	{
		int64_t fDspeed=(this->uiTotalR/1024)/this->uElapsedTime;
		std::ostringstream oss;
		oss<<fDspeed;
		return oss.str();
	}
	return string("0");
	
}


//! Added by Amir Krifa.
//! Returns the device local ip @.
string Session::GetLocalIpAdr()
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

std::string Session::GetPeerNumberIp(int nbr)
{
	string ip("");
	if(nbr<=abs(this->m_peers.size()))
	{
		peer_v::iterator pi=this->m_peers.begin();
		for(int i=0;i<nbr;i++)
			pi++;
		ip.assign((*pi)->GetIP());
	}
	return ip;

}
int Session::GetPeerNumberNbrHops(int nbr)
{
	int iNbrHops;
	if(nbr<=abs(this->m_peers.size()))
	{
		peer_v::iterator pi=this->m_peers.begin();
		for(int i=0;i<nbr;i++)
			pi++;
		iNbrHops=(*pi)->GetNumberOfHops();
	}
	return iNbrHops;
}

void Session::UpdateSeedingStatus()
{
	PeerHandler& ref = static_cast<PeerHandler&>(Handler());
	for (piece_v::iterator it = m_complete.begin(); it != m_complete.end(); it++)
	{
		ref.SendHave(m_info_hash, (*it) -> GetNumber());
	}
}

void Session::AddUBytesToTrafficMatrix(string host, size_t bytes)
{
	if(u_traffic_matrix_map.empty())
	{
		// A new host, we have to add it and init the number of bytes
		u_traffic_matrix_map.insert(make_pair<string, size_t>(host, bytes));
	}
	else
	{
		std::map<string, size_t>::iterator iter;
		if((iter = u_traffic_matrix_map.find(host)) != u_traffic_matrix_map.end())
		{
			// Host found, so we will add bytes
			iter->second = iter->second + bytes;

		}
		else
		{
			// A new host, we have to add it and init the number of bytes
			u_traffic_matrix_map.insert(make_pair<string, size_t>(host, bytes));
		}
	}
}

void Session::AddDBytesToTrafficMatrix(string host, size_t bytes)
{
	if(d_traffic_matrix_map.empty())
	{
		// A new host, we have to add it and init the number of bytes
		d_traffic_matrix_map.insert(make_pair<string, size_t>(host, bytes));
	}
	else
	{
		std::map<string, size_t>::iterator iter;
		if((iter = d_traffic_matrix_map.find(host)) != d_traffic_matrix_map.end())
		{
			// Host found, so we will add bytes
			iter->second = iter->second + bytes;

		}
		else
		{
			// A new host, we have to add it and init the number of bytes
			d_traffic_matrix_map.insert(make_pair<string, size_t>(host, bytes));
		}
	}
}


void Session::AddNewCharingRatioToMatrix(string ips,string ipd, double rate)
{
	if(sharing_ratio_matrix.empty())
	{
		// A new host, we have to add it and init the number of bytes
		sharing_ratio_matrix.insert(make_pair<string, couple>(ipd, couple(ips,rate)));
	}
	else
	{
		std::map<string, couple>::iterator iter;
		if((iter = sharing_ratio_matrix.find(ipd)) != sharing_ratio_matrix.end())
		{
			// Host found, so we update the rate
			iter->second.sharingR = rate;

		}else
		{
			// A new host, we have to add it and init the number of bytes
			sharing_ratio_matrix.insert(make_pair<string, couple>(ipd, couple(ips, rate)));
		}
	}
}

void Session::CalculateTheSharingRatioMatrix()
{
	// R(i,j) = min(U(i,j), D(i,j)) / max(U(i,j), D(i,j))
		
	if(!d_traffic_matrix_map.empty() && !u_traffic_matrix_map.empty())
	{	
		std::map<string, size_t>::iterator iterD;
		std::map<string, size_t>::iterator iterU;
		double Rij = 0;
		
		for(iterD = d_traffic_matrix_map.begin(); iterD != d_traffic_matrix_map.end(); iterD++)
		{
			if((iterU = u_traffic_matrix_map.find(iterD->first)) != u_traffic_matrix_map.end())
			{
				// U(i,j) <> 0  and D(i,j) <> 0
				Rij =((double) GetMin<size_t>(iterD->second, iterU->second)) / ((double)GetMax<size_t>(iterD->second, iterU->second));
				AddNewCharingRatioToMatrix(GetLocalIpAdr(), iterD->first, Rij);
			}else
			{
				// We suppose that U(i,j) == 0
				AddNewCharingRatioToMatrix(GetLocalIpAdr(), iterD->first, 0);
			}
		}
	}else if(!u_traffic_matrix_map.empty() && d_traffic_matrix_map.empty())
	{
		std::map<string, size_t>::iterator iterU;
		
		for(iterU = u_traffic_matrix_map.begin(); iterU != u_traffic_matrix_map.end(); iterU++)
		{
			// We suppose that D(i,j) == 0
			AddNewCharingRatioToMatrix(GetLocalIpAdr(), iterU->first, 0);
		}
	}else if(u_traffic_matrix_map.empty() && !d_traffic_matrix_map.empty())
	{
		std::map<string, size_t>::iterator iterD;
		for(iterD = d_traffic_matrix_map.begin(); iterD != d_traffic_matrix_map.end(); iterD++)
		{
			// We suppose that U(i,j) == 0
			AddNewCharingRatioToMatrix(GetLocalIpAdr(), iterD->first, 0);
		}

	}
}


void Session::WriteTheSharingRatioMatrixToLogFile()
{

	LogFile l(this->sLogDirectory + this->GetName()+ string("-srm.txt"));
	if(!sharing_ratio_matrix.empty())
	{	
		
		std::map<std::string, couple>::iterator iter = sharing_ratio_matrix.begin();
		
		while(iter != sharing_ratio_matrix.end())
		{
			string line;
			line.append(iter->second.ips);
			line.append(" ");
			line.append(iter->first);
			line.append(" ");
			std::ostringstream oss;
			oss<<iter->second.sharingR;
			line.append(oss.str());
			if(ACTIVATE_LOG)
				l.add_line(line,false);

			iter++;
		}
		
	}
	l.close();
}
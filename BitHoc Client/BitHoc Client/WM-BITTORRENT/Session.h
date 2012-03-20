
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
* \file Session.h
* \brief Status of a Bittorent downloading session.
**/

#include "stdafx.h"
#ifndef _SESSION_H
#define _SESSION_H

#include <string>
#ifndef _WIN32
#include <stdint.h>
#else
typedef __int64 int64_t;
#include <time.h>
#endif
#include <list>
#include <SocketHandler.h>
#include "bitmap_t.h"


class Peer;
class Piece;
class Request;
class FileManager;
class pSocket;
class TcpSocket;
class LogFile;

typedef std::list<Piece *> piece_v;
//! File information from metainfo file
struct file_t {
	file_t(const std::string& n,int64_t l) : name(n),offset(0),length(l) {}
	std::string name;
	int64_t offset; // file begin
	int64_t length;
};
typedef std::list<file_t *> file_v;

typedef struct couple
{
	couple(std::string ip, double sr)
	{
		ips = ip;
		sharingR = sr;
	}
	void Init(std::string ip, double sr)
	{
		ips = ip;
		sharingR = sr;
	}
	std::string ips;
	double sharingR;
}couple;

/*!
* \class Session
* \brief Information about one metainfo file / transfer session.
*/
class Session
{
	typedef std::list<Peer *> peer_v;
public:
	Session(SocketHandler&,const std::string& info_hash, bool,std::string l);
	virtual ~Session();

	/*!
	* \fn const std::string& GetInfoHash()
	* \brief Return the Info_hash MetaInfo field. 
	*/
	const std::string& GetInfoHash() { return m_info_hash; }
	SocketHandler& Handler() const { return m_handler; }
	/*!
	* \fn void SetHash(unsigned char *p)
	* \brief Sets the torrent hash. 
	*/
	void SetHash(unsigned char *p) { memcpy(m_hashptr, p, 20); }
	unsigned char *GetHashptr() { return m_hashptr; }
	/*!
	* \fn unsigned char *GetPeerId()
	* \brief Returns the peer id. 
	*/
	unsigned char *GetPeerId() { return m_peer_id; }
	/*!
	* \fn void SetAnnounce(const std::string& a)
	* \brief Set the announce MetaInfo field. 
	*/
	void SetAnnounce(const std::string& a) { m_announce = a; }
	/*!
	* \fn const std::string& GetAnnounce()
	* \brief Returns the announce MetaInfo field. 
	*/
	const std::string& GetAnnounce() { return m_announce; }
	/*!
	* \fn void SetName(const std::string& a)
	* \brief Sets the metainfo file name. 
	*/
	void SetName(const std::string& a) { m_name = a; }
	/*!
	* \fn const std::string& GetName()
	* \brief Returns the metainfo file name. 
	*/
	const std::string& GetName() { return m_name; }
	/*!
	* \fn void SetPieceLength(int64_t x)
	* \brief Sets the piece length. 
	*/
	void SetPieceLength(int64_t x);
	/*!
	* \fn size_t GetPieceLength()
	* \brief Returns the piece length. 
	*/
	int64_t GetPieceLength() { return m_piece_length; }
	void AddFile(int64_t);
	void AddFile(const std::string& path,int64_t);

	/*!
	* \fn void Load()
	* \brief Restores an already saved session. 
	*/	
	void Load();
	/*!
	* \fn void Save()
	* \brief Used to save the current session into a binary file. 
	*/	
	void Save();
	/*!
	* \fn std::string GetBitmapFilename()
	* \brief Returns the file name where the session was saved. 
	*/
	std::string GetBitmapFilename();

	/*!
	* \fn void SetInterval(int x)
	* \brief Sets the tracker announce interval. 
	*/
	void SetInterval(int x) { m_interval = x; }
	/*!
	* \fn int GetInterval()
	* \brief Returns the tracker announce interval. 
	*/
	int GetInterval() { return m_interval; }
	/*!
	* \fn void AddPeer(Peer *p)
	* \brief Used to add a new peer to the torrent. 
	*/
	void AddPeer(Peer *p) { m_peers.push_back(p); }
	/*!
	* \fn Peer *GetPeer(const std::string& ip)
	* \brief Used to get a peer from the Ip address. 
	*/
	Peer *GetPeer(const std::string& ip);
	/*!
	* \fn void SetTimeTracker()
	* \brief Used to tracker time. 
	*/	
	void SetTimeTracker() { m_t_tracker = time(NULL); }
	/*!
	* \fn time_t GetTimeTracker()
	* \brief Returns the tracker time. 
	*/
	time_t GetTimeTracker() { return m_t_tracker; }
	/*!
	* \fn std::string GetAnnounceUrl(const std::string& event)
	* \brief Returns the tracker announce url. 
	*/	
	std::string GetAnnounceUrl(const std::string& event); // started, stopped
	/*!
	* \fn size_t GetNumberOfPieces()
	* \brief Returns the number of pieces. 
	*/
	size_t GetNumberOfPieces() { return m_number_of_pieces; }
	/*!
	* \fn void SetPieces(const std::string& )
	* \brief Used to set the pieces hash. 
	*/
	void SetPieces(const std::string& );
	/*!
	* \fn int64_t GetLastLength()
	* \brief Returns the length of the last piece. 
	*/
	int64_t GetLastLength() { return m_last_length; }

	void Status(TcpSocket *p);
	/*!
	* \fn void Update()
	* \brief Used to update the session. 
	*/
	void Update();
	/*!
	* \fn piece_v& Complete()
	* \brief Returns the map of completed pieces. 
	*/
	piece_v& Complete() { return m_complete; }
	/*!
	* \fn piece_v& Incomplete()
	* \brief Returns the map incompleted pieces. 
	*/
	piece_v& Incomplete() { return m_incomplete; }
	/*!
	* \fn void AddConnect()
	* \brief Used to update the list of peers we are connected to. 
	*/
	void AddConnect();
	/*!
	* \fn bool GetRandomNotRequested(Peer *,size_t& piece,size_t& offset,size_t& length)
	* \brief Looks randomly for not requested pieces. 
	*/
	bool GetRandomNotRequested(Peer *,size_t& piece,size_t& offset,size_t& length);
	/*!
	* \fn Request *AddRequest(Peer *,size_t piece,size_t offset,size_t length)
	* \brief Adds a new request for a given piece. 
	*/
	Request *AddRequest(Peer *,size_t piece,size_t offset,size_t length);
	/*!
	* \fn void RemoveRequest(Peer *,size_t piece,size_t offset,size_t length)
	* \brief Removes a request. 
	*/
	void RemoveRequest(Peer *,size_t piece,size_t offset,size_t length);
	/*!
	* \fn void SaveSlice(size_t piece,size_t offset,size_t length,unsigned char *)
	* \brief Saves a new recieved SLICE. 
	*/
	void SaveSlice(size_t piece,size_t offset,size_t length,unsigned char *);

	/*!
	* \fn int64_t GetLength() 
	* \brief Returns the file length. 
	*/
	int64_t GetLength() { return m_length_one; }
	/*!
	* \fn file_v& GetFiles() 
	* \brief Returns the files vector we are already downloading. 
	*/
	file_v& GetFiles() { return m_files; }
	/*!
	* \fn void CreateFileManager() 
	* \brief Creates the file manager that will take in charge updating the downloaded files. 
	*/
	void CreateFileManager();
	/*!
	* \fn FileManager * GetFileManager() 
	* \brief Returns the file manager. 
	*/
	FileManager * GetFileManager(){return m_filemanager;};
	/*!
	* \fn void Verify() 
	* \brief Used to verify the integrity of the files we are downloading. 
	*/
	void Verify();
	/*!
	* \fn const std::string& GetPieces()
	* \brief Returns the number of pieces. 
	*/
	const std::string& GetPieces() { return m_pieces; }
	Piece *GetIncomplete(size_t piece);
	/*!
	* \fn bool SliceSent(size_t piece,size_t offset)
	* \brief Used to verify if it is possible to send a given slice. 
	*/
	bool SliceSent(size_t piece,size_t offset);
	/*!
	* \fn void SendSlice(pSocket *,size_t piece,size_t offset,size_t length)
	* \brief Used to send a slice. 
	*/
	void SendSlice(pSocket *,size_t piece,size_t offset,size_t length);
	Piece *GetComplete(size_t piece);

	/*!
	* \fn void save_piece_v(FILE *,piece_v&)
	* \brief Used to save a new recieved piece to the file. 
	*/
	void save_piece_v(FILE *,piece_v&);
	/*!
	* \fn void load_piece_v(FILE *,piece_v&)
	* \brief Used to load a piece from the file. 
	*/
	void load_piece_v(FILE *,piece_v&);
	/*!
	* \fn bool GenerateRequest(Peer *peer)
	* \brief Used to generate requests to a given peer. 
	*/
	bool GenerateRequest(Peer *peer);
	/*!
	* \fn bool PieceUnique(size_t piece)
	* \brief Used to verify if a piece is owned by any other peer. 
	*/
	bool PieceUnique(size_t piece);
	/*!
	* \fn void PeerStatus()
	* \brief Used to show the downloading advancement. 
	*/
	void PeerStatus();
	/*!
	* \fn void SetCheckComplete()
	* \brief Used to indicate that there is some new completed pieces that should be verified by comparing the hashs. 
	*/
	void SetCheckComplete() { m_b_check_complete = true; }
	void SetUpdateInterested() { m_b_update_interested = true; }

	bool IsDemon() { return m_demon; }
	size_t Available(int piece);
	void RequestAvailable(Peer *);
	void RemoveRequests(Peer *);
	void CheckRequests();
	/*!
	* \fn int64_t GetTotalBytesR()
	* \brief Returns the total recieved bytes. 
	*/
	int64_t GetTotalBytesR();
	/*!
	* \fn int64_t GetTotalBytesS()
	* \brief Returns the total bytes sent. 
	*/
	int64_t GetTotalBytesS();
	/*!
	* \fn bool isSeeding()
	* \brief Indicates if this session is for seeding or downloading purpose. 
	*/
	bool isSeeding(){return bSeeding;};
	/*!
	* \fn void SetFileToSeedPath(std::string s)
	* \brief Sets the path of the file we are currently seeding. 
	*/
	void SetFileToSeedPath(std::string s){this->file_to_seed_path=s;};
	/*!
	* \fn std::string GetFileToSeedPath()
	* \brief Returns the absolute path of the file that we are currently seeding. 
	*/
	std::string GetFileToSeedPath(){return file_to_seed_path;};
	/*!
	* \fn void SetLogDirectory(std::string & l)
	* \brief Sets the directory where log files will be saved. 
	*/
	void SetLogDirectory(std::string & l)
	{
		this->sLogDirectory=l;
	};
	/*!
	* \fn std::string & GetLogDirectory()
	* \brief Returns the absolute path where log files are saved. 
	*/
	std::string & GetLogDirectory()
	{
		return this->sLogDirectory;
	}
	void InCriticalSection()
	{
		EnterCriticalSection(&csStop);
	};
	void OutCriticalSection()
	{
		LeaveCriticalSection(&csStop);
	};
	bool IsAbsent(int piece);
	Peer* ChooseAPeer(std::vector<Peer *> & pv,Peer * diff,int scope);
	LogFile * session_log;
	LogFile * downloaded_files_log;
	LogFile * matrix_log;
	

	/*!
	* \fn size_t GetElapsedTime()
	* \brief Returns the elapsed time since we are downloading the file. 
	*/
	size_t GetElapsedTime()
	{
		return this->uElapsedTime; 
	};
	/*!
	* \fn void UpdateElapsedTime()
	* \brief Updates the downloading Elapsed time. 
	*/
	void UpdateElapsedTime()
	{
		if(m_incomplete.size()!=0 && m_complete.size()<m_number_of_pieces)
		{
			this->uElapsedTime+=time(NULL)-this->uElapsedTimeLastUpdate;
			this->uElapsedTimeLastUpdate=time(NULL);
		}
	};

	/*!
	* \fn string GetDownloadSpeed()
	* \brief Returns the current download speed. 
	*/
	std::string GetDownloadSpeed();
	/*!
	* \fn std::string GetLocalIpAdr()
	* \brief Returns the Local Ip Adr. 
	*/
	std::string GetLocalIpAdr();
	/*!
	* \fn void SuspendDownloading(bool deleting)
	* \brief Suspends a downloading session. 
	*/
	void SuspendDownloading(bool deleting);
	/*!
	* \fn void Synchronize()
	* \brief Synchronizes the elapsed time of a given session. 
	*/
	void Synchronize()
	{
		uElapsedTime=0;
		uElapsedTimeLastUpdate=time(NULL);
	};
	int GetNumberOfInvolvedPeers()
	{
		return this->m_peers.size();
	};

	void RetryRequest(Peer *peer,size_t piece,size_t offset,size_t length);

	std::string GetPeerNumberIp(int nbr);
	int GetPeerNumberNbrHops(int nbr);
	bool bToDelete;
	
	int iPaused;
	bool bDeleteRelatedFiles;

	// Format: U localIp remoteIP nbrHops bytes  
	void AddUBytesToTrafficMatrix(std::string ip, size_t bytes);
	// Format: D localIp remoteIP nbrHops bytes
	void AddDBytesToTrafficMatrix(std::string host, size_t bytes);
	void AddNewCharingRatioToMatrix(std::string ips,std::string ipd, double bytes);
	void CalculateTheSharingRatioMatrix();
	void WriteTheSharingRatioMatrixToLogFile();
private:

	Session(const Session& s) : m_handler(s.Handler()) {} // copy constructor
	Session& operator=(const Session& ) { return *this; } // assignment operator
	//! Random peer id.
	unsigned char m_peer_id[20];
	//! Socket Handler.
	SocketHandler& m_handler;
	//! Info Hash.
	std::string m_info_hash;
	//! Announce Url.
	std::string m_announce;
	//! Name of the file we are downloading.
	std::string m_name;
	//! Piece length.
	int64_t m_piece_length;
	//! Total file length.
	int64_t m_length; 
	//! Vector of the files we are downloading.
	file_v m_files;
	//! Number of pieces according to the file size and the piece length.
	size_t m_number_of_pieces;
	//! Pieces hash.
	std::string m_pieces;
	//! Last piece length.
	int64_t m_last_length;
	//! Tracker reconnect interval.
	int m_interval;
	//! Vector of peers involved in the downloading proccess.
	peer_v m_peers;
	//! Last time we were connected to the tracker.
	time_t m_t_tracker;
	bitmap_t *m_bitmap;
	int64_t m_prev_offset;
	int64_t m_prev_length;
	unsigned char m_hashptr[20];
	//! Vector of completed pieces.
	piece_v m_complete;
	//! Vector of incompleted pieces.
	piece_v m_incomplete;
	int64_t m_length_one;
	//! Ptr to the file manager.
	FileManager *m_filemanager;
	//! Bool value that indicates that we have to check the hash of completed pieces.
	bool m_b_check_complete;
	bool m_b_update_interested;
	bool m_demon;
	//! Bool value that indictaes if this session is for seeding or downloading process.
	bool bSeeding;
	//! Total recieved bytes.
	int64_t uiTotalR;
	//! Total sent bytes.
	int64_t uiTotalS;
	//! Path of the file to seed.
	std::string file_to_seed_path;
	//! Number of completed pieces.
	int iNbrPieceComplete;
	//! Log Directory.
	std::string sLogDirectory;
	CRITICAL_SECTION csStop;
	//! Last opti;istic peer.
	Peer* pLastChoosedPeer;
	//! Elapsed time since we started the downloading.
	size_t uElapsedTime;
	//! Last time we updated the elapsed time.
	size_t uElapsedTimeLastUpdate;
	//! 
	bool bAlreadyDownloaded;
	int64_t uLastTR;
	__int64 uLastTotalRecievedBytes;
	void UpdateSeedingStatus();

	// Maintain statistics 
	std::map<std::string, size_t> u_traffic_matrix_map;
	std::map<std::string, size_t> d_traffic_matrix_map;
	std::map<std::string, couple > sharing_ratio_matrix;
};

#endif // _SESSION_H

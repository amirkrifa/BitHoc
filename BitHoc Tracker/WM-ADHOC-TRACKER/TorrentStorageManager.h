#ifndef TORRENT_STORAGE_MANAGER_H
#define TORRENT_STORAGE_MANAGER_H

#pragma once
#define TORRENT_FILE_NAME_LENGTH 200
#define TORRENT_FILE_DESCRIPTION_LENGTH 1024
#include<map>
#include<list>
#include<string>
using namespace std;

typedef struct torrent_file_info
{
	char file_name[TORRENT_FILE_NAME_LENGTH];
	char file_description[TORRENT_FILE_DESCRIPTION_LENGTH];
	unsigned int file_size;

}torrent_file_info;

typedef list<torrent_file_info *> FilesList;
class TorrentManager;
class TorrentStorageManager
{
public:
	TorrentStorageManager(void);
	~TorrentStorageManager(void);
	void AddNewTorrentFile(string &node_adr, string & file_name, string & file_description, unsigned int fs);
	string GetFileNode(string & file_name, int &nbr, string & fileName, TorrentManager * ptr_tm, string peer_id, bool localRequest);
	void GetTheListOfPublishedTorrents(string &list, TorrentManager * ptr_tm, std::string peer_id );
	void GetTheListOfPublishedTorrentsJustForInfo(string &list);
	bool PossibleCorrectFile(string & file_name, torrent_file_info * tfi);

private:
	map<string, FilesList> files_map;
};

#endif

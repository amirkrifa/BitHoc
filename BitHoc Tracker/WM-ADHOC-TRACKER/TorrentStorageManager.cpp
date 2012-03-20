#include "StdAfx.h"
#include "TorrentStorageManager.h"
#include "TorrentManager.h"
TorrentStorageManager::TorrentStorageManager(void)
{
}

TorrentStorageManager::~TorrentStorageManager(void)
{
	if(!files_map.empty())
	{
		map<string, list<torrent_file_info *>>::iterator iter = files_map.begin();
		list<torrent_file_info*>::iterator iter2;
		while(iter != files_map.end())
		{
			while(!iter->second.empty())
			{
				iter2 = iter->second.begin();
				free(*iter2);
				iter->second.erase(iter2);
			}
			iter++;	
		}
	}
}

void TorrentStorageManager::AddNewTorrentFile(string &node_adr, string & fn, string & file_description, unsigned int fs)
{
	map<string, FilesList>::iterator iter;
	string file_name(fn);
	string ip_adr(node_adr);

	if(files_map.empty())
	{
		// The map is empty
		torrent_file_info * tmp = (torrent_file_info *) malloc(sizeof(torrent_file_info));
		memset(tmp->file_name,'\0',strlen(tmp->file_name));
		memset(tmp->file_description,'\0',strlen(tmp->file_description));

		strcpy(tmp->file_name, file_name.c_str());
		strcpy(tmp->file_description,file_description.c_str());
		tmp->file_size = fs;
		FilesList l;
		l.push_back(tmp);
		files_map.insert(make_pair<string, FilesList>(ip_adr, l)); 
		tmp = NULL;

	}else 
	{	
		// Looking for the node
		iter = files_map.find(ip_adr);
		if(iter != files_map.end())
		{
			// The node exist, looking for the file in the list
			FilesList::iterator lIter = iter->second.begin();
			while(lIter != iter->second.end())
			{
				if(strcmp((*lIter)->file_name, file_name.c_str()) == 0)
				{
					// The file is already here 

					// Update the file description
					memset((*lIter)->file_description, '\0', TORRENT_FILE_DESCRIPTION_LENGTH);
					strcpy((*lIter)->file_description, file_description.c_str());

					break;
				}
				lIter++;
			}

			if(lIter == iter->second.end())
			{
				// The file does not exist, add it
				torrent_file_info * tmp = (torrent_file_info *) malloc(sizeof(torrent_file_info));
				
				memset(tmp->file_name,'\0',strlen(tmp->file_name));
				memset(tmp->file_description,'\0',strlen(tmp->file_description));

				strcpy(tmp->file_name, file_name.c_str());
				strcpy(tmp->file_description, file_description.c_str());
				tmp->file_size = fs;
				iter->second.push_back(tmp);		
				tmp = NULL;
			}
			
		}else
		{
			// Node does not exist, we add the node and the list containing the file
			torrent_file_info * tmp = (torrent_file_info *) malloc(sizeof(torrent_file_info));
			memset(tmp->file_name,'\0',strlen(tmp->file_name));
			memset(tmp->file_description,'\0',strlen(tmp->file_description));

			strcpy(tmp->file_name, file_name.c_str());
			strcpy(tmp->file_description, file_description.c_str());
			tmp->file_size = fs;
			FilesList l;
			l.push_back(tmp);
			files_map.insert(make_pair<string, FilesList>(ip_adr, l)); 
			tmp = NULL;
		}
		
	}
}

string TorrentStorageManager::GetFileNode(string & file_name, int & n, string & fileName, TorrentManager * ptr_tm, string peer_id, bool localRequest)
{
	map<string, FilesList>::iterator iter = files_map.begin();
	FilesList::iterator iter2;
	string res("");
	char tmp[100];
	int nbr = 0;
	while(iter != files_map.end())
	{
		iter2 = iter->second.begin();
		while(iter2 != iter->second.end())
		{
			// The file is considered as found if the 
			if(PossibleCorrectFile(file_name, *iter2))
			{
				if(nbr > 0)
					res.append("\n");
				// Add the file Name
				res.append(iter->first);
				res.append("#");
				res.append((*iter2)->file_name);
				if(nbr == 0)
				{
					// Append the file Name
					fileName.assign((*iter2)->file_name);
				}
				
				// Add the file size
				memset(tmp,'\0', strlen(tmp));
				sprintf(tmp,"%u",(*iter2)->file_size);
				res.append("*");
				res.append(tmp);
				
				// Add the file description
				res.append("*");
				res.append((*iter2)->file_description);
				
				// Add the number of seeders and leechers of the related Torrent only if it is a local request
				if(localRequest)
				{
					int number_of_seeders = ptr_tm->get_number_of_ppers_with_entire_file(ptr_tm->GetTorrentInfoHash(string((*iter2)->file_name)), peer_id);
					int number_of_leechers = ptr_tm->get_number_of_non_seeder_peers(ptr_tm->GetTorrentInfoHash(string((*iter2)->file_name)), peer_id);
					char tmpTorrentDetails[200];
					sprintf(tmpTorrentDetails, "*%i*%i",number_of_leechers, number_of_seeders);
					res.append(tmpTorrentDetails);
				}

				nbr++;
			}

			iter2 ++;

		}

		iter++;
	}

	n = nbr;
	return res;
}

bool TorrentStorageManager::PossibleCorrectFile(string & file_name, torrent_file_info * tfi)
{
	// The exact file Name
	if(string(tfi->file_name).compare(file_name) == 0)
		return true;
	
	// Some part of the file Name
	if(string(tfi->file_name).find(file_name) != string::npos)
		return true;
	
	// Last chance: a key word from the description related to the torrent file
	if(string(tfi->file_description).find(file_name) != string::npos)
		return true;

	// This is not the correct file, we are looking for
	return false;
}
void TorrentStorageManager::GetTheListOfPublishedTorrents(string &list, TorrentManager * ptr_tm, string peer_id)
{
	/*
		list format :node1Ip#file1*file1.size-file2*file2.size-file3*file3.size \n
	*/
	if(files_map.empty())
		list.assign("");
	else 
	{
		char tmp[100];
		// The map is not empty
		map<string, FilesList>::iterator iter = files_map.begin();
		FilesList::iterator iter2;
		while(iter != files_map.end())
		{
			list.append(iter->first);
			list.append("#");
			iter2 = iter->second.begin();
			while(iter2 != iter->second.end())
			{
				// Add the file Name
				list.append((*iter2)->file_name);
				
				// Add the file size
				memset(tmp,'\0', 100);
				sprintf(tmp,"%u",(*iter2)->file_size);
				list.append("*");
				list.append(tmp);
				
				// Add the file description
				list.append("*");
				list.append((*iter2)->file_description);
				// We will add the number of leechers and seeders related to this torrent
				std::string info_hash = ptr_tm->GetTorrentInfoHash(string((*iter2)->file_name));
				int number_of_seeders = ptr_tm->get_number_of_ppers_with_entire_file(info_hash, peer_id);
				int number_of_leechers = ptr_tm->get_number_of_non_seeder_peers(info_hash, peer_id);
				char tmpTorrentDetails[200];
				sprintf(tmpTorrentDetails, "*%i*%i",number_of_leechers, number_of_seeders);
				list.append(tmpTorrentDetails);

				iter2 ++;
				if(iter2 != iter->second.end())
				{
					list.append("\n");
					list.append(iter->first);
					list.append("#");
				}
				else break;
			}
			iter++;
			if(iter != files_map.end())
				list.append("\n");
			else break;
		}
	}

}

void TorrentStorageManager::GetTheListOfPublishedTorrentsJustForInfo(string &list)
{
	list.append("List of local tracker'known torrents:\r\n");
	
	if(files_map.empty())
		list.assign("");
	else 
	{
		// The map is not empty
		map<string, FilesList>::iterator iter = files_map.begin();
		FilesList::iterator iter2;
		while(iter != files_map.end())
		{
			list.append("  ");
			list.append(iter->first);
			list.append(" --> ");
			iter2 = iter->second.begin();
			while(iter2 != iter->second.end())
			{
				// We can add also the size of the file if needed
				list.append((*iter2)->file_name);
				iter2 ++;
				if(iter2 != iter->second.end())
				{
					list.append(", ");
					list.append(iter->first);
					list.append(" --> ");
				}
				else break;
			}

			iter++;
			
			if(iter != files_map.end())
				list.append("\r\n");
			else break;
		}
		
	}

}
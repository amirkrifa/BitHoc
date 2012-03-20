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
* \file Socket.cpp
* \brief This file include the implemetation of the class Socket, SocketClient, SocketClientBroadcast, SocketServerBroadcast, SocketServer and SocketSelect used in different points among the rest of the project. 
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#include "stdafx.h"
#include "LogFile.h"
#include <iostream>
#include "Socket.h"
#include <ws2tcpip.h>

using namespace std;
#define MAXBUFLEN 1024
int Socket::nofSockets_= 0;

void Socket::Start() {
  if (!nofSockets_) {
    WSADATA info;
    if (WSAStartup(MAKEWORD(2,0), &info)) {
      throw "Could not start WSA";
    }
  }
  ++nofSockets_;
}

void Socket::End() {
  WSACleanup();
}

Socket::Socket() : s_(0) {
  Start();
  // UDP: use SOCK_DGRAM instead of SOCK_STREAM

  s_ = socket(AF_INET,SOCK_STREAM,0);

  if (s_ == INVALID_SOCKET) {
    throw string("INVALID_SOCKET");
  }

  refCounter_ = new int(1);
}
Socket::Socket(bool broadcast): s_(0)
{
	if(broadcast)
	{
		Start();
		// UDP: use SOCK_DGRAM instead of SOCK_STREAM
		s_ = socket(AF_INET,SOCK_DGRAM,0);
		if (s_ == INVALID_SOCKET) {
			throw string("INVALID_SOCKET");
		}
		refCounter_ = new int(1);
	}
}
Socket::Socket(SOCKET s) : s_(s) {
  Start();
  refCounter_ = new int(1);
};

Socket::~Socket() {
  if (! --(*refCounter_)) {
    Close();
    delete refCounter_;
  }


  --nofSockets_;
  if (!nofSockets_) End();
}

Socket::Socket(const Socket& o) {
  refCounter_=o.refCounter_;
  (*refCounter_)++;
  s_         =o.s_;


  nofSockets_++;
}

Socket& Socket::operator =(Socket& o) {
  (*o.refCounter_)++;


  refCounter_=o.refCounter_;
  s_         =o.s_;


  nofSockets_++;


  return *this;
}

void Socket::Close() {
  closesocket(s_);
}

std::string Socket::ReceiveBytes() {
  std::string ret;
  char buf[1024];
 
 for ( ; ; ) {
  u_long arg = 0;
  if (ioctlsocket(s_, FIONREAD, &arg) != 0)
   break;
  if (arg == 0)
   break;
  if (arg > 1024)
   arg = 1024;
  int rv = recv (s_, buf, arg, 0);
  if (rv <= 0)
   break;
  std::string t;
  t.assign (buf, rv);
  ret += t;
 }
 
  return ret;
}

std::string Socket::ReceiveLine() {
  std::string ret;
  while (1) {
     char r;

     switch(recv(s_, &r, 1, 0)) {
       case 0: // not connected anymore;
         return "";
       case -1:
           int err=WSAGetLastError();
		   int test=err;
		   return "";
         //}
     }
	if (r == '\n')  return ret;
    ret += r;
     
   }
}

void Socket::SendLine(std::string s) {
  s += '\n';
  send(s_,s.c_str(),s.length(),0);
}

void Socket::SendBytes(const std::string& s) {
  send(s_,s.c_str(),s.length(),0);
}

SocketServer::SocketServer(int port, int connections, TypeSocket type) {
  sockaddr_in sa;

  memset(&sa, 0, sizeof(sa));

  sa.sin_family = PF_INET;             
  sa.sin_port = htons(port);          
  s_ = socket(AF_INET, SOCK_STREAM, 0);
  if (s_ == INVALID_SOCKET) {
    throw "INVALID_SOCKET";
  }

  if(type==NonBlockingSocket) {
    u_long arg = 1;
    ioctlsocket(s_, FIONBIO, &arg);
  }

  /* bind the socket to the internet address */
  if (bind(s_, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
    closesocket(s_);
    throw "INVALID_SOCKET";
  }
  
  listen(s_, connections);                               
}

Socket* SocketServer::Accept() {
  SOCKET new_sock = accept(s_, 0, 0);
  if (new_sock == INVALID_SOCKET) {
          int rc = WSAGetLastError();
          if(rc==WSAEWOULDBLOCK) {
                  return 0; // non-blocking call, no request pending
          }
          else {
            throw string("Invalid Socket");
      }
  }



  Socket* r = new Socket(new_sock);
  return r;
}


SocketClient::SocketClient(const std::string& host, int port) : Socket() {

  int res;

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.S_un.S_addr=inet_addr((char*)host.c_str());
  
  res=::connect(s_, (sockaddr *) &addr, sizeof(sockaddr));
  
  if(res<0)
  {	
	  throw string("Error in Connection");
  }
  
}

SocketClientBroadcast::SocketClientBroadcast():Socket(true)
{

}
SocketServerBroadcast::SocketServerBroadcast():Socket(true)
{
	
}
  
int SocketClientBroadcast::send_broadcast_message(string msg,string host,int port)
{
	int broadcast=1;
	int res;
	int sent=0;
	socklen_t b_len=sizeof(broadcast);
	// Setting the broadcast mode
	if(setsockopt(this->s_,SOL_SOCKET,SO_BROADCAST,(const char *)&broadcast,b_len)==-1)
	{
		res=WSAGetLastError(); 
		throw string(" setsockopt SO_BROADCAST Error");
	}
	// Set the ttl of the outgoing packets to 1
	//if(setsockopt(this->s_,SOL_SOCKET,SO_IP_TTL,(const char *)&broadcast,b_len)==-1)
	//{
	//	res=WSAGetLastError(); 
	//	throw string(" setsockopt SO_BROADCAST Error");
	//}
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr=inet_addr((char*)host.c_str());
	
	char message_to_send[100];
	strcpy(message_to_send,msg.c_str());
	strcat(message_to_send,"\0");
	if((sent=sendto(s_, message_to_send,strlen(message_to_send),0,(sockaddr *) &addr, sizeof(sockaddr)))==SOCKET_ERROR)
	{
		res=WSAGetLastError();
		throw string(" sendto Error");
	}
	
	return sent;
}
void SocketServerBroadcast::bind_broadcast_socket(int port)
{
	
	struct sockaddr_in my_addr;  // my address information
	my_addr.sin_family=AF_INET; //host byte order
	my_addr.sin_port=htons(port); // short network byte order
	my_addr.sin_addr.s_addr=INADDR_ANY; // automatically fill with my ip @
	
	int res;
	if(bind(s_,(struct sockaddr*)&my_addr,sizeof(my_addr))==SOCKET_ERROR)
	{
		res=WSAGetLastError(); 
		throw string("Bind Error");

	}
}
int SocketServerBroadcast::receive_broadcast_message(char *source_of_the_message,char * received_message)
{
	struct sockaddr_in their_addr; // connector's address information
	socklen_t addr_len;
	int numbytes;
	char buf[MAXBUFLEN];

	
	addr_len=sizeof(their_addr);
	if((numbytes=recvfrom(s_,buf,MAXBUFLEN-1,0,(struct sockaddr*)&their_addr,&addr_len))==SOCKET_ERROR)
	{
		throw string("RecvFrom Error");
	}

	if(numbytes>0)
	{
		// Getting the source of the message
		strcpy(source_of_the_message,inet_ntoa(their_addr.sin_addr));
		buf[numbytes]='\0';
		strcpy(received_message,buf);
	}
	return numbytes;
}

SocketSelect::SocketSelect(Socket const * const s1, Socket const * const s2, TypeSocket type) {
  FD_ZERO(&fds_);
  FD_SET(const_cast<Socket*>(s1)->s_,&fds_);
  if(s2) {
    FD_SET(const_cast<Socket*>(s2)->s_,&fds_);
  }     


  TIMEVAL tval;
  tval.tv_sec  = 0;
  tval.tv_usec = 1;


  TIMEVAL *ptval;
  if(type==NonBlockingSocket) {
    ptval = &tval;
  }
  else { 
    ptval = 0;
  }


  if (select (0, &fds_, (fd_set*) 0, (fd_set*) 0, ptval) 
      == SOCKET_ERROR) throw "Error in select";
}

bool SocketSelect::Readable(Socket const * const s) {
  if (FD_ISSET(s->s_,&fds_)) return true;
  return false;
}

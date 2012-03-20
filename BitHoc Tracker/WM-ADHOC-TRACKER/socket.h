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
* \file socket.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#ifndef _SOCKET_H
#define _SOCKET_H

#include <WinSock2.h>

#include <string>


enum TypeSocket {BlockingSocket, NonBlockingSocket};

/*!
* \class Socket
* \brief Main Socket class. 
*/
	
class Socket {
public:

  virtual ~Socket();
  Socket(const Socket&);
  Socket& operator=(Socket&);
 
  std::string ReceiveLine();
  std::string ReceiveBytes();
	/*!
	* \fn void Close()
	* \brief Method used to close a socket. 
	*/
  void   Close();

  	/*!
	* \fn void SendLine (std::string)
	* \brief Method used to send a line. 
	*/
  void SendLine (std::string);

  	/*!
	* \fn void SendBytes(const std::string&)
	* \brief Method used to send a bytes. 
	*/
  void SendBytes(const std::string&);
  	/*!
	* \fn SOCKET get_pointer(){return s_;}
	* \brief Function used to return the SOCKET ptr. 
	*/
  SOCKET get_pointer(){return s_;};
protected:
  friend class SocketServer;
  friend class SocketSelect;
  Socket(SOCKET s);
  Socket();

  Socket(bool broadcast);
  SOCKET s_;

  int* refCounter_;

private:
  	/*!
	* \fn static void Start()
	* \brief Function used to initialize the main class parameters. 
	*/
  static void Start();
  	/*!
	* \fn static void End()
	* \brief Function used to close the the SOCKET if it is still open. 
	*/
  static void End();
  static int  nofSockets_;
};
/*!
* \class SocketClient 
* \brief Class that inherits from the class Socket and that is used to create a socket to connect to a server. 
*/
class SocketClient : public Socket {
public:
  SocketClient(const std::string& host, int port);
};

/*!
* \class SocketClientBroadcast 
* \brief Class that inherits from the class Socket and that is used to create a broadcast socket in order to send messages in broadcast. 
*/
class SocketClientBroadcast : public Socket {
public:
  SocketClientBroadcast();
  int send_broadcast_message(string msg,string host,int port);
};
/*!
* \class SocketServerBroadcast 
* \brief Class that inherits from the class Socket and that is used to create a broadcast server socket in order to recieve broadcast messages. 
*/
class SocketServerBroadcast : public Socket {
public:
  SocketServerBroadcast();
  void bind_broadcast_socket(int port); 
  int receive_broadcast_message(char *source_of_the_message,char * received_message);

};

/*!
* \class SocketServer 
* \brief Class that inherits from the class Socket and that is used to create a tcp server. 
*/
class SocketServer : public Socket {
public:
  SocketServer(int port, int connections, TypeSocket type=BlockingSocket);

  Socket* Accept();

};

class SocketSelect {
  public:
    SocketSelect(Socket const * const s1, Socket const * const s2=NULL, TypeSocket type=BlockingSocket);

    bool Readable(Socket const * const s);

  private:
    fd_set fds_;
}; 



#endif _SOCKET_H

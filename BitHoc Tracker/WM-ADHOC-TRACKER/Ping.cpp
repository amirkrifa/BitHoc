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
* \file Ping.cpp
* \brief Implementation of the class used to send, receive and interpret a ping request to a given destination.
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/
#include "StdAfx.h"
#include "Ping.h"
#include "afxwin.h"
#include <stdio.h>
#include <string>
using namespace std;

Ping::Ping(void)
{
}

Ping::~Ping(void)
{
}


char * Ping::send_a_ping(char *ip_adr)
{
 HANDLE hIcmp;
 char *SendData = "ICMP SEND DATA";
 LPVOID ReplyBuffer;
 DWORD dwRetVal;
 DWORD buflen;
 PICMP_ECHO_REPLY pIcmpEchoReply;

 hIcmp = IcmpCreateFile();

 buflen = sizeof(ICMP_ECHO_REPLY) + strlen(SendData) + 1;
 ReplyBuffer = (VOID*) malloc(buflen);
 if (ReplyBuffer == NULL) {
   return NULL;
 }
 memset(ReplyBuffer, 0, buflen);
 pIcmpEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;

 if(ip_adr==NULL)
 {
	 string error;
	 error.assign("Please Enter a Valid IP @\r\n");
	 return (char *)error.c_str();
 }else {
	 char l_ip_adr[100];
	 strcpy(l_ip_adr,ip_adr);
	 
	 dwRetVal = IcmpSendEcho(hIcmp, 
   	 inet_addr(l_ip_adr), 
     SendData, strlen(SendData), 
     NULL, ReplyBuffer, 
     buflen,
     1000);

 }
 
 if (dwRetVal != 0) {
   char temp[1024];
   char * ch_ping_result=new char[2000];
   sprintf(temp,"Received %ld messages.\r\n", dwRetVal);
   strcpy(ch_ping_result,"");
   strcat(ch_ping_result,temp);
   strcpy(temp,"");
   sprintf(temp,"RTT: %d\r\n", pIcmpEchoReply->RoundTripTime);
   strcat(ch_ping_result,temp);
   strcpy(temp,"");
   sprintf(temp,"Data Size: %d\r\n", pIcmpEchoReply->DataSize);
   strcat(ch_ping_result,temp);
   strcpy(temp,"");
   sprintf(temp,"Message: %s\r\n", pIcmpEchoReply->Data);
   strcat(ch_ping_result,temp);
   strcpy(temp,"");
  
   return ch_ping_result;
 } else {

	 char temp[1024];
	 char * ch_ping_result=new char[2000];
     sprintf(temp,"Call to IcmpSendEcho() failed.\r\n");
     strcat(ch_ping_result,temp);
     strcpy(temp,"");
	 sprintf(temp,"Error: %ld\r\n", GetLastError());
	 strcat(ch_ping_result,temp);
     strcpy(temp,"");
	 return ch_ping_result;
 }

 IcmpCloseHandle(hIcmp);

 return NULL;

}
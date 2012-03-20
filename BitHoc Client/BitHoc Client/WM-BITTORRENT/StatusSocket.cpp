

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


#include "stdafx.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <Utility.h>
#include <HttpdCookies.h>
#include <HttpdForm.h>
#include <Parse.h>
#include "PeerHandler.h"
#include "StatusSocket.h"
#include <httpsocket.h>

#define DEB(x) x


StatusSocket::StatusSocket(SocketHandler& h)
:HttpdSocket(h)
,m_sheet(0)
{
}


StatusSocket::~StatusSocket()
{
}


void StatusSocket::Exec()
{
	GetEnvironment();
	ReadCookies();
	FormInput();
	CreateHeader();
	GenerateDocument();
}


void StatusSocket::GetEnvironment()
{
}


void StatusSocket::ReadCookies()
{
	HttpdCookies *cs = GetCookies();
	if (!cs)
		return;
	std::string slask;
	if (cs -> getvalue("bleep", slask))
	{
		Parse pa(slask,":");
		m_sheet = pa.getvalue();
	}
}


void StatusSocket::FormInput()
{
	HttpdForm *form = (HttpdForm *)GetForm();
	if (!form)
	{
		return;
	}

	m_tmp = form -> getvalue("ah");

	if (GetUri() == "/stop")
	{
		static_cast<PeerHandler&>(Handler()).SetQuit();
	}
	if (GetUri() == "/debug")
	{
		int val = 0;
		std::string key;
		std::string value;
		form -> getfirst(key, value);
		while (key.size())
		{

			if (!strcmp(key.c_str(), "debug"))
				val |= atoi(value.c_str());
			//
			form -> getnext(key, value);
		}
		static_cast<PeerHandler&>(Handler()).SetDebug(val);
	}
	std::string tmp = form -> getvalue("sheet");
	if (tmp.size())
		m_sheet = atoi(tmp.c_str());
}


void StatusSocket::CreateHeader()
{
	// header
	AddResponseHeader("Date", GetHttpDate());
	AddResponseHeader("Server", "++ 0.001");
	AddResponseHeader("Connection", "close");
	AddResponseHeader("Content-type", "text/html");

	// status
	SetStatus("200");
	SetStatusText("OK");

	// cookie
	SetTheCookie();

	// send
	SendResponse();
}


void StatusSocket::GenerateDocument()
{
	// page
	Send("<html>"
		"<head>"
		"<style type='text/css'>"
		"td.h {"
		"background: #e0e0e0;"
		"}"
		"</style>"
		"</head>"
		"<body>"); //<h1>status</h1>");
	Send("<div style='border: 2px #000000 solid; background: #e0e0e0; margin: 5px;'>");
	menyval(1, m_sheet, "server commands");
	menyval(2, m_sheet, "debug options");
	menyval(3, m_sheet, "sessions");
	Send("</div>");
	Send("<br>");

	switch (m_sheet)
	{
	case 1:
		Send("<a href='/stop'>stop server</a><br>");
		Send("AH: " + m_tmp + "<br>");
		break;
	case 2:
		Send("<h3>Debug</h3>");
		Send("<form action=/debug method=POST>");
		{
		int debug = static_cast<PeerHandler&>(Handler()).GetDebug();
		char slask[200];
		sprintf(slask, "<input type=checkbox name=debug value=1%s>     * 0 - choke<br>", (debug & 1) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=2%s>     * 1 - unchoke<br>", (debug & 2) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=4%s>     * 2 - interested<br>", (debug & 4) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=8%s>     * 3 - not interested<br>", (debug & 8) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=16%s>     * 4 - have [piece(integer)]<br>", (debug & 16) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=32%s>     * 5 - bitfield [bitmap]<br>", (debug & 32) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=64%s>     * 6 - request [index begin length]<br>", (debug & 64) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=128%s>     * 7 - piece [index begin piece(byte[])]<br>", (debug & 128) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=256%s>     * 8 - cancel [index begin length]<br>", (debug & 256) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=512%s> Time<br>", (debug & 512) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=1024%s> Connect<br>", (debug & 1024) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=2048%s> SocketLog<br>", (debug & 2048) ? " checked" : "");
		Send(slask);
		sprintf(slask, "<input type=checkbox name=debug value=4096%s> Speeds<br>", (debug & 4096) ? " checked" : "");
		Send(slask);
		Send("<input type=submit name=submit value=' Update '>");
		}
		Send("</form>");
		break;
	case 3:
		static_cast<PeerHandler&>(Handler()).Show( );
		break;
	} // switch (m_sheet)
	

	//
	Send("</body></html>");

	// close
	SetCloseAndDelete();
}


void StatusSocket::SetTheCookie()
{
	char slask[1000];
	sprintf(slask, "%d", m_sheet);
	HttpdCookies *cs = GetCookies();
	if (!cs)
		return;
	cs -> setcookie(this, COOKIE_DOMAIN, COOKIE_PATH, "bleep", slask);
}


void StatusSocket::menyval(int val,int sheet,const std::string& text)
{
	char slask[1000];
	Send("&nbsp;&nbsp;&nbsp;");
	if (val == sheet)
	{
		sprintf(slask,"<a href='/?sheet=%d'>%s</a>", val, text.c_str());
	}
	else
	{
		sprintf(slask,"<a style='text-decoration: none;' href='/?sheet=%d'>%s</a>", val, text.c_str());
	}
	Send(slask);
	Send("&nbsp;&nbsp;&nbsp;");
}



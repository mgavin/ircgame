/*
Copyright Notice:
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <algorithm>
#include "ircbot.h"
#include "ircmessage..h"

#undef max
#define print(a, end) cout << #a << ":\t" << a << end

typedef unsigned short ushort;

using namespace std;
using namespace IrcMessage;

IrcBot::~IrcBot()
{
}

IrcBot::IrcBot(const string& iniFile)
:m_IniFile(iniFile), m_PingsSurvived(0), m_isAlive(false), m_Pinged(false), m_Connected(false)
{	
	m_Server = "default";
	m_Port = 6667;
	m_Nick = "default";

	ifstream config_file;
	config_file.open(m_IniFile);
	string temp;

	if (!config_file)
	{
		cout << "config file doesn't exist, using defaults" << endl;
	}
	else
	{
		while (config_file.good())
		{
			if(config_file.peek() == '#')
			{
				config_file.ignore(numeric_limits<int>::max(), '\n');
				continue;
			}
			config_file >> temp;
			if (temp == "[NICK]")
				config_file >> m_Nick;
			if (temp == "[SERVER]")
				config_file >> m_Server;
			if (temp == "[PORT]")
				config_file >> m_Port;
			if (temp == "[MODE]")
				config_file >> m_Mode;
			if (temp == "[REGISTER]")
			{
				getline(config_file, m_Register, '\n');
				IrcMessage::TrimWhitespace(m_Register);
			}
			if (temp == "[CHANNEL]")
			{
				getline(config_file, temp, '\n');
				string::size_type i = temp.find_first_not_of(' ');
				string::size_type j;
				do
				{
					j = temp.find_first_of(';', i);
					if (j != string::npos)
						IrcBot::AddChannel(temp.substr(i, (j+1)-i));
					else
					{
						j = temp.length();
						IrcBot::AddChannel(temp.substr(i, (j+1)-i));
					}
					i = j+1;
				}while (j != temp.length());
			
			}
		}
	}

	config_file.close();
}

void IrcBot::Connect()
{
	string user = "USER " + m_Nick + " 0 * :" + m_Nick;
	string nick = "NICK " + m_Nick;
	try
	{
		if (m_Server.find(".") == string::npos)
			throw xNoServer();

		//startup Winsock
		WSAStartup(MAKEWORD(2,2), &m_Data);

		//create the socket
		m_DataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//find the server ip
		m_ServerIP = gethostbyname(m_Server.c_str());

		//fill in the socket address
		m_Addr.sin_family = AF_INET;
		m_Addr.sin_port = htons(m_Port);
		m_Addr.sin_addr.s_addr = *((unsigned long*)m_ServerIP->h_addr_list[0]);
		memset(&m_Addr.sin_zero, 0, 8);

		//connect to the server
		connect(m_DataSocket, (sockaddr*)&m_Addr, sizeof(sockaddr_in));
	}
	catch(...)
	{
		cout << "Unable to open connection. WSAGetLastError(): " << WSAGetLastError() << endl;
	}
	
	//send the credentials
	Sleep(1000);
	try
	{
		IrcBot::Send(user);
		IrcBot::Send(nick);
		while (!IrcBot::IsConnected())
		{
			IrcBot::Read(); 
		}
	}
	catch (xCantSend&)
	{
		cout << "can't send to the server" << endl;
		throw;
	}
	catch (xCantRead&)
	{
		cout << "can't read from the server" << endl;
		throw;
	}
	IrcBot::JoinChannels();
}

void IrcBot::Connect(const string& server, ushort port)
{
	m_Server = server;
	m_Port = port;
	IrcBot::Connect();
}

void IrcBot::Connect(const string& server, const string& nick, ushort port)
{
	m_Server = server;
	m_Nick = nick;
	m_Port = port;
	IrcBot::Connect();
}

void IrcBot::Disconnect()
{
	shutdown(m_DataSocket, 2);
	closesocket(m_DataSocket);
	WSACleanup();
}

void IrcBot::Send(string& rawMessage)
{
	if (rawMessage.length() > 512)
	{	//message exceeds acceptable length;
		rawMessage.erase(512);
		rawMessage[511] = '\0';
	}

	if (rawMessage.at(rawMessage.length()-1) != '\n')
			rawMessage.append("\n");

	if (send(m_DataSocket, rawMessage.c_str(), rawMessage.length(), 0) == SOCKET_ERROR)
		throw xCantSend();
}

void IrcBot::Send(char* msg)
{
	string send_me(msg);
	IrcBot::Send(send_me);
}

void IrcBot::Send(MSG_TYPE typeMsg, const string& message)
{
	string send = "";
	switch (typeMsg)
	{
		case MSG_PRIVMSG:
			send = "PRIVMSG ";
			break;
		case MSG_QUERY:
			send = "QUERY ";
			break;
		case MSG_NOTICE:
			send = "NOTICE ";
			break;
		default:
			send = "PRIVMSG ";
			break;
	}
	send += message;
	
	IrcBot::Send(send);
}

void IrcBot::Send(MSG_TYPE typeMsg, const string& target, const string& message)
{
	if (target == "all")
	{
		for (vps::iterator it = m_Channels.begin();
			 it != m_Channels.end();
			 it++)
		{
		string send = it->first + " :" + message;
		IrcBot::Send(typeMsg, send);
		}
	}
	else
	{
		string send = target + " :" + message;
		IrcBot::Send(typeMsg, send);
	}
}

//Read takes some commands from the server,
//by itself, ie. PING
string IrcBot::Read()
{
	char				szRecvBuff[512];	//max for IRC according to RFC2812 is 512(w/CRLF) characters
	string				recv_holder;		//easy formatting
	string::size_type	startln = 0;
	string::size_type	endln = 0;
	
	//take the text from the server
	if (recv(m_DataSocket, szRecvBuff, 512, 0) == SOCKET_ERROR)
		throw xCantRead();
		
	recv_holder = szRecvBuff;
	
	//delete all the extra characters passed the last new line
	endln = recv_holder.find_last_of('\n');
	recv_holder.erase(endln+1);
	endln = 0;
	while (endln != string::npos)
	{
		endln = recv_holder.find('\n', startln);
		IrcMessage::IrcMsg msg(recv_holder.substr(startln, endln - startln));
		if (msg.GetTypeMsg() == MSG_PING)
		{
			m_Pinged = true;
			m_PingsSurvived++;
			m_Pong = msg.GetMessage();
			Pong();
		}

		if (msg.GetSrvrTypeMsg() == 4)
			m_Connected = true;

		if (msg.GetMessage() != "")
			m_Messages.push_back(msg);

		startln = endln+1;
	}

	return recv_holder;
}

void IrcBot::Read(string& in)
{
	//so you can pass in a string&, IrcBot::Read(my_string);
	in = IrcBot::Read();
}

void IrcBot::Pong() try
{
	if (m_Pinged)
	{
		string pong = "PONG : " + m_Pong;
		IrcBot::Send(pong);
		m_isAlive = true;
		m_Pinged = false;
	}
} catch (xCantSend)
{
	m_isAlive = false;
	m_Connected = false;
}

void IrcBot::AddChannel(char* chanName)
{
	string chan(chanName);
	IrcBot::AddChannel(chan);
}

void IrcBot::RemoveChannel(char* chanName)
{
	string chan(chanName);
	IrcBot::RemoveChannel(chan);
}

void IrcBot::AddChannel(const string& chan, const string& message = "")
{
	pair<string, string> new_entry;
	new_entry.first = chan;
	new_entry.second = message;

	m_Channels.push_back(new_entry);
}

void IrcBot::AddChannel(const string& chanStr)
{
	//format #mychanneltojoin:message;
	pair<string, string> new_entry;
	string::size_type x = chanStr.find(':', 0);
	string::size_type y = 0;
	if (x != string::npos)
	{
		new_entry.first = chanStr.substr(y, x-y);
		y = x;
		x = chanStr.find_last_not_of(';');
		new_entry.second = chanStr.substr(y+1, x-y);
	}
	else
	{
		new_entry.second = "";
		x = chanStr.find_last_not_of(';');
		new_entry.first = chanStr.substr(y, x+1-y);
	}
	m_Channels.push_back(new_entry);
}

void IrcBot::RemoveChannel(const string& chanStr)
{
	for (vps::iterator it = m_Channels.begin(); it!= m_Channels.end(); it++)
		if (it->first == chanStr)
			m_Channels.erase(it);
}

void IrcBot::JoinChannel(const string& channel, const string& message)
{
	IrcBot::Send("JOIN " + channel);
	if (message != "")
		IrcBot::Send(MSG_PRIVMSG, channel, message);
	IrcBot::AddChannel(channel, message);
}

void IrcBot::JoinChannels()
{
	for (vps::iterator it = m_Channels.begin(); it!= m_Channels.end(); it++)
	{
		IrcBot::Send("JOIN " + it->first);
		if (it->second != "")
			IrcBot::Send(MSG_PRIVMSG, it->first, it->second);
		Sleep(300); //don't overload joins?
	}
}

void IrcBot::LeaveChannel(const string& channel)
{
	IrcBot::Send("PART " + channel);
}

void IrcBot::LeaveChannels()
{
	for (vps::iterator it = m_Channels.begin(); it!= m_Channels.end(); it++)
	{
		IrcBot::Send("PART " + it->first);
		Sleep(300);
	}
}

ostream& operator<<(ostream& otpt, IrcBot& obj)
{
	//IrcMessage::IrcMsg msg = obj.m_Messages.front();
	for (list<IrcMsg>::iterator it = obj.m_Messages.begin();
		 it != obj.m_Messages.end();
		 it++)
			otpt << endl << it->GetSender() << "@" << it->GetTarget() << ": " << it->GetMessage();

	obj.m_Messages.clear();

	return otpt;
}

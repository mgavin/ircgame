/*
Copyright Notice:
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

//includes
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <windows.h>
#include <winsock.h>
#include "C:\Documents and Settings\Owner\My Documents\Visual Studio 2008\Projects\ircgame\ircgame\IrcMsg.h"

using std::ostream;
using std::string;
using std::vector;
using std::pair;
using std::list;
using IrcMessage::MSG_TYPE;
using IrcMessage::IrcMsg;

typedef unsigned short ushort;
typedef vector<pair<string,string>> vps;


class IrcBot
{
protected:
	WSADATA				m_Data;
	SOCKET				m_DataSocket;
	struct hostent*		m_ServerIP;
	struct sockaddr_in	m_Addr;
	vps					m_Channels;
	string				m_Nick;
	string				m_Server;
	ushort				m_Port;
	string				m_Mode;
	string				m_Register;
	string				m_IniFile;
	string				m_Pong;
	bool				m_isAlive;
	bool				m_Pinged;
	bool				m_Connected;
	unsigned int		m_PingsSurvived;
	list<IrcMsg>		m_Messages;
	
	void Pong(); //private pong message handling function
	
public:
	//constructor/destructor
	IrcBot(const string& iniFile = "");
	IrcBot(const string& server, const string& nick, ushort port = 6667):
										m_Server(server),m_Nick(nick),m_Port(port),m_PingsSurvived(0), m_isAlive(false), m_Pinged(false), m_Connected(false){}
	//IrcBot(const IrcBot&);
	virtual ~IrcBot();
	
	virtual	IrcBot* Clone() { return new IrcBot(*this); }
	
	void	Connect();
	void	Connect(const string& server, ushort port = 6667);
	void	Connect(const string& server, const string& nick, ushort port = 6667);
	
	void	Disconnect();
	void	Die() { m_isAlive = false; m_Connected = false; IrcBot::Disconnect(); }
	
	void	Send(string& rawMessage);
	void	Send(char* string);
	void	Send(MSG_TYPE typeMsg, const string& message);
	void	Send(MSG_TYPE typeMsg, const string& target, const string& message);
	
	string  Read();
	void	Read(string&);
	
	friend ostream& operator<< (ostream& otpt, IrcBot& obj); //couts messages read
	
	bool	IsAlive() { return m_isAlive; }
	bool	IsConnected() { return m_Connected; }
	void	AddChannel(char* chanName);
	void	AddChannel(const string& chanStr);
	void	AddChannel(const string& chan, const string& message = "");
	void	RemoveChannel(char* chanName);
	void	RemoveChannel(const string& chanStr);
	void	JoinChannels();
	void	LeaveChannels();
	void	JoinChannel(const string& channel, const string& message);
	void	LeaveChannel(const string& channel);
	
	//accessors
	unsigned int GetPings() const { return m_PingsSurvived; }
	string		 GetNick() const { return m_Nick; }
	bool		 GetPinged() const { return m_Pinged; }
	string		 GetPong() const { return m_Pong; }
	string		 GetIniFile() const { return m_IniFile; }
	string		 GetServer() const { return m_Server; }
	string		 GetMode() const { return m_Mode; }
	ushort		 GetPort() const { return m_Port; }
	string		 GetRegister() const { return m_Register; }
	IrcMsg		 GetFrontMsg() const { return m_Messages.front(); }
	vps			 GetChannels() { return m_Channels; }

	void		 PopFrontMsg() { 	m_Messages.pop_front(); }
	void		 SetIniFile(string newConf) { m_IniFile = newConf; }
	void		 SetServer(string newServ) { m_Server = newServ; }
	void		 SetMode(string	newMode) { m_Mode = newMode; }
	void		 SetPort(ushort newPort) { m_Port = newPort; }
	void		 SetRegister(string newReg) { m_Register = newReg; }
	
	//Exceptions, may not be the best usage :) but I tried.!
	class xNoServer{};
	class xCantSend{};
	class xCantRead{};
};
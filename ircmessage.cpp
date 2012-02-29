/*
Copyright Notice:
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include "IrcMsg.h"

using namespace std;

//using std::string;

IrcMessage::IrcMsg::~IrcMsg()
{}

string IrcMessage::TrimWhitespace(string& in)
{
	string ret = "";
	string::size_type start = 0;
	string::size_type end = 0;

	//trim beginning
	end = in.find_first_not_of(WHITESPACE);

	if (end != string::npos)
		in.erase(start, end-start);
	else //what? no non whitespace?
		return ""; //in = "";

	//trim end
	end = in.length();
	start = in.find_last_not_of(WHITESPACE);
	if (start != string::npos)
		in.erase(start+1, end-start);

	ret = in;
	return ret;
}



void IrcMessage::IrcMsg::Parse(string& msg)
{
	//parse the message here
	string::size_type end = 0;
	stringstream str(msg);

	if (msg.at(0) == ':')
	{
		str.ignore(1);
		end = msg.find_first_of('!');
		//it's a message sent from the server
		if (end > 60 || end == string::npos)
		{
			//server sent message
			str >> m_Msg.sender;
			
			string a;
			str >> a;

			if (a == "NOTICE")
				m_Msg.srvrTypeMsg = MSG_NOTICE;
			else if (a == "PRIVMSG")
				m_Msg.srvrTypeMsg = MSG_PRIVMSG;
			else if (a == "QUERY")
				m_Msg.srvrTypeMsg = MSG_QUERY;
			else
			{ stringstream b(a); b >> m_Msg.srvrTypeMsg; }
			str >> m_Msg.target;
			getline(str, m_Msg.message, '\n');
		}
		else
		{
			getline(str, m_Msg.sender, '!');
			//str.ignore(1);
			str >> m_Msg.hostmask;

			string a;
			str >> a;
			if (a == "NOTICE")
				m_Msg.typeMsg = MSG_NOTICE;
			else if (a == "PRIVMSG")
				m_Msg.typeMsg = MSG_PRIVMSG;
			else if (a == "QUERY")
				m_Msg.typeMsg = MSG_QUERY;
			str >> m_Msg.target;
			str.ignore(2, ':');
			getline(str, m_Msg.message, '\n');
		}

	}
	else if (msg.substr(0, 6) == "PING :")
	{
		str.ignore(6);
		//it's a ping message
		m_Msg.typeMsg = MSG_PING;
		str >> m_Msg.message;
	}
	else if (msg.substr(0, 6) == "NOTICE")
	{
		m_Msg.message = msg;
	}
	else
	{
		m_Msg.message = msg;
	}
}

string IrcMessage::ParseWord(const string& str, int slot)
{
	string::size_type start = 0;
	string::size_type end = 0;

	start = str.find_first_not_of(WHITESPACE);

	for (; slot > 0; slot--)
	{
		start = str.find_first_of(WHITESPACE , start);
		start = str.find_first_not_of(WHITESPACE , start);
	}

	end = str.find_first_of(WHITESPACE, start);

	if (start == string::npos)
	{
		start = 0;
		end = 0;
	}


	return str.substr(start, end-start);
}

string IrcMessage::ToUpper(string& str)
{
	//string& x = str;
	for (string::size_type i = 0; i < str.length(); i++)
		str[i] = std::toupper(str[i]);

	return str;
}

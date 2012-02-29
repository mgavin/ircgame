/*
Copyright Notice:
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

//#include <string>
//#include <iostream>
//#include <windows.h>

using std::string;

#undef GetMessage

namespace IrcMessage
{
	
	enum  MSG_TYPE { MSG_SERVER,
                    MSG_PRIVMSG ,
                    MSG_QUERY ,
                    MSG_NOTICE,
                    MSG_PING };
	const string	WHITESPACE = " \t\n\r\f\v";

	typedef struct _IrcMsgStruct
	{
       string				   sender;
       string				   hostmask;
       MSG_TYPE			   typeMsg;
       string				   target;
       string				   message;
       unsigned short		srvrTypeMsg;
	} IrcMsgs;

	string	TrimWhitespace(string& in);
	string	ParseWord(const string& str, int slot);
	string	ToUpper(string& str);

	class IrcMsg
	{
	//the purpose of this class is to parse a string recieved from IRC
	//usage: IrcMsg(raw irc msg)
	//then you can get common info about that line
	//user name, @server, typeMsg, target, :msg
	public:
		IrcMsg(const string& in):m_rawMsg(in){ memset(&m_Msg, 0, sizeof(IrcMsgs)); if (m_rawMsg != "") IrcMsg::Parse(m_rawMsg); }
		IrcMsg(){}
		~IrcMsg();

		void		Parse(string& msg);

		//accessors
		void		SetRawMsg(const string& msg) { m_rawMsg = msg; IrcMsg::Parse(m_rawMsg); }
		string		GetRawMsg() const { return m_rawMsg; }
		IrcMsgs		GetMsg() { return m_Msg; }
		string		GetSender() const { return m_Msg.sender; }
		string		GetHostmask() const { return m_Msg.hostmask; }
		MSG_TYPE	GetTypeMsg() const { return m_Msg.typeMsg; }
		string		GetTarget() const { return m_Msg.target; }
		string		GetMessage() const { return m_Msg.message; }
		short		GetSrvrTypeMsg() const { return m_Msg.srvrTypeMsg; }

	protected:
		string	m_rawMsg;
		IrcMsgs	m_Msg;
	};

}

#include "stdafx.h"
#include "WASocketConnection.h"

HANDLE g_hNetlibUser = NULL;

void WASocketConnection::initNetwork(HANDLE hNetlibUser) throw (WAException)
{
	g_hNetlibUser = hNetlibUser;
}

void WASocketConnection::quitNetwork()
{
}

WASocketConnection::WASocketConnection(const std::string &dir, int port) throw (WAException)
{
	NETLIBOPENCONNECTION	noc = { sizeof(noc) };
	noc.szHost = dir.c_str();
	noc.wPort = port;
	noc.flags = NLOCF_V2; // | NLOCF_SSL;
	this->hConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, WPARAM(g_hNetlibUser), LPARAM(&noc));
	if (this->hConn == NULL)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_OPEN);

	this->connected = true;
}

void WASocketConnection::write(int i)
{
	char buffer;
	buffer = (char)i;

	NETLIBBUFFER nlb;
	nlb.buf = &buffer;
	nlb.len = 1;
	nlb.flags = MSG_NOHTTPGATEWAYWRAP | MSG_NODUMP;

	int result = CallService(MS_NETLIB_SEND, WPARAM(this->hConn), LPARAM(&nlb));
	if (result < 1)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_SEND);
}

void WASocketConnection::makeNonBlock()
{
	throw WAException("Error setting socket nonblocking!", WAException::SOCKET_EX, WAException::SOCKET_EX_OPEN);
}

void WASocketConnection::flush() {}

void WASocketConnection::write(const std::vector<unsigned char> &bytes, int length)
{
	NETLIBBUFFER nlb;
	std::string tmpBuf = std::string(bytes.begin(), bytes.end());
	nlb.buf = (char*)&(tmpBuf.c_str()[0]);
	nlb.len = length;
	nlb.flags = MSG_NODUMP;

	int result = CallService(MS_NETLIB_SEND, WPARAM(hConn), LPARAM(&nlb));
	if (result < length)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_SEND);
}

unsigned char WASocketConnection::read()
{
	SetLastError(0);

	char c;
	int result = Netlib_Recv(this->hConn, &c, 1, 0);
	if (result <= 0)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	return c;
}

int WASocketConnection::read(unsigned char *buf, int length)
{
	int result = Netlib_Recv(this->hConn, (char*)buf, length, MSG_NODUMP);
	if (result <= 0)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	return result;
}

int WASocketConnection::read(std::vector<unsigned char>& b, int off, int length)
{
	if (off < 0 || length < 0)
		throw new WAException("Out of bounds", WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	char* buffer = (char*)_alloca(length);
	int result = Netlib_Recv(this->hConn, buffer, length, MSG_NOHTTPGATEWAYWRAP | MSG_NODUMP);
	if (result <= 0)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	for (int i = 0; i < result; i++)
		b[off + i] = buffer[i];

	return result;
}

void WASocketConnection::forceShutdown()
{
	Netlib_Shutdown(this->hConn);
}

void WASocketConnection::log(const char *prefix, const char *str)
{
	Netlib_Logf(g_hNetlibUser, "%s%s", prefix, str);
}

WASocketConnection::~WASocketConnection()
{
	this->forceShutdown();
	Netlib_CloseHandle(this->hConn);
}

#include <sbl/system/Socket.h>
#include <sbl/core/Display.h>
#include <sbl/core/Command.h>
#ifdef WIN32
	#include <WinSock2.h>
	#pragma comment( lib, "Ws2_32.lib" )
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	#include <stdio.h>
	#include <errno.h>
#endif
namespace sbl {


//-------------------------------------------
// SHARED BETWEEN SOCKETS
//-------------------------------------------



// terminate socket library
void socketSystemCleanUp() {
#ifdef WIN32
	WSACleanup();
#endif
}


// initialize socket library
void socketSystemStart() {
#ifdef WIN32
	static bool s_socketSystemStarted = false;
	if (s_socketSystemStarted == false) {
		WSADATA info;
		if (WSAStartup( MAKEWORD( 2, 0 ), &info )) 
			warning( "socketSystemStart failed" );
		s_socketSystemStarted = true;
		registerCleanUp( socketSystemCleanUp );
	}
#endif
}


// get last socket error message
const char *socketErrorText() {
#ifdef WIN32
	switch (WSAGetLastError()) {
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	};
#endif
	return "OTHER";
}


//-------------------------------------------
// SOCKET CLASS
//-------------------------------------------


/// create unconnected socket
Socket::Socket() {
	m_sock = 0;
}


/// disconnect if connected
Socket::~Socket() {
#ifdef WIN32
	if (m_sock) {
		closesocket(m_sock);
	}
#else
	if (m_sock) {
		close(m_sock);
	}
#endif
}


/// connect to the specified host and port (used for client)
bool Socket::connect(const char *hostName, int port) {
	disp(1, "connecting to %s / %d", hostName, port);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (hostName[0] == 0) {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		addr.sin_addr.s_addr = inet_addr(hostName); // fix(later): assuming IP address not host name
	}
	memset(&(addr.sin_zero), 0, 8);

	// create/init/allocate the socket file descriptor
	if (createSocket() == false) {
		return false;
	}

	// connect
	if (::connect(m_sock, (sockaddr *) &addr, sizeof(sockaddr))) {
		warning( "Socket::connect connection error: %s", socketErrorText() );
		return false;
	}
	return true;
}


/// listen for connections at the specified host and port (used for server)
bool Socket::listen(const char *hostName, int port) {
	disp(1, "connecting to %s / %d", hostName, port);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (hostName[0] == 0) {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		addr.sin_addr.s_addr = inet_addr(hostName); // fix(later): assuming IP address not host name
	}
	memset(&(addr.sin_zero), 0, 8);

	// create/init/allocate the socket file descriptor
	if (createSocket() == false) {
		return false;
	}

	// bind and listen for connections
	if (bind(m_sock, (sockaddr *) &addr, sizeof(sockaddr))) {
		warning( "Socket::listen bind error: %s", socketErrorText());
		return false;
	}
	if (::listen(m_sock, 10)) {
		warning( "Socket::listen listen error: %s", socketErrorText());
		return false;
	}
	return true;
}


/// write a text line (adds newline)
void Socket::write( String &s ) {
	send( m_sock, s.c_str(), s.length(), 0 );
	send( m_sock, "\n", 1, 0 );
}


/// write a text line (adds newline)
void Socket::write( const char *cstr ) {
	send( m_sock, cstr, (int) strlen( cstr ), 0 );
	send( m_sock, "\n", 1, 0 );
}


/// read a text line (removes newline); blocking
String Socket::read() {
	const int maxLen = 1000;
	char buf[ maxLen ];
	buf[ 0 ] = 0;
	int pos = 0;

	// read one character at a time
	while (1) {
		char c = 0;
		int ret = recv( m_sock, &c, 1, 0 );
		if (ret == 0) // disconnected?
			break;
		if (ret == -1) // ?
			break;
		if (c == 0)
			break;
		if (c == '\n')
			break;
		buf[ pos++ ] = c;
		if (pos + 1 == maxLen)
			break;
	}
	buf[ pos ] = 0;
	return String( buf );
}


bool Socket::createSocket() {
	socketSystemStart();

	// create the socket
	m_sock = (int) socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef WIN32
	if (m_sock == INVALID_SOCKET) {
		warning("Socket::connect creation error: %s", socketErrorText());
		return false;
	}
#endif
	return true;
}


} // end namespace sbl

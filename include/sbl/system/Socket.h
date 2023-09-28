#ifndef _SBL_SOCKET_H_
#define _SBL_SOCKET_H_
#include <sbl/core/String.h>
namespace sbl {


/// The Socket class provides a simple platform-independent wrapper for network sockets.
class Socket {
public:

	/// create unconnected socket
	Socket();

	/// disconnect if connected
	~Socket();

	/// connect to the specified host and port (used for client)
	bool connect(const char *hostName, int port);

	/// listen for connections at the specified host and port (used for server)
	bool listen(const char *hostName, int port);

	/// write a text line (adds newline)
	void write(String &s);
	void write(const char *cstr);

	/// read a text line (removes newline); blocking
	String read();

	/// direct access to socket file descriptor
	int fd() { return m_sock; }

private:

	bool createSocket();

	// operating system socket handle
	int m_sock;

	// disable copy constructor and assignment operator
	Socket(const Socket &x);
	Socket &operator=(const Socket &x);
};


} // end namespace sbl
#endif // _SBL_SOCKET_H_

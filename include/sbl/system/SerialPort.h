#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_
#include <sbl/core/StringUtil.h>
namespace sbl {


class SerialHandler {
public:
	virtual bool handleSerialMessage(const String &message) = 0;
};


// The SerialPort class provides a high-level interface to a serial port
class SerialPort {
public:

	// open/close serial port
	SerialPort(const String &portName, int baud, int bufferLength=200);
	~SerialPort();

	// set the verbosity level
	inline void verbose(bool v) { m_verbose = v; }

	// reads until receives untilChar; if hasn't yet received untilChar, returns empty string; 
	// should call at least as often as will receive untilChar (this will only return one string (up to untilChar per call);
	// untilChar itself won't be included
	String readString(char untilChar);

	// send string
	void writeString(const String &s);

	// send a command immediately; add a checksum
	void writeCommand(const String &command);

	// ======== high-level message handling with queuing and acks functions ========

	// set an object to be notified on incoming serial messages
	void setHandler(SerialHandler *handler) { m_handler = handler; }

	// sends a command or adds to outgoing queue
	void sendCommand(const String &command, bool waitForAck);

	// process any incoming serial messages (may handle multiple messages in a single call)
	void checkForMessages();

	// returns the number of checksum errors encountered since startup
	inline int checksumErrorCount() const { return m_checksumErrorCount; }

	// returns number of queued outgoing messages (waiting for acks)
	int outgoingMessageCount() const { return m_outgoingMessages.count(); }

private:

	int m_port;  // file descriptor
	int m_pos;  // position with the buffer
	char *m_buf;  // a buffer of incoming data
	int m_bufLen;
    bool m_checkForData;
	bool m_verbose;
	int m_checksumErrorCount;
	double m_lastCommandTime;
	SerialHandler *m_handler;
	Array<String> m_outgoingMessages;  // first element is always the most recently sent command
};


// an implementation of the CRC16-CCITT algorithm
unsigned short crc16(const char *message);


} // end namespace sbl
#endif // _SERIAL_PORT_H_

#include <sbl/system/SerialPort.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
	#include <fcntl.h>
	#include <errno.h>
	#include <termios.h>
	#include <unistd.h>
    #include <sys/file.h>
#endif
#include <sbl/core/Command.h>
#include <sbl/core/StringUtil.h>
#include <sbl/system/Timer.h>
namespace sbl {


// some code based on this code: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/


// constructor opens serial port
SerialPort::SerialPort(const String &portName, int baud, int bufferLength) {
	m_buf = NULL;
    m_checkForData = false;
	m_verbose = true;
	m_checksumErrorCount = 0;
	m_lastCommandTime = 0;

#ifndef WIN32
    int baudDef = -1;
    switch (baud) {
    case 9600: baudDef = B9600; break;
    case 38400: baudDef = B38400; break;
    default:
        disp(1, "baud not supported: %d", baud);
        m_port = -1;
        return;
    }

	// open the serial port file
	m_port = open(portName.c_str(), O_RDWR);
	if (m_port < 0) {
		disp(1, "error %i from open: %s", errno, strerror(errno));
		return;
	}

	// lock the serial port file
	if(flock(m_port, LOCK_EX | LOCK_NB) == -1) {
		disp(1, "serial port %s already in use", portName.c_str());
		m_port = -1;
		return;
	}

	// prepare buffer
	m_buf = new char[bufferLength];
	m_bufLen = bufferLength;
	m_pos = 0;

	// update settings
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if(tcgetattr(m_port, &tty) != 0) {
		disp(1, "error %i from tcgetattr: %s", errno, strerror(errno));
	}
	tty.c_cflag &= ~PARENB;  // no parity bit
	tty.c_cflag &= ~CSTOPB;  // one stop bit
	tty.c_cflag |= CS8;  // 8 bits per byte
	tty.c_cflag &= ~CRTSCTS;  // disable hardware flow control
	tty.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines (CLOCAL = 1)
	tty.c_lflag &= ~ICANON;  // disable canonical mode
	tty.c_lflag &= ~ECHO;  // disable echo
	tty.c_lflag &= ~ECHOE;  // disable erasure
	tty.c_lflag &= ~ECHONL;  // disable new-line echo
	tty.c_lflag &= ~ISIG;  // disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // disable software flow control
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // disable any special handling of received bytes
	tty.c_oflag &= ~OPOST;  // prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR;  // prevent conversion of newline to carriage return/line feed
	tty.c_cc[VTIME] = 0;  // no blocking (see https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/)
	tty.c_cc[VMIN] = 0;
	cfsetispeed(&tty, baudDef);
	cfsetospeed(&tty, baudDef);
	if (tcsetattr(m_port, TCSANOW, &tty) != 0) {
		disp(1, "error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
    disp(1, "opened port: %s, baud: %d, fd: %d", portName.c_str(), baud, m_port);
#endif
}


// destructor closes serial port
SerialPort::~SerialPort() {
	if (m_port >= 0) {
#ifndef WIN32
		close(m_port);
#endif
	}
	if (m_buf) {
		delete [] m_buf;
	}
}


// reads until receives untilChar; if hasn't yet received untilChar, returns empty string; 
// should call at least as often as will receive untilChar (this will only return one string (up to untilChar per call);
// untilChar itself won't be included
String SerialPort::readString(char untilChar) {
	String result;
	if (m_port >= 0) {
		// at start: m_pos is index of first unused byte in buffer
#ifdef WIN32
		int bytesRead = 0;
#else
		int bytesRead = read(m_port, m_buf + m_pos, m_bufLen - m_pos);
#endif
		if (bytesRead) {
            m_buf[m_pos + bytesRead] = 0;
			m_pos += bytesRead;
            m_checkForData = true;
        }
        if (m_checkForData) {
            bool found = false;
			for (int i = 0; i < m_pos; i++) {
				if (m_buf[i] == untilChar) {

					// copy up to (but not including) untilChar to result string
					m_buf[i] = 0;
					result = m_buf;

					// move the buffer forward
					int newStart = i + 1;  // char after untilChar will become new start of buffer
					for (int j = newStart; j < m_pos; j++) {
						m_buf[j - newStart] = m_buf[j];
					}
					m_pos -= newStart;
                    found = true;
					break;
				}
			}
            m_checkForData = found;  // don't need to check again until receive more data
		}
	}
	return result;
}


// send string
void SerialPort::writeString(const String &s) {
	if (m_port >= 0) {
#ifndef WIN32
        int len = s.length();
        int wrote_bytes = write(m_port, s.c_str(), len);
        if (wrote_bytes != len) {
            warning("only wrote %d of %d bytes", wrote_bytes, len);
        }
#endif
	}
}


// send a command immediately; add a checksum
void SerialPort::writeCommand(const String &command) {
	unsigned short crc = crc16(command.c_str());
	String fullCommand = sprintF("%s|%d\r\n", command.c_str(), crc);
	writeString(fullCommand);
	m_lastCommandTime = getPerfTime();
}


// ======== high-level message handling with queuing and acks functions ========


// sends a command or adds to outgoing queue
void SerialPort::sendCommand(const String &command, bool waitForAck) {
	if (m_outgoingMessages.count() == 0) {
		writeCommand(command);
	}
	m_outgoingMessages.appendCopy(command);  // append even if sending immediately, since the first item in the list should always be the most recently sent (until ack'd)
	if (waitForAck) {
		while (m_outgoingMessages.count()) {  // once list is empty, our command has been ack'd
			checkForMessages();
			if (checkCommandEvents()) {
				disp(1, "serial timeout waiting for ack (of %s); cancelling", command.c_str());
				break;
			}
		}
	}
}


// process any incoming serial messages (may handle multiple messages in a single call)
void SerialPort::checkForMessages() {
	while (true) {
		String message = readString(10);
		if (message.length() == 0) {
			break;  // if no incoming message, nothing else to do in this loop
		}

		// check checksum
        int checksum = message.rightOfLast('|').toInt();
        message = message.leftOfLast('|');
        if (checksum != crc16(message.c_str())) {
			disp(1, "checksum error on message: %s", message.c_str());
			m_checksumErrorCount++;

		// checksum is ok
		} else {

			// check for ack
			String deviceId = message.leftOfFirst(':');
			String messageBody = message.rightOfFirst(':').strip();  // TODO: if we get boards to ack without space, can remove this
			if (messageBody.startsWith("ack ")) {
				if (m_outgoingMessages.count()) {
					String origMessage = deviceId + ":" + messageBody.rightOfFirst(' ');  // TODO: we're assuming no space after colon in original message; maybe should store deviceId and messageBody separately
					if (origMessage == m_outgoingMessages[0]) {  // the most recent message is always at the front of the queue
						m_outgoingMessages.remove(0);
						if (m_outgoingMessages.count()) {  // if there's another message in the queue, send it now
							writeCommand(m_outgoingMessages[0]);
						}
					}
				}

			// pass non-ack messages to handler (if specified)
			} else if (m_handler) {
				m_handler->handleSerialMessage(message);
			}
		}
	}

	// resend last message if we've been waiting a while and it hasn't been ack'd
	if (m_outgoingMessages.count()) {
		if (getPerfTime() - m_lastCommandTime > 1.0) {
			disp(1, "resending: %s", m_outgoingMessages[0].c_str());
			writeCommand(m_outgoingMessages[0]);
		}
	}
}


// ======== checksum utilities ========


// an implementation of the CRC16-CCITT algorithm; assumes data is an 8-bit value
unsigned short crc16Update(unsigned short crc, unsigned char data) {
    data = data ^ (crc & 0xFF);
    data = data ^ ((data << 4) & 0xFF);
    return (((data << 8) & 0xFFFF) | ((crc >> 8) & 0xFF)) ^ (data >> 4) ^ (data << 3);
}


// an implementation of the CRC16-CCITT algorithm
unsigned short crc16(const char *message) {
    unsigned short crc = 0xFFFF;
	int i = 0;
    while (message[i]) {
		char c = message[i];
        crc = crc16Update(crc, c);
		i++;
	}
    return crc;
}


} // end namespace sbl

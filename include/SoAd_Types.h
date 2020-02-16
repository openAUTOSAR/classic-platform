/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef SOAD_TYPES_H
#define SOAD_TYPES_H
#include "Std_Types.h"

typedef enum {
	AF_INET = 0x02,	// Use IPv4
	AF_INET6 = 0x1c	// Use IPv6
} SoAd_DomainType;	/** @req SOAD110 */

typedef enum {
	IPPROTO_TCP = 0x06,	// Use TCP
	IPROTO_UDP = 0x11, 	// Use UDP
	SOL_SOCKET = 0xffff	// Options for socket level
} SoAd_ProtocolType;	/** @req SOAD111 */

typedef enum {
	SOCK_STREAM = 0x01,	// Use streaming socket (TCP)
	SOCK_DGRAM = 0x02	// Use datagram socket (UDP)
} SoAd_SocketType;	/** @req SOAD112 */

typedef struct {
	uint8			sa_len;			// Total length
	SoAd_DomainType	sa_family;		// Address family
	sint8 			sa_data[16];	// Address value
} SoAd_SockAddrType;	/** @req SOAD113 */

typedef enum {
	POLLIN = 0x0001,		// Any readable data available
	POLLPRI = 0x0002,		// OOB/Urgent readable data
	POLLOUT = 0x0004,		// File descriptor is writeable
	POLLRDNORM = 0x0040,	// Non-OOB/URG data available
	POLLRDNPORM = POLLOUT,	// No write type differentiation
	POLLRDBAND = 0x0080,	// OOB/Urgent readable data
	POLLWRBAND = 0x0100,	// OOB/Urgent data can be written
	POLLERR = 0x0008,		// Some poll error occurred
	POLLHUP = 0x0010,		// File descriptor was "hung up"
	POLLNVAL = 0x0020		// Request event "invalid"
} SoAd_PollEventType;	/** @req SOAD115 */

typedef struct {
	uint16	socket;					// Socket handle to be polled for events
	SoAd_PollEventType	event;		// Event to be reported
	SoAd_PollEventType	events;		// Events found
} SoAd_PollFdType;	/** @req SOAD114 */

typedef enum {
	SO_ACCEPTCONN = 0x0002,		// Socket has had listen()
	SO_ACCEPTFILTER = 0x1000,	// There is an accept filter
	SO_BROADCAST  = 0x0020,		// Permit sending of broadcast messages
	SO_DEBUG = 0x0001,			// Turn on debugging info recording
	SO_DONTROUTE = 0x0010,		// Just use interface addresses
	SO_KEEPALIVE = 0x0008,		// Keep connections alive
	SO_LINGER = 0x0080,			// Linger on close if data present
	SO_OOBINLINE = 0x0100,		// Leave received OOB data in line
	SO_RCVBUF = 0x1002, 		// Receive buffer size
	SO_RCVLOWAT = 0x1004,		// Receive low-water mark
	SO_RCVTIMEO = 0x1006,		// Receive timeout
	SO_REUSEADDR = 0x0004,		// Allow local address reuse
	SO_REUSEPORT = 0x0200,		// Allow local address &amp; port reuse
	SO_SNDBUF = 0x1001,			// Send buffer size
	SO_SNDLOWAT = 0x1003,		// Send low-water mark
	SO_SNDTIMEO = 0x1005,		// Send timeout
	SO_TIMESTAMP = 0x0400,		// Timesatmp received datagram traffic
	SO_USELOOPBACK = 0x0040,	// Bypass hardware when possible
	TCP_NODELAY = 0x0001,		// Don't delay send to coalesce packets
	TCP_MAXSEG = 0x0002,		// Set maximum segment size
	TCP_NOPUSH = 0x0004,		// Don't push last block of write
	TCP_NOOPT = 0x0008			// Don't use TCP options
} SoAd_SoOptionType; /** @req SOAD116 */

#if 0
typedef enum {
	ENOENT = 0x0002,			// No such file or directory
	EINTR = 0x0004,				// Interrupted system call
	EIO = 0x0005,				// Input/output error
	EBADF = 0x0009,				// Bad file descriptor
	EDEADLK = 0x000b,			// Resource deadlock avoided
	ENOMEM = 0x000c,			// Cannot allocate memory
	EACCESS = 0x000d,			// Permission denied
	ENOTDIR = 0x0014,			// Not a directory
	EISDIR = 0x0015,			// Is a directory
	EINVAL = 0x0016,			// Invalid argument
	ENFILE = 0x0017,			// Too many open files in system
	EMFILE = 0x0018,			// Too many open files
	EROFS = 0x001e,				// Read-only file system
	EPIPE = 0x0020,				// Broken pipe
	EDOM = 0x0021,				// Numerical argument out of domain
	EAGAIN = 0x0023,			// Resource temporarily unavailable
	EWOULDBLOCK = 0x0023,		// Operation would block
	EINPROGRESS = 0x0024,		// Operation now in progress
	EALREADY = 0x0025,			// Operation already in progress
	ENOTSOCK = 0x0026,			// Socket operation on non-socket
	EDESTADDRREQ = 0x0027,		// Destination address required
	EMSGSIZE = 0x0028,			// Message too long
	EPROTOTYPE = 0x0029,		// Protocol wrong type for socket
	ENOPROTOOPT = 0x002a,		// Protocol not available
	EPROTONOSUPPORT = 0x002b,	// Protocol not supported
	EOPNOTSUPP = 0x002d,		// Operation not supported
	// ENOTSUP  				// EOPNOTSUPP: Operation not supported
	EAFNOSUPPORT = 0x002f,		// Address family not supported by protocol family
	EADDRINUSE = 0x0030,		// Address already in use
	EADDRNOTAVAIL = 0x0031,		// Can't assign requested address
	ENETDOWN = 0x0032,			// Network is down
	ENETUNREACH = 0x0033,		// Network is unreachable
	ENETRESET = 0x0034,			// Network dropped connection on reset
	ECONNABORTED = 0x0035,		// Software caused connection abort
	ECONNRESET = 0x0036,		// Connection reset by peer
	ENOBUFS = 0x0037,			// No buffer space available
	EISCONN = 0x0038,			// Socket is already connected
	ENOTCONN = 0x0039,			// Socket is not connected
	ETIMEDOUT = 0x003c,			// Operation timed out
	ECONNREFUSED = 0x003d,		// Connection refused
	ELOOP = 0x003e,				// Too many levels of symbolic links
	ENAMETOOLONG = 0x003f,		// File name too long
	EHOSTDOWN = 0x0040,			// Host is down
	EHOSTUNREACH = 0x0041,		// No route to host
	ENOLCK = 0x004d,			// No locks available
	EOVERFLOW = 0x0054,			// Value too large to be stored in data type
} SoAd_TcpIpErrorType; /** @req SOAD117 */
#endif

typedef enum {
	O_NONBLOCK = 0x0004
} SoAd_FcntlFlagType;	/** @req SOAD118 */

typedef enum {
	F_GETFL = 0x0003,	// Get file status flags
	F_SETFL = 0x0004	// Set file status flags
} SoAd_FcntlCmdType;	/** @req SOAD119 */

typedef enum {
	MSG_OOB = 0x01,			// Process out-of-band data
	MSG_PEEK = 0x02,		// Peek at incoming message
	MSG_DONTROUTE = 0x04,	// Send without using routing tables
	MSG_EOR = 0x08,			// Data completes record
	MSG_TRUNC = 0x10,		// Data discarded before delivery
	MSG_CTRUNC = 0x20,		// Control data lost before delivery
	MSG_WAITALL = 0x40,		// Wait for full request or error
	MSG_DONTWAIT = 0x80,	// This message should be non blocking
	MSG_EOF = 0x100			// Data completes connection
}  SoAd_RecvfromFlagType;	/** @req SOAD142 */

#ifndef NO_SOAD_TCPIP_EVENT_TYPE
typedef enum {
	RESET = 0x01,		// TCP connection was reset
	CLOSED = 0x02		// TCP connetion was closed successfully
} SoAd_TcpIpEventType;	/** @req SOAD147 */
#endif

typedef struct {
	uint8*		payload;	// Pointer to payload
	uint32		totLen;		// Total length in bytes of this pbuf + all following
	uint16		len;		// Length of this pbuf in bytes
} SoAd_TcpIpPbufType;	/** @req SOAD190 */

typedef struct {
	uint16				port;	// TCP/UDP port
	SoAd_SockAddrType	addr;	// Address
} SoAd_TcpIp_IpAddrPortType;	/** @req SOAD192 */


typedef enum {
	SOAD_DOIP_NODETYPE_GATEWAY = 0x00,
	SOAD_DOIP_NODETYPE_NODE = 0x01,
	SOAD_DOIP_NODETYPE_NOF_VALID_MODES, // Reserved 2..255
} SoAd_DoIp_ArcNodeTypeType;

typedef uint8 SoAd_SoConIdType;

typedef uint8 SoAd_DoIp_PowerMode;


#endif

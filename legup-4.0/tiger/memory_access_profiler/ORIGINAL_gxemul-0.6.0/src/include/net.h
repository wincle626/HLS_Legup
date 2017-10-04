#ifndef	NET_H
#define	NET_H

/*
 *  Copyright (C) 2004-2010  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  Emulated network support.  (See net.c for more info.)
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct emul;
struct ethernet_packet_link;
struct remote_net;


/*  Default emulated "simple" IPv4 network, if nothing else is specified:  */
#define	NET_DEFAULT_IPV4_MASK		"10.0.0.0"
#define	NET_DEFAULT_IPV4_LEN		8


/*****************************************************************************/

/*
 *  NOTE: These are already defined in <net/ethernet.h> on e.g. FreeBSD,
 *        but they are missing in some systems (at least in Linux).
 */
#define	ETHERTYPE_SPRITE		0x0500
#define	ETHERTYPE_IP			0x0800
#define	ETHERTYPE_ARP			0x0806
#define	ETHERTYPE_REVARP		0x8035
#define	ETHERTYPE_IPV6			0x86DD

/*****************************************************************************/

/*  NOTE: udp_connection and tcp_connection are actually for
          internal use only.  */
struct udp_connection {
	int		in_use;
	int64_t		last_used_timestamp;

	/*  Inside:  */
	unsigned char	ethernet_address[6];
	unsigned char	inside_ip_address[4];
	int		inside_udp_port;

	/*  TODO: Fragment support for outgoing packets!  */
	int		fake_ns;

	/*  Outside:  */
	int		udp_id;
	int		socket;
	unsigned char	outside_ip_address[4];
	int		outside_udp_port;
};

struct tcp_connection {
	int		in_use;
	int64_t		last_used_timestamp;

	/*  Inside:  */
	unsigned char	ethernet_address[6];
	unsigned char	inside_ip_address[4];
	int		inside_tcp_port;
	uint32_t	inside_timestamp;

	/*  TODO:  tx and rx buffers?  */
	unsigned char	*incoming_buf;
	int		incoming_buf_rounds;
	int		incoming_buf_len;
	uint32_t	incoming_buf_seqnr;

	uint32_t	inside_seqnr;
	uint32_t	inside_acknr;
	uint32_t	outside_seqnr;
	uint32_t	outside_acknr;

	/*  Outside:  */
	int		state;
	int		tcp_id;
	int		socket;
	unsigned char	outside_ip_address[4];
	int		outside_tcp_port;
	uint32_t	outside_timestamp;
};

/*****************************************************************************/


#define	MAX_TCP_CONNECTIONS	100
#define	MAX_UDP_CONNECTIONS	100

struct net {
	/*  The emul struct which this net belong to:  */
	struct emul	*emul;

	/*  The network's addresses:  */
	struct in_addr	netmask_ipv4;
	int		netmask_ipv4_len;

	/*  NICs connected to this network:  */
	int		n_nics;
	void		**nic_extra;	/*  one void * per NIC  */

	/*  The "special machine":  */
	unsigned char	gateway_ipv4_addr[4];
	unsigned char	gateway_ethernet_addr[6];

	/*  Read from /etc/resolv.conf:  */
	char		*domain_name;
	int		nameserver_known;
	struct in_addr	nameserver_ipv4;

	int64_t		timestamp;

	struct ethernet_packet_link *first_ethernet_packet;
	struct ethernet_packet_link *last_ethernet_packet;

	struct udp_connection udp_connections[MAX_UDP_CONNECTIONS];
	struct tcp_connection tcp_connections[MAX_TCP_CONNECTIONS];

	/*  Distributed network:  */
	int		local_port;
	int		local_port_socket;
	struct remote_net *remote_nets;
};

/*  net_misc.c:  */
void net_debugaddr(void *addr, int type);
void net_generate_unique_mac(struct machine *, unsigned char *macbuf);
void send_udp(struct in_addr *addrp, int portnr, unsigned char *packet,
	size_t len);

/*  net_ip.c:  */
void net_ip_checksum(unsigned char *ip_header, int chksumoffset, int len);
void net_ip_tcp_checksum(unsigned char *tcp_header, int chksumoffset,
	int tcp_len, unsigned char *srcaddr, unsigned char *dstaddr,
	int udpflag);
void net_ip_tcp_connectionreply(struct net *net, void *extra,
	int con_id, int connecting, unsigned char *data, int datalen, int rst);
void net_ip_broadcast(struct net *net, void *extra,
        unsigned char *packet, int len);
void net_ip(struct net *net, void *extra, unsigned char *packet, int len);
void net_udp_rx_avail(struct net *net, void *extra);
void net_tcp_rx_avail(struct net *net, void *extra);

/*  net.c:  */
struct ethernet_packet_link *net_allocate_ethernet_packet_link(
	struct net *net, void *extra, size_t len);
int net_ethernet_rx_avail(struct net *net, void *extra);
int net_ethernet_rx(struct net *net, void *extra,
	unsigned char **packetp, int *lenp);
void net_ethernet_tx(struct net *net, void *extra,
	unsigned char *packet, int len);
void net_dumpinfo(struct net *net);
void net_add_nic(struct net *net, void *extra, unsigned char *macaddr);
struct net *net_init(struct emul *emul, int init_flags,
	const char *ipv4addr, int netipv4len, char **remote, int n_remote,
	int local_port, const char *settings_prefix);

/*  Flag used to signify that this net should have a gateway:  */
#define	NET_INIT_FLAG_GATEWAY		1


/*
 *  This is for internal use in src/net.c:
 */
struct ethernet_packet_link {
	struct ethernet_packet_link *prev;
	struct ethernet_packet_link *next;

	void		*extra;
	unsigned char	*data;
	int		len;
};

struct remote_net {
	struct remote_net *next;

	char		*name;
	struct in_addr	ipv4_addr;
	int		portnr;
};

#define	TCP_OUTSIDE_TRYINGTOCONNECT	1
#define	TCP_OUTSIDE_CONNECTED		2
#define	TCP_OUTSIDE_DISCONNECTED	3
#define	TCP_OUTSIDE_DISCONNECTED2	4

#define	TCP_INCOMING_BUF_LEN	2000

#define	NET_ADDR_IPV4		1
#define	NET_ADDR_IPV6		2
#define	NET_ADDR_ETHERNET	3

#endif	/*  NET_H  */

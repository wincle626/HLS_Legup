/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  Emulated network.
 *
 *  (Read the README file in this directory for more details.)
 *
 *
 *  NOTE: The 'extra' argument used in many functions in this file is a pointer
 *  to something unique for each NIC (i.e. the NIC itself :-), so that if
 *  multiple NICs are emulated concurrently, they will not get packets that
 *  are meant for some other controller.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#include "machine.h"
#include "misc.h"
#include "net.h"


/*  #define debug fatal  */


/*
 *  net_allocate_ethernet_packet_link():
 *
 *  This routine allocates an ethernet_packet_link struct, and adds it at
 *  the end of the packet chain.  A data buffer is allocated, and the data,
 *  extra, and len fields of the link are set.
 *
 *  Note: The data buffer is not zeroed.
 *
 *  Return value is a pointer to the link on success. It doesn't return on
 *  failure.
 */
struct ethernet_packet_link *net_allocate_ethernet_packet_link(
	struct net *net, void *extra, size_t len)
{
	struct ethernet_packet_link *lp;

	CHECK_ALLOCATION(lp = (struct ethernet_packet_link *)
	    malloc(sizeof(struct ethernet_packet_link)));

	lp->len = len;
	lp->extra = extra;
	CHECK_ALLOCATION(lp->data = (unsigned char *) malloc(len));

	lp->next = NULL;

	/*  Add last in the link chain:  */
	lp->prev = net->last_ethernet_packet;
	if (lp->prev != NULL)
		lp->prev->next = lp;
	else
		net->first_ethernet_packet = lp;
	net->last_ethernet_packet = lp;

	return lp;
}


/*
 *  net_arp():
 *
 *  Handle an ARP (or RARP) packet, coming from the emulated NIC.
 *
 *  An ARP packet might look like this:
 *
 *	ARP header:
 *	    ARP hardware addr family:	0001
 *	    ARP protocol addr family:	0800
 *	    ARP addr lengths:		06 04
 *	    ARP request:		0001
 *	    ARP from:			112233445566 01020304
 *	    ARP to:			000000000000 01020301
 *
 *  An ARP request with a 'to' IP value of the gateway should cause an
 *  ARP response packet to be created.
 *
 *  An ARP request with the same from and to IP addresses should be ignored.
 *  (This would be a host testing to see if there is an IP collision.)
 */
static void net_arp(struct net *net, void *extra,
	unsigned char *packet, int len, int reverse)
{
	int q;
	int i;

	/*  TODO: This debug dump assumes ethernet->IPv4 translation:  */
	if (reverse)
		debug("[ net: RARP: ");
	else
		debug("[ net: ARP: ");
	for (i=0; i<2; i++)
		debug("%02x", packet[i]);
	debug(" ");
	for (i=2; i<4; i++)
		debug("%02x", packet[i]);
	debug(" ");
	debug("%02x", packet[4]);
	debug(" ");
	debug("%02x", packet[5]);
	debug(" req=");
	debug("%02x", packet[6]);	/*  Request type  */
	debug("%02x", packet[7]);
	debug(" from=");
	for (i=8; i<18; i++)
		debug("%02x", packet[i]);
	debug(" to=");
	for (i=18; i<28; i++)
		debug("%02x", packet[i]);
	debug(" ]\n");

	if (packet[0] == 0x00 && packet[1] == 0x01 &&
	    packet[2] == 0x08 && packet[3] == 0x00 &&
	    packet[4] == 0x06 && packet[5] == 0x04) {
		int r = (packet[6] << 8) + packet[7];
		struct ethernet_packet_link *lp;

		switch (r) {
		case 1:		/*  Request  */
			/*  Only create a reply if this was meant for the
			    gateway:  */
			if (memcmp(packet+24, net->gateway_ipv4_addr, 4) != 0)
				break;

			lp = net_allocate_ethernet_packet_link(
			    net, extra, 60 + 14);

			/*  Copy the old packet first:  */
			memset(lp->data, 0, 60 + 14);
			memcpy(lp->data + 14, packet, len);

			/*  Add ethernet ARP header:  */
			memcpy(lp->data + 0, lp->data + 8 + 14, 6);
			memcpy(lp->data + 6, net->gateway_ethernet_addr, 6);
			lp->data[12] = 0x08; lp->data[13] = 0x06;

			/*  Address of the emulated machine:  */
			memcpy(lp->data + 18 + 14, lp->data + 8 + 14, 10);

			/*  Address of the gateway:  */
			memcpy(lp->data +  8 + 14, net->gateway_ethernet_addr,
			    6);
			memcpy(lp->data + 14 + 14, net->gateway_ipv4_addr, 4);

			/*  This is a Reply:  */
			lp->data[6 + 14] = 0x00; lp->data[7 + 14] = 0x02;

			break;
		case 3:		/*  Reverse Request  */
			lp = net_allocate_ethernet_packet_link(
			    net, extra, 60 + 14);

			/*  Copy the old packet first:  */
			memset(lp->data, 0, 60 + 14);
			memcpy(lp->data + 14, packet, len);

			/*  Add ethernet RARP header:  */
			memcpy(lp->data + 0, packet + 8, 6);
			memcpy(lp->data + 6, net->gateway_ethernet_addr, 6);
			lp->data[12] = 0x80; lp->data[13] = 0x35;

			/*  This is a RARP reply:  */
			lp->data[6 + 14] = 0x00; lp->data[7 + 14] = 0x04;

			/*  Address of the gateway:  */
			memcpy(lp->data +  8 + 14, net->gateway_ethernet_addr,
			    6);
			memcpy(lp->data + 14 + 14, net->gateway_ipv4_addr, 4);

			/*  MAC address of emulated machine:  */
			memcpy(lp->data + 18 + 14, packet + 8, 6);

			/*
			 *  IP address of the emulated machine:  Automagically
			 *  generated from the MAC address. :-)
			 *
			 *  packet+8 points to the client's mac address,
			 *  for example 10:20:30:00:00:z0, where z is 0..15.
			 *  10:20:30:00:00:10 results in 10.0.0.1.
			 */
			/*  q = (packet[8 + 3]) >> 4;  */
			/*  q = q*15 + ((packet[8 + 4]) >> 4);  */
			q = (packet[8 + 5]) >> 4;
			lp->data[24 + 14] = 10;
			lp->data[25 + 14] =  0;
			lp->data[26 + 14] =  0;
			lp->data[27 + 14] =  q;
			break;
		case 2:		/*  Reply  */
		case 4:		/*  Reverse Reply  */
		default:
			fatal("[ net: ARP: UNIMPLEMENTED request type "
			    "0x%04x ]\n", r);
		}
	} else {
		fatal("[ net: ARP: UNIMPLEMENTED arp packet type: ");
		for (i=0; i<len; i++)
			fatal("%02x", packet[i]);
		fatal(" ]\n");
	}
}


/*
 *  net_ethernet_rx_avail():
 *
 *  Return 1 if there is a packet available for this 'extra' pointer, otherwise
 *  return 0.
 *
 *  Appart from actually checking for incoming packets from the outside world,
 *  this function basically works like net_ethernet_rx() but it only receives
 *  a return value telling us whether there is a packet or not, we don't
 *  actually get the packet.
 */
int net_ethernet_rx_avail(struct net *net, void *extra)
{
	if (net == NULL)
		return 0;

	/*
	 *  If the network is distributed across multiple emulator processes,
	 *  then receive incoming packets from those processes.
	 */
	if (net->local_port != 0) {
		struct sockaddr_in si;
		socklen_t si_len = sizeof(si);
		int res, i, nreceived = 0;
		unsigned char buf[60000];

		do {
			res = recvfrom(net->local_port_socket, buf,
			    sizeof(buf), 0, (struct sockaddr *)&si, &si_len);

			if (res != -1) {
				nreceived ++;

				/*  fatal("[ incoming DISTRIBUTED packet, %i "
				    "bytes from %s:%d\n", res,
				    inet_ntoa(si.sin_addr),
				    ntohs(si.sin_port));  */

				/*  Add the packet to all "our" NICs on this
				    network:  */
				for (i=0; i<net->n_nics; i++) {
					struct ethernet_packet_link *lp;
					lp = net_allocate_ethernet_packet_link(
					    net, net->nic_extra[i], res);
					memcpy(lp->data, buf, res);
				}
			}
		} while (res != -1 && nreceived < 100);
	}

	/*  IP protocol specific:  */
	net_udp_rx_avail(net, extra);
	net_tcp_rx_avail(net, extra);

	return net_ethernet_rx(net, extra, NULL, NULL);
}


/*
 *  net_ethernet_rx():
 *
 *  Receive an ethernet packet. (This means handing over an already prepared
 *  packet from this module to a specific ethernet controller device.)
 *
 *  Return value is 1 if there was a packet available. *packetp and *lenp
 *  will be set to the packet's data pointer and length, respectively, and
 *  the packet will be removed from the linked list). If there was no packet
 *  available, 0 is returned.
 *
 *  If packetp is NULL, then the search is aborted as soon as a packet with
 *  the correct 'extra' field is found, and a 1 is returned, but as packetp
 *  is NULL we can't return the actual packet. (This is the internal form
 *  if net_ethernet_rx_avail().)
 */
int net_ethernet_rx(struct net *net, void *extra,
	unsigned char **packetp, int *lenp)
{
	struct ethernet_packet_link *lp, *prev;

	if (net == NULL)
		return 0;

	/*  Find the first packet which has the right 'extra' field.  */

	lp = net->first_ethernet_packet;
	prev = NULL;
	while (lp != NULL) {
		if (lp->extra == extra) {
			/*  We found a packet for this controller!  */
			if (packetp == NULL || lenp == NULL)
				return 1;

			/*  Let's return it:  */
			(*packetp) = lp->data;
			(*lenp) = lp->len;

			/*  Remove this link from the linked list:  */
			if (prev == NULL)
				net->first_ethernet_packet = lp->next;
			else
				prev->next = lp->next;

			if (lp->next == NULL)
				net->last_ethernet_packet = prev;
			else
				lp->next->prev = prev;

			free(lp);

			/*  ... and return successfully:  */
			return 1;
		}

		prev = lp;
		lp = lp->next;
	}

	/*  No packet found. :-(  */
	return 0;
}


/*
 *  net_ethernet_tx():
 *
 *  Transmit an ethernet packet, as seen from the emulated ethernet controller.
 *  If the packet can be handled here, it will not necessarily be transmitted
 *  to the outside world.
 */
void net_ethernet_tx(struct net *net, void *extra,
	unsigned char *packet, int len)
{
	int i, eth_type, for_the_gateway;

	if (net == NULL)
		return;

	for_the_gateway = !memcmp(packet, net->gateway_ethernet_addr, 6);

	/*  Drop too small packets:  */
	if (len < 20) {
		fatal("[ net_ethernet_tx: Warning: dropping tiny packet "
		    "(%i bytes) ]\n", len);
		return;
	}

	/*
	 *  Copy this packet to all other NICs on this network (except if
	 *  it is aimed specifically at the gateway's ethernet address):
	 */
	if (!for_the_gateway && extra != NULL && net->n_nics > 0) {
		for (i=0; i<net->n_nics; i++)
			if (extra != net->nic_extra[i]) {
				struct ethernet_packet_link *lp;
				lp = net_allocate_ethernet_packet_link(net,
				    net->nic_extra[i], len);

				/*  Copy the entire packet:  */
				memcpy(lp->data, packet, len);
			}
	}

	/*
	 *  If this network is distributed across multiple emulator processes,
	 *  then transmit the packet to those other processes.
	 */
	if (!for_the_gateway && net->remote_nets != NULL) {
		struct remote_net *rnp = net->remote_nets;
		while (rnp != NULL) {
			send_udp(&rnp->ipv4_addr, rnp->portnr, packet, len);
			rnp = rnp->next;
		}
	}


	/*
	 *  The code below simulates the behaviour of a "NAT"-style gateway.
	 *
	 *  Packets that are not destined for the gateway are dropped first:
	 *  (DHCP packets are let through, though.)
	 */

	if (!for_the_gateway && packet[0] != 0xff && packet[0] != 0x00)
		return;

#if 0
	fatal("[ net: ethernet: ");
	for (i=0; i<6; i++)	fatal("%02x", packet[i]); fatal(" ");
	for (i=6; i<12; i++)	fatal("%02x", packet[i]); fatal(" ");
	for (i=12; i<14; i++)	fatal("%02x", packet[i]); fatal(" ");
	for (i=14; i<len; i++)	fatal("%02x", packet[i]); fatal(" ]\n");
#endif

	eth_type = (packet[12] << 8) + packet[13];

	/*  IP:  */
	if (eth_type == ETHERTYPE_IP) {
		/*  Routed via the gateway?  */
		if (for_the_gateway) {
			net_ip(net, extra, packet, len);
			return;
		}

		/*  Broadcast? (DHCP does this.)  */
		if (packet[0] == 0xff && packet[1] == 0xff &&
		    packet[2] == 0xff && packet[3] == 0xff &&
		    packet[4] == 0xff && packet[5] == 0xff) {
			net_ip_broadcast(net, extra, packet, len);
			return;
		}

		if (net->n_nics < 2) {
			fatal("[ net_ethernet_tx: IP packet not for gateway, "
			    "and not broadcast: ");
			for (i=0; i<14; i++)
				fatal("%02x", packet[i]);
			fatal(" ]\n");
		}
		return;
	}

	/*  ARP:  */
	if (eth_type == ETHERTYPE_ARP) {
		if (len != 42 && len != 60)
			fatal("[ net_ethernet_tx: WARNING! unusual "
			    "ARP len (%i) ]\n", len);
		net_arp(net, extra, packet + 14, len - 14, 0);
		return;
	}

	/*  RARP:  */
	if (eth_type == ETHERTYPE_REVARP) {
		net_arp(net, extra, packet + 14, len - 14, 1);
		return;
	}

	/*  Sprite:  */
	if (eth_type == ETHERTYPE_SPRITE) {
		/*  TODO.  */
		fatal("[ net: TX: UNIMPLEMENTED Sprite packet ]\n");
		return;
	}

	/*  IPv6:  */
	if (eth_type == ETHERTYPE_IPV6) {
		/*  TODO.  */
		fatal("[ net_ethernet_tx: IPv6 is not implemented yet! ]\n");
		return;
	}

	fatal("[ net_ethernet_tx: ethernet packet type 0x%04x not yet "
	    "implemented ]\n", eth_type);
}


/*
 *  parse_resolvconf():
 *
 *  This function parses "/etc/resolv.conf" to figure out the nameserver
 *  and domain used by the host.
 */
static void parse_resolvconf(struct net *net)
{
	FILE *f;
	char buf[8000];
	size_t len;
	int res;
	unsigned int i, start;

	/*
	 *  This is a very ugly hack, which tries to figure out which
	 *  nameserver the host uses by looking for the string 'nameserver'
	 *  in /etc/resolv.conf.
	 *
	 *  This can later on be used for DHCP autoconfiguration.  (TODO)
	 *
	 *  TODO: This is hardcoded to use /etc/resolv.conf. Not all
	 *        operating systems use that filename.
	 *
	 *  TODO: This is hardcoded for AF_INET (that is, IPv4).
	 *
	 *  TODO: This assumes that the first nameserver listed is the
	 *        one to use.
	 */
	f = fopen("/etc/resolv.conf", "r");
	if (f == NULL)
		return;

	/*  TODO: get rid of the hardcoded values  */
	memset(buf, 0, sizeof(buf));
	len = fread(buf, 1, sizeof(buf) - 100, f);
	fclose(f);
	buf[sizeof(buf) - 1] = '\0';

	for (i=0; i<len; i++)
		if (strncmp(buf+i, "nameserver", 10) == 0) {
			char *p;

			/*
			 *  "nameserver" (1 or more whitespace)
			 *  "x.y.z.w" (non-digit)
			 */

			/*  debug("found nameserver at offset %i\n", i);  */
			i += 10;
			while (i<len && (buf[i]==' ' || buf[i]=='\t'))
				i++;
			if (i >= len)
				break;
			start = i;

			p = buf+start;
			while ((*p >= '0' && *p <= '9') || *p == '.')
				p++;
			*p = '\0';

#ifdef HAVE_INET_PTON
			res = inet_pton(AF_INET, buf + start,
			    &net->nameserver_ipv4);
#else
			res = inet_aton(buf + start, &net->nameserver_ipv4);
#endif
			if (res < 1)
				break;

			net->nameserver_known = 1;
			break;
		}

	for (i=0; i<len; i++)
		if (strncmp(buf+i, "domain", 6) == 0) {
			/*  "domain" (1 or more whitespace) domain_name  */
			i += 6;
			while (i<len && (buf[i]==' ' || buf[i]=='\t'))
				i++;
			if (i >= len)
				break;

			start = i;
			while (i<len && buf[i]!='\n' && buf[i]!='\r')
				i++;
			if (i < len)
				buf[i] = '\0';
			/*  fatal("DOMAIN='%s'\n", buf + start);  */
			CHECK_ALLOCATION(net->domain_name = strdup(buf+start));
			break;
		}
}


/*
 *  net_add_nic():
 *
 *  Add a NIC to a network. (All NICs on a network will see each other's
 *  packets.)
 */
void net_add_nic(struct net *net, void *extra, unsigned char *macaddr)
{
	if (net == NULL)
		return;

	if (extra == NULL) {
		fprintf(stderr, "net_add_nic(): extra = NULL\n");
		exit(1);
	}

	net->n_nics ++;
	CHECK_ALLOCATION(net->nic_extra = (void **)
	    realloc(net->nic_extra, sizeof(void *) * net->n_nics));

	net->nic_extra[net->n_nics - 1] = extra;
}


/*
 *  net_gateway_init():
 *
 *  This function creates a "gateway" machine (for example at IPv4 address
 *  10.0.0.254, if the net is 10.0.0.0/8), which acts as a gateway/router/
 *  nameserver etc.
 */
static void net_gateway_init(struct net *net)
{
	unsigned char *p = (unsigned char *) &net->netmask_ipv4;
	uint32_t x;
	int xl;

	x = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	xl = 32 - net->netmask_ipv4_len;
	if (xl > 8)
		xl = 8;
	x |= ((1 << xl) - 1) & ~1;

	net->gateway_ipv4_addr[0] = x >> 24;
	net->gateway_ipv4_addr[1] = x >> 16;
	net->gateway_ipv4_addr[2] = x >> 8;
	net->gateway_ipv4_addr[3] = x;

	net->gateway_ethernet_addr[0] = 0x60;
	net->gateway_ethernet_addr[1] = 0x50;
	net->gateway_ethernet_addr[2] = 0x40;
	net->gateway_ethernet_addr[3] = 0x30;
	net->gateway_ethernet_addr[4] = 0x20;
	net->gateway_ethernet_addr[5] = 0x10;
}


/*
 *  net_dumpinfo():
 *
 *  Called from the debugger's "machine" command, to print some info about
 *  a network.
 */
void net_dumpinfo(struct net *net)
{
	int iadd = DEBUG_INDENTATION;
	struct remote_net *rnp;

	debug("net:\n");

	debug_indentation(iadd);

	debug("simulated network: ");
	net_debugaddr(&net->netmask_ipv4, NET_ADDR_IPV4);
	debug("/%i", net->netmask_ipv4_len);

	debug(" (max outgoing: TCP=%i, UDP=%i)\n",
	    MAX_TCP_CONNECTIONS, MAX_UDP_CONNECTIONS);

	debug("simulated gateway+nameserver: ");
	net_debugaddr(&net->gateway_ipv4_addr, NET_ADDR_IPV4);
	debug(" (");
	net_debugaddr(&net->gateway_ethernet_addr, NET_ADDR_ETHERNET);
	debug(")\n");

	if (!net->nameserver_known) {
		debug("(could not determine nameserver)\n");
	} else {
		debug("simulated nameserver uses real nameserver ");
		net_debugaddr(&net->nameserver_ipv4, NET_ADDR_IPV4);
		debug("\n");
	}

	if (net->domain_name != NULL && net->domain_name[0])
		debug("domain: %s\n", net->domain_name);

	rnp = net->remote_nets;
	if (net->local_port != 0)
		debug("distributed network: local port = %i\n",
		    net->local_port);
	debug_indentation(iadd);
	while (rnp != NULL) {
		debug("remote \"%s\": ", rnp->name);
		net_debugaddr(&rnp->ipv4_addr, NET_ADDR_IPV4);
		debug(" port %i\n", rnp->portnr);
		rnp = rnp->next;
	}
	debug_indentation(-iadd);

	debug_indentation(-iadd);
}


/*
 *  net_init():
 *
 *  This function creates a network, and returns a pointer to it.
 *
 *  ipv4addr should be something like "10.0.0.0", netipv4len = 8.
 *
 *  If n_remote is more than zero, remote should be a pointer to an array
 *  of strings of the following format: "host:portnr".
 *
 *  Network settings are registered if settings_prefix is non-NULL.
 *  (The one calling net_init() is also responsible for calling net_deinit().)
 *
 *  On failure, exit() is called.
 */
struct net *net_init(struct emul *emul, int init_flags,
	const char *ipv4addr, int netipv4len,
	char **remote, int n_remote, int local_port,
	const char *settings_prefix)
{
	struct net *net;
	int res;

	CHECK_ALLOCATION(net = (struct net *) malloc(sizeof(struct net)));
	memset(net, 0, sizeof(struct net));

	/*  Set the back pointer:  */
	net->emul = emul;

	/*  Sane defaults:  */
	net->timestamp = 0;
	net->first_ethernet_packet = net->last_ethernet_packet = NULL;

#ifdef HAVE_INET_PTON
	res = inet_pton(AF_INET, ipv4addr, &net->netmask_ipv4);
#else
	res = inet_aton(ipv4addr, &net->netmask_ipv4);
#endif
	if (res < 1) {
		fprintf(stderr, "net_init(): could not parse IPv4 address"
		    " '%s'\n", ipv4addr);
		exit(1);
	}

	if (netipv4len < 1 || netipv4len > 30) {
		fprintf(stderr, "net_init(): extremely weird ipv4 "
		    "network length (%i)\n", netipv4len);
		exit(1);
	}
	net->netmask_ipv4_len = netipv4len;

	net->nameserver_known = 0;
	net->domain_name = strdup("");
	parse_resolvconf(net);

	/*  Distributed network? Then add remote hosts:  */
	if (local_port != 0) {
		struct sockaddr_in si_self;

		net->local_port = local_port;
		net->local_port_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (net->local_port_socket < 0) {
			perror("socket");
			exit(1);
		}

		memset((char *)&si_self, 0, sizeof(si_self));
		si_self.sin_family = AF_INET;
		si_self.sin_port = htons(local_port);
		si_self.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(net->local_port_socket, (struct sockaddr *)&si_self,
		    sizeof(si_self)) < 0) {
			perror("bind");
			exit(1);
		}

		/*  Set the socket to non-blocking:  */
		res = fcntl(net->local_port_socket, F_GETFL);
		fcntl(net->local_port_socket, F_SETFL, res | O_NONBLOCK);
	}
	if (n_remote != 0) {
		struct remote_net *rnp;
		while ((n_remote--) != 0) {
			struct hostent *hp;

			/*  debug("adding '%s'\n", remote[n_remote]);  */
			CHECK_ALLOCATION(rnp = (struct remote_net *)
			    malloc(sizeof(struct remote_net)));
			memset(rnp, 0, sizeof(struct remote_net));

			rnp->next = net->remote_nets;
			net->remote_nets = rnp;

			CHECK_ALLOCATION(rnp->name = strdup(remote[n_remote]));
			if (strchr(rnp->name, ':') != NULL)
				strchr(rnp->name, ':')[0] = '\0';

			hp = gethostbyname(rnp->name);
			if (hp == NULL) {
				fprintf(stderr, "could not resolve '%s'\n",
				    rnp->name);
				exit(1);
			}
			memcpy(&rnp->ipv4_addr, hp->h_addr, hp->h_length);
			free(rnp->name);

			/*  And again:  */
			CHECK_ALLOCATION(rnp->name = strdup(remote[n_remote]));
			if (strchr(rnp->name, ':') == NULL) {
				fprintf(stderr, "Remote network '%s' is not "
				    "'host:portnr'?\n", rnp->name);
				exit(1);
			}
			rnp->portnr = atoi(strchr(rnp->name, ':') + 1);
		}
	}

	if (init_flags & NET_INIT_FLAG_GATEWAY)
		net_gateway_init(net);

	net_dumpinfo(net);

	/*  This is necessary when using the real network:  */
	signal(SIGPIPE, SIG_IGN);

	return net;
}


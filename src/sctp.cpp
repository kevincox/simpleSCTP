// Copyright 2013 Kevin Cox

/*******************************************************************************
*                                                                              *
*  This software is provided 'as-is', without any express or implied           *
*  warranty. In no event will the authors be held liable for any damages       *
*  arising from the use of this software.                                      *
*                                                                              *
*  Permission is granted to anyone to use this software for any purpose,       *
*  including commercial applications, and to alter it and redistribute it      *
*  freely, subject to the following restrictions:                              *
*                                                                              *
*  1. The origin of this software must not be misrepresented; you must not     *
*     claim that you wrote the original software. If you use this software in  *
*     a product, an acknowledgment in the product documentation would be       *
*     appreciated but is not required.                                         *
*                                                                              *
*  2. Altered source versions must be plainly marked as such, and must not be  *
*     misrepresented as being the original software.                           *
*                                                                              *
*  3. This notice may not be removed or altered from any source distribution.  *
*                                                                              *
*******************************************************************************/

#include "sctp.hpp"

#include <stdio.h>
#include <string.h>

#include <netinet/sctp.h>

enum PPIDFlags {
	MORE = 1,
};

Remote::Remote():
	a({0}),
	alen(sizeof(a))
{
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = htonl(INADDR_ANY);
}

int Remote::setIP(string ip)
{
	inet_pton(AF_INET, ip.c_str(), &a.sin_addr);
	
	return 0;
}

int Remote::setPort(uint16_t port)
{
	a.sin_port = htons(1234);
	
	return 0;
}

SCTP::SCTP()
{
	s = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	if ( s < 0 ) perror("socket");
	
	int r;
	
	/*struct sctp_event event = {0};
	
	event.se_assoc_id = SCTP_ALL_ASSOC;
	event.se_type = SCTP_SNDINFO;
	event.se_on = 1;
	r = setsockopt(s, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(event));
	if ( r < 0 ) perror("setsockopt(,IPPROTO_SCTP,SCTP_EVENT,SCTP_SNDINFO=1,)");*/
	
	struct sctp_event_subscribe events = {0};
	events.sctp_data_io_event = 1;
	//events.sctp_association_event;
	//events.sctp_address_event;
	//events.sctp_send_failure_event;
	//events.sctp_peer_error_event;
	//events.sctp_shutdown_event;
	//events.sctp_partial_delivery_event;
	//events.sctp_adaptation_layer_event;
	//events.sctp_authentication_event;
	//events.sctp_sender_dry_event;
	r = setsockopt(s, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));
	if ( r < 0 ) perror("setsockopt(,IPPROTO_SCTP,SCTP_EVENTS,,)");

	int v;
	socklen_t vlen = sizeof(v);
	
	#ifdef SCTP_EXPLICIT_EOR
	v = 1;
	r = setsockopt(s, SOL_SOCKET, SCTP_EXPLICIT_EOR, &v, vlen);
	if ( r < 0 ) perror("getsockopt(,SOL_SOCKET,SCTP_EXPLICIT_EOR,,)");
	#endif
}

void SCTP::listSocOptions()
{
	int r;
	
	int i;
	socklen_t ilen = sizeof(i);
	
	#ifdef SO_SNDBUF
	r = getsockopt(s, SOL_SOCKET, SO_SNDBUF, &i, &ilen);
	if ( r < 0 ) perror("getsockopt(,SOL_SOCKET,SO_SNDBUF,,)");
	else         printf("SO_SNDBUF: %d\n", i);
	#endif
	
	#ifdef SO_RCVBUF
	r = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &i, &ilen);
	if ( r < 0 ) perror("getsockopt(,SOL_SOCKET,SO_RCVBUF,,)");
	else         printf("SO_RCVBUF: %d\n", i);
	#endif
	
	#ifdef SCTP_DISABLE_FRAGMENTS
	r = getsockopt(s, SOL_SOCKET, SCTP_DISABLE_FRAGMENTS, &i, &ilen);
	if ( r < 0 ) perror("getsockopt(,SOL_SOCKET,SCTP_DISABLE_FRAGMENTS,,)");
	else         printf("SCTP_DISABLE_FRAGMENTS: %s (%d)\n", i?"true":"false", i);
	#endif
	
	#ifdef SCTP_EXPLICIT_EOR
	r = getsockopt(s, SOL_SOCKET, SCTP_EXPLICIT_EOR, &i, &ilen);
	if ( r < 0 ) perror("getsockopt(,SOL_SOCKET,SCTP_EXPLICIT_EOR,,)");
	else         printf("SCTP_EXPLICIT_EOR: %s (%d)\n", i?"true":"false", i);
	#endif
	
}

unsigned SCTP::listen(string addr, uint16_t port, uint16_t buffer)
{
	struct sockaddr_in saddr = {0};
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(port);
	
	int r = bind(s, (struct sockaddr*)&saddr, sizeof(saddr));
	if ( r < 0 ) perror("bind()");
	r = ::listen(s, buffer);
	if ( r < 0 ) perror("listen()");
	
	return 0;
}

unsigned SCTP::send(Remote *r, uint16_t stream, vector<uint8_t> &data)
{
	int sbuf;
	socklen_t sbuflen = sizeof(sbuf);
	
	int res = getsockopt(s, SOL_SOCKET, SO_SNDBUF, &sbuf, &sbuflen);
	if ( res < 0 ) perror("getsockopt(,SOL_SOCKET,SO_SNDBUF,,)");
	//sbuf = 4;
	
	//printf("SO_SNDBUF: %d\n", sbuf);
	
	struct iovec d;
	
	struct msghdr mh = {0};
	mh.msg_name    = (sockaddr*)&r->a;
	mh.msg_namelen = r->alen;
	mh.msg_iov    = &d;
	mh.msg_iovlen = 1;
	mh.msg_controllen = sizeof(struct cmsghdr)+sizeof(struct sctp_sndrcvinfo);
	mh.msg_control = malloc(mh.msg_controllen);
	memset(mh.msg_control, 0, mh.msg_controllen);
	if (!mh.msg_control) perror("malloc");
	
	struct cmsghdr *cm = CMSG_FIRSTHDR(&mh);
	cm->cmsg_len   = sizeof(struct cmsghdr)+sizeof(struct sctp_sndrcvinfo);
	cm->cmsg_level = IPPROTO_SCTP;
	cm->cmsg_type  = SCTP_SNDRCV;
	struct sctp_sndrcvinfo *i = (struct sctp_sndrcvinfo*)CMSG_DATA(cm);
	i->sinfo_stream = stream;
	
	#ifdef EXPLICIT_EOR_HACK
	i->sinfo_ppid = PPIDFlags::MORE;
	#endif
	
	int tsent = 0;
	#if defined(SCTP_EXPLICIT_EOR) || defined(EXPLICIT_EOR_HACK)
	while ( data.size() > tsent+sbuf )
	{
		d.iov_base = (void*) (data.data()+tsent);
		d.iov_len  = sbuf;
	
		int sent = sendmsg(s, &mh, 0);
		if ( sent < 0 ) perror("sendmsg");
		//printf("Sent: %d bytes.\n", sent);
		tsent += sent;
	}
	#endif
	
	#ifdef EXPLICIT_EOR_HACK
	i->sinfo_ppid = 0;
	#endif
	
	d.iov_base = (void*) (data.data()+tsent);
	d.iov_len  = data.size()-tsent;
	
	int sent = sendmsg(s, &mh, MSG_EOR);
	if ( sent < 0 ) perror("sendmsg");
	//printf("Sent: %d bytes.\n", sent);
	
	free(mh.msg_control);
	
	return 0;
}

unsigned SCTP::recieve(SCTPMessage *m)
{
	int r;
	int rbuf;
	socklen_t optlen = sizeof(rbuf);
	r = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rbuf, &optlen);
	if ( r < 0 ) perror("getsockopt(,SOL_SOCKET,SO_RCVBUF,,)");
	
	size_t received = 0;

	struct iovec d;
	
	struct msghdr mh = {0};
	mh.msg_iov    = &d;
	mh.msg_iovlen = 1;
	mh.msg_controllen = sizeof(struct cmsghdr) + sizeof(struct sctp_sndrcvinfo);
	mh.msg_control = malloc(mh.msg_controllen);
	if (!mh.msg_control) perror("malloc");
	
	do {
		m->data.resize(received+rbuf);
		d.iov_base = (void*) (m->data.data()+received);
		d.iov_len  = rbuf;
		
		m->flags = 0;
		//int l = sctp_recvmsg(s, &m->data.front(), m->data.length(),
		//                     (struct sockaddr*)&m->r.a, &m->r.alen,
		//                     &m->i, &m->flags);
		int l = recvmsg(s, &mh, 0);
		if ( l < 0 ) perror("recvmsg");
		//printf("read %d bytes.\n", l);
		
		
		for ( struct cmsghdr *cm = CMSG_FIRSTHDR(&mh); cm; cm = CMSG_NXTHDR(&mh, cm))
		{
			if (cm->cmsg_len == 0)
			{
				fprintf(stderr, "Something bad happened at %s:%d\n", __FILE__, __LINE__);
				break;
			}
			if ( cm->cmsg_level == IPPROTO_SCTP && cm->cmsg_type == SCTP_SNDRCV )
			{
				struct sctp_sndrcvinfo *i = (struct sctp_sndrcvinfo*)CMSG_DATA(cm);
				
				if ( i->sinfo_ppid & PPIDFlags::MORE ) // EOR hack.
					mh.msg_flags &= ~MSG_EOR;
			}
		}
		
		//printf("f: %x, eor: %x, tru: %x\n", mh.msg_flags, MSG_EOR, MSG_TRUNC);
		
		received += l;
	} while (!( mh.msg_flags & MSG_EOR ));
	
	for ( struct cmsghdr *cm = CMSG_FIRSTHDR(&mh); cm; cm = CMSG_NXTHDR(&mh, cm))
	{
		if (cm->cmsg_len == 0)
		{
			fprintf(stderr, "Something bad happened at %s:%d\n", __FILE__, __LINE__);
			break;
		}
		if ( cm->cmsg_level == IPPROTO_SCTP && cm->cmsg_type == SCTP_SNDRCV )
		{
			struct sctp_sndrcvinfo *i = (struct sctp_sndrcvinfo*)CMSG_DATA(cm);
			
			m->stream = i->sinfo_stream;
		}
	}
	
	//printf("Received %ld bytes.\n", received);
	m->data.resize(received);
	free(mh.msg_control);
	
	return 0;
}

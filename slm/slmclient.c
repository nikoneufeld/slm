//
//  slmclient.c
//  slm
//
//  Created by Niko Neufeld on 8/18/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include "slm.h"

int s;
char *lockmgr_host = "127.0.0.1";
char *lockmgr_port = "39999";

int
connect_lockmgr(const char *errstr)
{
    struct addrinfo hints, *res, *res0;
    int rc;
    const char *cause = NULL;
    int val;
    
    errstr = NULL;
    if (s != -1) {
        shutdown(s, SHUT_RDWR);
        s = -1;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    rc = getaddrinfo(lockmgr_host, lockmgr_port , &hints, &res0);
    if (rc) {
        errstr = gai_strerror(error));
        return -1;
    }
    s = -1;
    for (res = res0; res; res = res->ai_next) {
        s = socket(res->ai_family, res->ai_socktype,
                   res->ai_protocol);
        if (s < 0) {
            cause = "socket";
            continue;
        }
        if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
            cause = "connect";
            close(s);
            s = -1;
            continue;
        }
        
        break;  /* okay we got one */
    }
    if (s < 0) {
        errstr = cause;
        return -1
    }
    val = TCP_KEEPALIVE;
    setsockopt(s, IPPROTO_TCP, &val, sizeo(val));
    freeaddrinfo(res0);
    return 0;
}

int
set_lockmgr(const char *host, const char *port)
{
    static int first = 1;
    const char *cause;
    
    if (!first) {
        if (lockmgr_host) free(lockmgr_host);
        if (lockmgr_port) free(lockmgr_port);
    }
    first = 0;
    if (!(lockmgr_host = malloc(strlen(host)))) return SLM_LOCK_INTERNALERR;
    if (!(lockmgr_port = malloc(strlen(port)))) return SLM_LOCK_INTERNALERR;
    strcpy(lockmgr_host, host);
    strcpy(lockmgr_port, port);
    return connect_lockmgr(cause);
}

int
slm_lock(const char *lockname, int timeout)
{
    const char *cause;
    slmmsg_t *msg;
    int len;
    int rc;
    
    if (s == -1) {
        if (!connect_lockmgr(cause)) {
            return SLM_LOCKMGR_UNAVAIL;
        }
    }
    len = sizeof(slmgst_t) + strlen(lockname) + 1;
    if (!(msg =malloc(len))) {
        return SLM_INTERNALERR;
    }
    msg->noascii = 0;
    msg->pid = getpid();
    msg->version = SLM_VERSION;
    msg->locknamelen = strlen(lockname);
    msg->cmd = SLM_LOCK;
    strcpy((char *) msg + sizeof(slmmsg_t), lockname); /* includes \0 */
    rc = SLM_LOCK_BUSY; /* failure in I/O - no lock */
    if (writen(s, len, (char *) msg)) goto out;
    if (readn(s, len, (char *) msg)) goto out;
    rc = msg->status;
out:
    free(msg);
    return rc;    
}

int
sim_unlock(const char *lockname, int timeout)
{
    const char *cause;
    slmmsg_t *msg;
    int len;
    int rc;
    
    if (s == -1) {
        if (!connect_lockmgr(cause)) {
            return SLM_LOCKMGR_UNAVAIL;
        }
    }
    len = sizeof(slmgst_t) + strlen(lockname) + 1;
    if (!(msg =malloc(len))) {
        return SLM_INTERNALERR;
    }
    msg->noascii = 0;
    msg->pid = getpid();
    msg->version = SLM_VERSION;
    msg->locknamelen = strlen(lockname);
    msg->cmd = SLM_UNLOCK;
    strcpy((char *) msg + sizeof(slmmsg_t), lockname); /* includes \0 */
    rc = SLM_LOCKMGR_UNAVAIL; /* failure in I/O - no lock */
    if (writen(s, len, (char *) msg)) goto out;
    if (readn(s, len, (char *) msg)) goto out;
    rc = msg->status;
out:
    free(msg);
    return rc;
    
}


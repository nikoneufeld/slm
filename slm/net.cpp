//
//  net.cpp
//  slm
//
//  Created by Niko Neufeld on 8/16/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#include "net.h"
#include "lock.h"
#include "slm.h"
#include "json.h"
#include <map>
#include <utility>
#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
using namespace std;
typedef pair<int, u_int8_t *> iobuffer;

map<int, iobuffer> pending_writes;

int
writen(int fd, int n, u_int8_t *buf)
{
    int i = 0;
    do {
        ssize_t rc = write(fd, buf + n, n - i);
        if (rc == -1) {
            perror("write");
            return -1;
        }
        if (rc == 0) break; // EOF received
        i += rc;
    } while (i < n);
    if (i != n) {
        fprintf(stderr, "WARNING: message to fd %d truncated.\n", fd);
        return -1;
    }
    return 0;
}
// send message to fd
// delete msg after send
// msg will be NULL after send

void
write_req(int fd)
{
    iobuffer buf = pending_writes[fd];
    writen(fd, buf.first, buf.second);
    delete(buf.second);
    
    pending_writes.erase(fd);
}

void
enqueue_answer(int fd, u_int8_t *buf, int len)
{
    pending_writes[fd] = iobuffer(len, buf);
}
    
void
dequeue_answer(int fd)
{
    delete pending_writes[fd].second;
    pending_writes.erase(fd);
}
// verify the pid of the command with the pid associated with this socket
// if no pid yet associated set it
// returns 0 on match; -1 on mismatch
int
check_pid(int fd, slmmsg_t *msg)
{
    pid_t pid = owners[fd]->pid;
    if (pid == -1) {
        owners[fd]->pid = pid;
        return 0;
    }
    if (pid == msg->pid) return 0;
    fprintf(stderr, "PID mismatch for socket %d - got %d - expected %d." \
            "Will ignore command.\n", fd, msg->pid, pid);
    return -1;
}

int
readn(int fd, int n, char *buf)
{
    int i = 0;
    do {
        size_t rc = read(fd, buf + i, n - i);
        if (rc == -1) {
            perror("read");
            return -1;
        }
        if (rc == 0) break; // EOF (socket closed in the middle of msg)?
        i += rc;
    } while (i < n);
    if (i != n) {
        fprintf(stderr, "EOF while reading from %d. Message ignored.\n", fd);
        return -1;
    }
    return 0;
}

int
read_lockname(int fd, slmmsg_t *msg, string *lockname)
{
    lockname->reserve(msg->locknamelen);
    return (readn(fd, msg->locknamelen, (char *) lockname->data()));
}

void
make_answer(SLM_STATUS status, slmmsg_t *msg, string *lockname, u_int8_t *buf,
            int *len)
{
    *len = sizeof(slmmsg_t) + lockname->size() + 1;
    buf = new u_int8_t[*len];
    memcpy(buf, msg, sizeof(slmmsg_t));
    memcpy(buf + sizeof(slmmsg_t), lockname->data(), lockname->size());
    slmmsg_t *hdr = (slmmsg_t *) buf;
    hdr->status = status;
}

SLM_STATUS
process_request(int fd, slmmsg_t *msg, string *lockname)
{
    switch (msg->cmd) {
        case SLM_LOCK:
            return lock(lockname, fd);
        case SLM_UNLOCK:
            return unlock(lockname, fd);
        default:
            fprintf(stderr, "Received illegal command: %d. Ignoring message.\n",
                    msg->cmd);
            return SLM_UNKNOWN_CMD;
    }
    
}

void
read_req(int fd)
{
    int i = 0;
    slmmsg_t msg;
    u_int8_t *buf;
    int len;
    
    if (readn(fd,  sizeof(msg), (char *) &msg)) return;
    if (msg.noascii != 0) {
        json_request(fd, &msg, buf, &len);
    } else {
        if (check_pid(fd, &msg)) return;
        string *lockname = new string;
        if (read_lockname(fd, &msg, lockname)) return; // any error
        SLM_STATUS rc;
        if ((rc = process_request(fd, &msg, lockname)) ==
            SLM_UNKNOWN_CMD) {
            delete lockname;
            return;
        }
        make_answer(rc, &msg, lockname, buf, &len);
        delete lockname;
    }
    enqueue_answer(fd, buf, len);
}

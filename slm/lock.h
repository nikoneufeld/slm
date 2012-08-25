//
//  lock.h
//  slm
//
//  Created by Niko Neufeld on 8/16/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#ifndef __slm__lock__
#define __slm__lock__
#include <list>
#include <map>
#include <sys/socket.h>
#include "slm.h"

using namespace std;

struct lock_t {
    list<int> queue;
    ~lock_t() {
        queue.clear();
    }
};

struct owner_t {
    list<lock_t*> locks;
    pid_t pid;
    struct sockaddr *saddr;
    socklen_t addrlen;
    owner_t(struct sockaddr *new_saddr, int new_addrlen, pid_t new_pid) {
        saddr = (struct sockaddr *) new char[addrlen];
        addrlen = new_addrlen;
        pid = new_pid;
    }
    ~owner_t() {
        locks.clear();
        delete saddr;
    }
};

SLM_STATUS lock(string *lockname, int fd);
SLM_STATUS unlock(string *lockname, int fd);

extern map<int, owner_t * > owners;
extern map<string, lock_t * > locks;

#endif /* defined(__slm__lock__) */

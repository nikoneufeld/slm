//
//  lock.cpp
//  slm
//
//  Created by Niko Neufeld on 8/16/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#include "lock.h"
#include <map>
#include <list>
#include <sys/socket.h>
#include <sys/types.h>

using namespace std;

map<int, owner_t * > owners;
map<string, lock_t * > locks;

// return 0 on lock got
// return 1 on lock owned by someone else
// return 2 on lock owned by calling process
SLM_STATUS
lock(string *lockname, int fd)
{
    if (locks.find(*lockname) != locks.end()) {
        lock_t *lock = locks[*lockname];
        if (lock->queue.front() == fd) return SLM_LOCK_AGAIN;
        lock->queue.push_back(fd);
        return SLM_LOCK_BUSY;
    }
    lock_t *newlock = new lock_t;
    locks[*lockname] = newlock;
    owners[fd]->locks.push_back(newlock);
    return SLM_LOCK_GRANTED;
}

// return 0 on release
// return 1 on not owned
// return 2 on lock does not exist
SLM_STATUS
unlock(string *lockname, int fd)
{
    if (locks.find(*lockname) == locks.end()) return SLM_LOCK_UNLOCKED;
    lock_t *lock = locks[*lockname];
    if (lock->queue.front() != fd) return SLM_LOCK_BUSY;
    owners[fd]->locks.remove(lock);
    locks.erase(*lockname);
    delete lock;
    return SLM_LOCK_UNLOCKED;
}

// adds a new owner identified by socket descriptor and address
// no check will be done if the owner already exists
void
add_owner(int fd, struct sockaddr *saddr, socklen_t addrlen, pid_t pid)
{
    owner_t *new_owner = new owner_t(saddr, addrlen, pid);
    owners[fd] = new_owner;
}

// remove an owner identified by socket descriptor
void
remove_owner(int fd)
{
    delete owners[fd];
    owners.erase(fd);
}
// Local Variables:
// c-basic-offset: 4
// End:

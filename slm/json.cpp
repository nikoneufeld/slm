//
//  json.cpp
//  slm
//
//  Created by Niko Neufeld on 8/18/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#include "json.h"
#include "net.h"
#include "slm.h"
#include "lock.h"
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <map>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;
// 
// add a chunk according to HTTP 1.1 specs
//
void 
add_chunk(const string &chunk, string &answer)
{
    char *buf;
    asprintf(&buf, "%lx", chunk.size());
    answer += buf;
    free(buf);
    answer += "\r\n" + chunk + "\r\n";        
}

void 
add_chunk(int number, string &answer)
{
    char *buf;
    asprintf(&buf, "%d", number);
    add_chunk(buf, answer);
    free(buf);
}

void 
add_owner(lock_t *lock, string &page)
{
    int fd = (lock->queue).front();
    owner_t *owner = owners[fd];
    int hostlen = 128;
    char host[hostlen + 2];
    getnameinfo(owner->saddr, owner->addrlen, &host[1], hostlen, NULL, 0, 
                NI_NOFQDN);
    host[0] = '"';
    host[hostlen] = '"';
    add_chunk("{", page);
    add_chunk("\"hostname\"", page);
    add_chunk(": ", page);
    add_chunk(host, page);
    add_chunk(", ", page);
    add_chunk("\"pid\"", page);
    add_chunk(": ", page);
    add_chunk(owner->pid, page);
    add_chunk("}", page);
}
    
void 
json_list_locks(string &page)
{
    page = "HTTP/1.1 200 OK\r\n";
    page += "Content-Type: application/json\r\n";
    page += "Connection: close\r\n";
    page += "Transfer-Encoding: chunked\r\n";
    page += "\r\n";
    add_chunk("{", page);
    for (map<string, lock_t *>::iterator it = locks.begin(); it != locks.end();
         ++it) {
        add_chunk(it->first, page);
        add_chunk(": ", page);
        add_owner(it->second, page);
        if (it != --locks.end()) {
            add_chunk(",", page);
        }
    
    }
    add_chunk("", page);
    page += "\r\n";
}

void
error404(string &page)
{
    page =  "HTTP/1.1 404 Not Found\r\n";
    page += "Content-Type: text/html; charset=UTF-8";
    page += "Connection: close\r\n";
    page += "\r\n";
}
// no attempt at proper Http / JSON parsing done here
// just look for GET and its argument(s)
void
json_request(int fd, u_int8_t *msg, u_int8_t *buf, int *len)
{
    string request;
    bool foundcr = false;
    bool foundeol = false;
    for (int i = 0; i < sizeof(slmmsg_t); ++i) {
        if (msg[i] == '\n' && foundcr) {
            foundeol = true;
            break;
        }
        foundcr = (msg[i] == '\r');
        request.push_back(msg[i]);
    }
    if (foundeol) {
        request.erase(request.end()); // delete trailing cr
    } else {
        for (;;) {
            char c;
            int rc = read(fd, c, 1);
            if (rc == 0) break;
            if (rc == -1) break;
            if (c == '\n' && foundcr) {
                foundeol = true;
                request.erase(request.end());
                break;
            }
            foundcr = (c == '\r');
            request.push_back(c);
        }
    }
    if (!foundeol) error404;
    //
    int i = request.find("GET");
    if (i == -1) error404;
    int j = request.find(' ', i);
    if (j == -1) error404;
    i = request.find_first_not_of(' ', j);
    j = request.find_first_of(' ', i);
    string para = request.substr(i,j)
    // decode
    if para.compare("listlocks") json_list_locks();
    error404;
}
        
    
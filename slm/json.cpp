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
#include <cstring>
#include <string>
using namespace std;

void json_list_locks(string &page) {
    page = "<html>\r\n";
    page += "<head>\r\n";
    page += "</head>\r\n";
    
    page += "</html>\r\n";
    
}
json_decode();

json_answer();
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
        
    
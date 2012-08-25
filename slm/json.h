//
//  json.h
//  slm
//
//  Created by Niko Neufeld on 8/18/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#ifndef __slm__json__
#define __slm__json__
#include <string>
using namespace std;

enum SLM_JSON_CMD {
    SLM_LIST_LOCKS,
    SLM_RELEASE_LOCK,
    SLM_RELEASE_ALLLOCKS
};
void json_request(int fd, char *msg, u_int8_t *buf, int *len);
SLM_JSON_CMD decode_json(string *msg, string paramenters[]);

#endif /* defined(__slm__json__) */

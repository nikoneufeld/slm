//
//  slm.h
//  slm
//
//  Created by Niko Neufeld on 8/16/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#ifndef slm_slm_h
#define slm_slm_h
#include <sys/types.h>

typedef struct slmmsg {
    u_int16_t noascii;  // fixed to 0
    u_int8_t version;
    union {
        u_int8_t cmd;
        u_int8_t status;
    };
    u_int32_t pid;
    u_int32_t locknamelen;
    //  u_int8_t lockname[]; // placeholder (not allowed by C99 (and C++))
} __attribute__((packed)) slmmsg_t;

enum SLM_CMD {
    SLM_LOCK,   // exclusive unlock
    SLM_UNLOCK  // unconditional release
};
enum SLM_STATUS {
    SLM_LOCK_GRANTED, // exclusive lock
    SLM_LOCK_BUSY,    // exclusive lock
    SLM_LOCK_AGAIN,   // lock already held by calling process
    SLM_LOCK_UNLOCKED, // lock released
    SLM_LOCKMGR_UNAVAIL, // lockmanager cannot be reached
    SLM_LOCK_INTERNALERR,  // internal error (out of memory etc...)
    SLM_UNKNOWN_CMD
};

extern "C" {
    /* timeout in ms, -1 wait forever
       returns
       0: lock granted
       1: lock busy
       2: lock already held
       3: lock manager unavailable
       4: timeout
     */
    int slm_lock(const char *lockname, int timeout);
    /* timeout in ms, -1 wait forever
        returns
        0: lock released
        1: lock not owned
        2: lock does not exist
        3: lock manager unavailable
        4: timeout
     */
    int slm_unlock(const char *lockname, int timeout);
    // defaults to 127.0.0.1:39999
    int slm_setlockmgr(const char *addr, int port);
}
#endif

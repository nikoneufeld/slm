//
//  net.h
//  slm
//
//  Created by Niko Neufeld on 8/16/12.
//  Copyright (c) 2012 Niko Neufeld. All rights reserved.
//

#ifndef __slm__net__
#define __slm__net__

extern "C" {
    int writen(int fd, int n, char *buf);
    int readn(int fd, int n, char *buf);
}

#endif /* defined(__slm__net__) */

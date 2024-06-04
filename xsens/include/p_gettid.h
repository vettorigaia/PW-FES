#ifndef __P_GETTID_H
#define __P_GETTID_H

    // for pid_t
    #include <sys/types.h>  
    // for priority management
    #include <unistd.h>
    #include <sys/resource.h>
    #include <sys/syscall.h>
    // for PIPE/popen
    #include "pstream.h"    

    pid_t pstream_gettid(uint8_t num_t);

#endif
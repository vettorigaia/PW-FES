#include "p_gettid.h"
#include <inttypes.h>

pid_t pstream_gettid(uint8_t num_t) {

    // Get all the PIDs of the process using 'top'; parse with 'grep'
    redi::ipstream in("top -b -n 1 -p $(pgrep -f [m]tw_driver_imu) -H -o PID 2>/dev/null | grep PID -A 4 | grep -v PID");
    std::string str;

    // Skip the first threads
    for( uint8_t i = 0; i < num_t; ++i ) {
        std::getline(in, str); 
    }
    // Get the thread number num_t (ordered list according to output of top)
    //std::getline(in, str);
    std::getline(in, str);
    
    // Return the PID by splitting the string on the first whitespace (" ")
    return (pid_t) std::stoi( str.substr(0, str.find(" ")));

}
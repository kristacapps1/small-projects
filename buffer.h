/*Buffer header file for multithreaded copy local*/
#include <string>
typedef struct {
    int infd;
    int outfd;
    std::string filename;
} buffer_t;

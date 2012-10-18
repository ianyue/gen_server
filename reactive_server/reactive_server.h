#ifndef __REACTIVE_SERVER
#define __REACTIVE_SERVER

namespace net {
class ReactiveServer {

public:
    ReactiveServer() {}

    int run(int);
    
    void processData(void *buffer, int len);

private:

};
}

#endif

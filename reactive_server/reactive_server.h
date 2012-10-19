#ifndef __REACTIVE_SERVER
#define __REACTIVE_SERVER

namespace net {
class ReactiveServer {

public:
    ReactiveServer() {}

    int run(int);

protected:

    virtual int processRawData(int sock_fd, void *buffer, size_t len, size_t &recv_len, int flags);

    virtual void processData(void *buffer, int len);


private:

};
}

#endif

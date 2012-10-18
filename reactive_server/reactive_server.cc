#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <memory.h>
#include <iostream>

#include "reactive_server.h"
#include "gen_head/GenHead.h"

#ifndef NET_DEBUG
#define NET_DEBUG 1
#endif

using namespace std;

int net::ReactiveServer::run(int port) {
    struct sockaddr_in servaddr;
    int listen_fd, connect_fd, max_fd = 0;
    int sock_fds[FD_SETSIZE];
    
    struct fd_set monitor_fds, read_fds;
    struct timeval timeout = {3, 0}; // TODO read from conf
    
    char buffer[1024]; // TODO buffer size
    //int port = 18321; // TODO read from conf

    for(int i = 0; i < FD_SETSIZE; i++) sock_fds[i] = -1;

    // create socket using default protocol
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        // print err log
        cerr << "create socket failed" << endl;
        return -1;
    }

    // initial servaddr
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        cerr << "bind addr failed" << endl;
        return -1;
    }
    
    // TODO read from conf
    int backlog = 10;

    if(listen(listen_fd, backlog) == -1) {
        cerr << "listen failed" << endl;
        return -1;
    }

    if(NET_DEBUG) {
        cout << "server start, listen on port: " << port << ", fd: " << listen_fd << endl;
    }
    // clear monitor_fds
    FD_ZERO(&monitor_fds);
    // add listen socket fd to monitor_fds
    FD_SET(listen_fd, &monitor_fds);
    
    int max_index = 0; // indicate the max index of sock_fds, optimize
    max_fd = max(max_fd, listen_fd);
    while(1) {
        read_fds = monitor_fds;

        int ret = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if(0 == ret) {
            continue;
        } else if(ret < 0) {
            cerr << "======select failed======" << endl;
        }

        if(FD_ISSET(listen_fd, &read_fds)) {
            // TODO use while
            // TODO use sockaddr
            if(NET_DEBUG) cout << "begin to accept" << endl;

            sleep(1);
            if((connect_fd = accept(listen_fd, (struct sockaddr *)NULL, NULL)) == -1) {
                // TODO check errno, (non-block accept)
                // non-block timeout
                if(EAGAIN == errno || EWOULDBLOCK == errno) {
                    // break
                    
                } else {
                    cerr << "accept failed" << endl;
                    continue;
                }
            }

            if(NET_DEBUG) {
                cout << "accept from client, fd: " << connect_fd << endl;
            }
            for(int i = 0; i < FD_SETSIZE; i++) {
                if(-1 == sock_fds[i]) {
                    sock_fds[i] = connect_fd;
                    
                    if(i > max_index) max_index = i;
                    break;
                }
            }
            // TODO if(i == FD_SETSIZE)  how to process the new socket ?
            // TODO close(connect_fd), active close will cause TIME_WAIT
            // TODO if do nothing, will consume server fd resource.
            FD_SET(connect_fd, &monitor_fds);
            max_fd = max(connect_fd, max_fd);

            // TODO end while

            // clear listen_fd flag
            FD_CLR(listen_fd, &read_fds);
            // check ret, use ret to reduce unused loop
            if(--ret <= 0) continue;
        }


        for(int i = 0; i <= max_index; i++) {
            if(sock_fds[i] < 0) continue; // invalid sock

            if(FD_ISSET(sock_fds[i], &read_fds)) {
                // process data
                net::GenHead head;
                size_t recv_len;
                int r_ret = head.readFromSock(sock_fds[i], (void *) buffer, 1024, recv_len, 0); // TODO buffer size


                // if socket close, should fd clr socket from monitor_fds
                if(0 == r_ret) {
                    if(NET_DEBUG) {
                        cout << "close socket fd: " << sock_fds[i] << endl;
                    }
                    // current tcp status is CLOSE_WAIT
                    close(sock_fds[i]);
                    // clear sock
                    FD_CLR(sock_fds[i], &monitor_fds);
                    sock_fds[i] = -1;
                    goto end;
                } else if(r_ret < 0) {
                    // non-block timeout
                    if(errno != EAGAIN) {
                        // TODO how to handle with socket error

                    }

                    goto end;
                }

                if(NET_DEBUG) {
                    cout << "receive data from fd: " << sock_fds[i] << endl;
                }

                // process data
                processData(buffer, recv_len);

            end:
                if(--ret <= 0) break;
            }// end if (FD_ISSET)
        } // end for (max_index)
        
    } // end while(1)
    close(listen_fd);
}

void net::ReactiveServer::processData(void *buffer, int len) {
    cout << "process data..." << endl;
    
}


int main()
{
    net::ReactiveServer server;
    server.run(17321);
    return 0;
}

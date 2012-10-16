#include <endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory.h>

#include "GenHead.h"


bool net::GenHead::writeToBuffer(void *buffer, uint32_t len, uint32_t& wlen) const {

    if(NULL == buffer) {
        return false;
    }

    if(len < sizeof(this->_head_t)) {
        return false;
    }
    // convert to big endian(network)
    uint16_t _b_version = htobe16(_head_t.version);
    uint32_t _b_req_id = htobe32(_head_t.req_id);
    uint32_t _b_req_ip = htobe32(_head_t.req_ip);
    uint32_t _b_timestamp = htobe32(_head_t.timestamp);
    uint32_t _b_body_length = htobe32(_head_t.body_length);

    char *c_buf = (char *)buffer;
    // pack GenHead
    // pack version
    *((uint16_t *)c_buf) = _b_version;
    c_buf += sizeof(_b_version);
    // pack req_id
    *((uint32_t *)c_buf) = _b_req_id;
    c_buf += sizeof(_b_req_id);
    // pack req_ip
    *((uint32_t *)c_buf) = _b_req_ip;
    c_buf += sizeof(_b_req_ip);
    // pack timestamp
    *((uint32_t *)c_buf) = _b_timestamp;
    c_buf += sizeof(_b_timestamp);
    // pack body_length
    *((uint32_t *)c_buf) = _b_body_length;
    c_buf += sizeof(_b_body_length);

    wlen = c_buf - (char *)buffer;

    return true;
}

bool net::GenHead::readFromBuffer(void *buffer, uint32_t len) {
    if(NULL == buffer) {
        return false;
    }

    if(len != sizeof(_head_t)) {
        return false;
    }

    // unpack GenHead
    char *c_buf = (char *)buffer;
    // unpack version
    _head_t.version = *((uint16_t *)c_buf);
    c_buf += sizeof(_head_t.version);
    // unpack req_id
    _head_t.req_id = *((uint32_t *)c_buf);
    c_buf += sizeof(_head_t.req_id);
    // unpack req_ip
    _head_t.req_ip = *((uint32_t *)c_buf);
    c_buf += sizeof(_head_t.req_ip);
    // unpack timestamp
    _head_t.timestamp = *((uint32_t *)c_buf);
    c_buf += sizeof(_head_t.timestamp);
    // unpack body_length
    _head_t.body_length = *((uint32_t *)c_buf);
    c_buf + sizeof(_head_t.body_length);

    _head_t.version = be16toh(_head_t.version);
    _head_t.req_id = be32toh(_head_t.req_id);
    _head_t.req_ip = be32toh(_head_t.req_ip);
    _head_t.timestamp = be32toh(_head_t.timestamp);
    _head_t.body_length = be32toh(_head_t.body_length);
    
    return true;
}

// TODO check flags usage
bool net::GenHead::writeToSock(int sock, void *buffer, size_t len, int flags) {
    
    if(NULL == buffer) {
        return false;
    }

    if(len <= 0) {
        return false;
    }

    _head_t.body_length = len;

    char *s_buf = new char[sizeof(_head_t) + len];
    size_t r_len, left_bytes, total_send_bytes;
    
    if(writeToBuffer(s_buf, sizeof(_head_t), r_len) == false) {
        goto failed;
    }

    if(r_len != sizeof(_head_t)) {
        // pack head error
        goto failed;
    }

    memcpy(s_buf + sizeof(_head_t), buffer, len);
    left_bytes = sizeof(_head_t) + len;
    total_send_bytes = 0;

    while(left_bytes > 0) {

        size_t send_bytes = send(sock, s_buf + total_send_bytes, left_bytes, flags);
        if(send_bytes < 0) {
            goto failed;
        }

        left_bytes -= send_bytes;
        total_send_bytes += send_bytes;

    }
    delete[] s_buf;
    return true;

failed:
    delete[] s_buf;
    return false;
}

// TODO timeout
bool net::GenHead::readFromSock(int sock, void *buffer, size_t len, size_t &rlen, int flags) {

    if(NULL == buffer) {
        return false;
    }

    if(len <= 0) {
        return false;
    }

    char head_buf[sizeof(_head_t)];

    size_t left_bytes = sizeof(_head_t);
    size_t total_recv_bytes = 0;

    while(left_bytes > 0) {
        size_t recv_bytes = recv(sock, head_buf + total_recv_bytes, left_bytes, flags);
        if(recv_bytes < 0) {
            return false;
        }

        left_bytes -= recv_bytes;
        total_recv_bytes += recv_bytes;
    }

    if(readFromBuffer(head_buf, sizeof(_head_t)) ==  false) {
        return false;
    }

    if(_head_t.body_length < 0) {
        
    }

    if(len < _head_t.body_length) {
        // buffer too small
        return false;
    }
    left_bytes = _head_t.body_length;
    total_recv_bytes = 0;
    
    while(left_bytes > 0) {
        size_t recv_bytes = recv(sock, (char *)buffer + total_recv_bytes, left_bytes, flags);
        if(recv_bytes < 0) {
            return false;
        }

        left_bytes -= recv_bytes;
        total_recv_bytes += recv_bytes;
    }

    rlen = _head_t.body_length;
    return true;
}


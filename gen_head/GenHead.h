#ifndef __GEN_HEAD
#define __Gen_HEAD

#include <stdint.h>

namespace net {

#pragma pack(1)

/* GenHead structure definition */
typedef struct _gen_head_t {
    uint16_t version;      /* head version */
    uint32_t req_id;       /* request id */
    uint32_t req_ip;       /* request ip */
    uint32_t timestamp;    /* request timestamp */
    uint32_t body_length;  /* body length */
} gen_head_t;

#pragma pack()

class GenHead {

public:
    GenHead() {
        _head_t.version = 0;
        _head_t.req_id = 0;
        _head_t.req_ip = 0;
        _head_t.timestamp = 0;
        _head_t.body_length = 0;
    }

    GenHead(uint16_t version, uint32_t req_id, uint32_t req_ip, uint32_t timestamp, uint32_t body_length) {
          _head_t.version = version;
          _head_t.req_id = req_id;
          _head_t.req_ip = req_ip;
          _head_t.timestamp = timestamp;
          _head_t.body_length = body_length;
    }

    /* setter method */
    void setVersion(uint16_t version) { _head_t.version = version; }

    void setReqId(uint32_t req_id) { _head_t.req_id = req_id; }

    void setReqIP(uint32_t req_ip) { _head_t.req_ip = req_ip; }

    void setTimestamp(uint32_t timestamp) { _head_t.timestamp = timestamp; }

    void setBodyLength(uint32_t body_length) { _head_t.body_length = body_length; }

    /* getter method */
    uint16_t getVersion() const { return _head_t.version; }

    uint32_t getReqId() const { return _head_t.req_id; }

    uint32_t getReqIP() const { return _head_t.req_ip; }

    uint32_t getTimestamp() const { return _head_t.timestamp; }

    uint32_t getBodyLength() const { return _head_t.body_length; }

    /* serialize object to buffer with big endian (network) */
    bool writeToBuffer(void *buffer, size_t len, size_t& wlen) const;

    /* read GenHead from string */
    bool readFromBuffer(const void *buffer, size_t len);

    /* write object to tcp socket */
    bool writeToSock(int sock, const void *buffer, size_t len, int flags);

    /* read object from tcp socket */
    int readFromSock(int sock, void *buffer, size_t len, size_t &rlen, int flags);

    static uint32_t headSize() { return sizeof(gen_head_t); }

private:
    
    gen_head_t _head_t;
};
}
#endif

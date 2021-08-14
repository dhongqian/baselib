#pragma once
#ifndef __HQ_BASE_LIB_BUFFER_UTILITY_H__
#define __HQ_BASE_LIB_BUFFER_UTILITY_H__

#include <memory>
#include "Utility.h"

namespace hq {

#define MAX_UDP_MTU  (1501)

class baselib_declspec BufferUtility {
protected:
    uint8_t* buffer_;
    int            data_len_;
    int            max_len_;
    bool           is_need_delete_;

public:
    BufferUtility(const int max_buffer_len=1500);
    BufferUtility(uint8_t *data, const uint8_t data_len);
    virtual ~BufferUtility();

    uint8_t* getBuffer();
    uint8_t*getData();
    int   getDataLen();
    void  setDataLen(const int data_len);
    int   getBufferMaxLen();
    void reset();
};

typedef std::shared_ptr<BufferUtility>  BufferUtilityPtr;

};
#endif   ///< __HQ_BASE_LIB_BUFFER_UTILITY_H__

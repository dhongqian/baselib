#include "baselib/BufferUtility.h"

namespace hq {

BufferUtility::BufferUtility(const int max_buffer_len): max_len_(max_buffer_len), data_len_(0), is_need_delete_(true) {
    if (max_buffer_len > 0) {
        buffer_ = new uint8_t[max_buffer_len];
        if (nullptr == buffer_) max_len_ = 0;
        reset();
    } else {
        buffer_ = nullptr;
        max_len_ = 0;
    }
}

BufferUtility::BufferUtility(uint8_t *data, const uint8_t data_len): buffer_(data), data_len_(data_len), max_len_(data_len), is_need_delete_(false) {

}

BufferUtility::~BufferUtility() {
    if(nullptr != buffer_ && is_need_delete_) {
        delete[]buffer_;
        buffer_ = nullptr;
    }
}

uint8_t* BufferUtility::getBuffer() {
    return buffer_;
}

uint8_t* BufferUtility::getData() {
    return buffer_;
}

int BufferUtility::getDataLen() {
    return data_len_;
}

void BufferUtility::setDataLen(const int data_len) {
    data_len_ = data_len;
}

int BufferUtility::getBufferMaxLen() {
    return max_len_;
}

void BufferUtility::reset() {
    if (nullptr != buffer_) {
        memset(buffer_, 0, max_len_);
    }
}

}

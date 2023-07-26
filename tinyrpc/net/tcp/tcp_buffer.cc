#include <memory>
#include "tcp_buffer.h"
#include "string.h"
#include "tinyrpc/common/log.h"

namespace tinyrpc {

TcpBuffer::TcpBuffer(int size) :size_(size) {
    buffer_.resize(size);
}

TcpBuffer::~TcpBuffer() {

}

int TcpBuffer::readAble() {
    return write_idx_ - read_idx_;
}

int TcpBuffer::writeAble() {
    return buffer_.size() - write_idx_;
}

int TcpBuffer::readIndex() {
    return read_idx_;
}

int TcpBuffer::writeIndex() {
    return write_idx_;
}

void TcpBuffer::resizeBuffer(int new_size) {
    std::vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());

    memcpy(&tmp[0], &buffer_[read_idx_], count);
    buffer_.swap(tmp);

    read_idx_ = 0;
    write_idx_ = read_idx_ + count;
}

void TcpBuffer::adjustBuffer() {
    if (read_idx_ < static_cast<int>(buffer_.size() / 3))
        return;

    std::vector<char> buffer(buffer.size());
    int count = readAble();

    memcpy(&buffer[0], &buffer_[read_idx_], count);
    // buffer_ = std::move(buffer);
    buffer_.swap(buffer);
    read_idx_ = 0;
    write_idx_ = read_idx_ + count;

    buffer.clear();
}

void TcpBuffer::writeToBuffer(const char *buf, int size) {
    if (size > writeAble()) {
        int new_size = static_cast<int>(1.5 * (write_idx_ + size));
        resizeBuffer(new_size);
    }

    memcpy(&buffer_[write_idx_], buf, size);
    write_idx_ += size;
}

void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
    if (readAble() == 0)
        return;

    int read_size = readAble() > size ? size : readAble();

    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &buffer_[read_idx_], read_size);

    // re = std::move(tmp);
    re.swap(tmp);
    read_idx_ += read_size;

    adjustBuffer();
}

void TcpBuffer::moveReadIndex(int size) {
    size_t moved = read_idx_ + size;
    if (moved >= buffer_.size()) {
        ERRORLOG("TcpBuffer::moveReadIndex() error, invalid size %d, old_read_index %d, buffer size %d", size, read_idx_, buffer_.size());
        return;
    }
    read_idx_ = moved;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size) {
    size_t moved = write_idx_ + size;
    if (moved >= buffer_.size()) {
        ERRORLOG("TcpBuffer::moveWriteIndex() error, invalid size %d, old_write_index %d, buffer size %d", size, write_idx_, buffer_.size());
        return;
    }
    write_idx_ = moved;
    adjustBuffer();
}

}
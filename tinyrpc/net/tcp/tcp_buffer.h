#pragma once

#include <vector>
#include <memory>

namespace tinyrpc {

class TcpBuffer {
public:
    using s_ptr = std::shared_ptr<TcpBuffer>;

    TcpBuffer(int size);

    ~TcpBuffer();

public:
    int readAble();

    int writeAble();

    int readIndex();

    int writeIndex();

    void resizeBuffer(int new_size);

    void adjustBuffer();

    void writeToBuffer(const char* buf, int size);

    void readFromBuffer(std::vector<char>& re, int size);

    void moveReadIndex(int size);

    void moveWriteIndex(int size);

private:
    int read_idx_ {0};
    int write_idx_ {0};
    int size_ {0};

public:
    std::vector<char> buffer_;
};

}  // namespace tinyrpc
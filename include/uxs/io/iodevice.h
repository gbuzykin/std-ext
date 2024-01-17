#pragma once

#include "iostate.h"

#include "uxs/span.h"

namespace uxs {

enum class iodevcaps : unsigned { none = 0, rdonly = 1, mappable = 2 };
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(iodevcaps, unsigned);

class iodevice {
 public:
    iodevice() = default;
    explicit iodevice(iodevcaps caps) : caps_(caps) {}
    virtual ~iodevice() = default;
    iodevice(const iodevice&) = delete;
    iodevice& operator=(const iodevice&) = delete;
    iodevcaps caps() const { return caps_; }
    virtual int read(void* data, size_t sz, size_t& n_read) = 0;
    virtual int write(const void* data, size_t sz, size_t& n_written) = 0;
    virtual void* map(size_t& sz, bool wr) { return nullptr; }
    virtual int64_t seek(int64_t off, seekdir dir) { return -1; }
    virtual int ctrlesc_color(uxs::span<const uint8_t> v) { return -1; }
    virtual int flush() = 0;

 private:
    iodevcaps caps_ = iodevcaps::none;
};

}  // namespace uxs

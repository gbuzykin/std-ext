#include "util/io/iobuf_impl.h"

namespace util {
iomode detail::iomode_from_str(const char* mode, iomode def) {
    iomode result = def;
    while (*mode) {
        switch (*mode) {
            case 'r': {
                result |= iomode::kIn;
                if (*(mode + 1) == '+') { result |= iomode::kOut, ++mode; }
            } break;
            case 'w': {
                result |= iomode::kOut | iomode::kCreate;
                if (*(mode + 1) == '+') { result |= iomode::kIn, ++mode; }
            } break;
            case 'a': {
                result |= iomode::kOut | iomode::kCreate | iomode::kAppend;
                if (*(mode + 1) == '+') { result |= iomode::kIn, ++mode; }
            } break;
            case 'x': result |= iomode::kOut | iomode::kCreate | iomode::kExcl; break;
            case 't': result |= iomode::kText; break;
            case 'b': result &= ~iomode::kText; break;
            default: break;
        }
        ++mode;
    }
    return result;
}

template class basic_iobuf<char>;
template class basic_iobuf<wchar_t>;
template class basic_iobuf<uint8_t>;
}  // namespace util

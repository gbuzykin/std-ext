#include "uxs/impl/io/zipfilebuf_impl.h"

namespace uxs {
template class basic_zipfilebuf<char>;
template class basic_zipfilebuf<wchar_t>;
template class basic_zipfilebuf<std::uint8_t>;
}  // namespace uxs

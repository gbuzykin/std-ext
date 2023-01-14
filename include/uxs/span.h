#pragma once

#include "iterator.h"

#include <stdexcept>

namespace uxs {

template<typename Ty>
class span {
 public:
    using value_type = std::remove_cv_t<Ty>;
    using pointer = Ty*;
    using const_pointer = const Ty*;
    using reference = Ty&;
    using const_reference = const Ty&;
    using iterator = uxs::array_iterator<span, false>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    CONSTEXPR span() NOEXCEPT {}
    template<typename Ty2, typename = std::enable_if_t<std::is_convertible<Ty2*, Ty*>::value>>
    CONSTEXPR span(Ty2* v, size_type count) NOEXCEPT : begin_(v), size_(count) {}
    template<typename Ty2, size_t N, typename = std::enable_if_t<std::is_convertible<Ty2*, Ty*>::value>>
    explicit CONSTEXPR span(Ty2 (&v)[N]) NOEXCEPT : begin_(v), size_(N) {}
    template<typename Range, typename = std::enable_if_t<is_contiguous_range<std::remove_reference_t<Range>, Ty>::value>>
    CONSTEXPR span(Range&& r) NOEXCEPT : begin_(r.data()), size_(r.size()) {}

    CONSTEXPR size_type size() const NOEXCEPT { return size_; }
    CONSTEXPR bool empty() const NOEXCEPT { return size_ == 0; }
    CONSTEXPR pointer data() const NOEXCEPT { return begin_; }

    CONSTEXPR iterator begin() const NOEXCEPT { return iterator{begin_, begin_, begin_ + size_}; }
    CONSTEXPR iterator end() const NOEXCEPT { return iterator{begin_ + size_, begin_, begin_ + size_}; }

    CONSTEXPR reverse_iterator rbegin() const NOEXCEPT { return reverse_iterator{end()}; }
    CONSTEXPR reverse_iterator rend() const NOEXCEPT { return reverse_iterator{begin()}; }

    CONSTEXPR reference operator[](size_type pos) const {
        assert(pos < size_);
        return begin_[pos];
    }
    reference at(size_type pos) const {
        if (pos >= size_) { throw std::out_of_range("invalid pos"); }
        return begin_[pos];
    }
    CONSTEXPR reference front() const {
        assert(size_ > 0);
        return begin_[0];
    }
    CONSTEXPR reference back() const {
        assert(size_ > 0);
        return *(begin_ + size_ - 1);
    }

    CONSTEXPR span subspan(size_type offset, size_type count = dynamic_extent) const {
        offset = std::min(offset, size_);
        return span(begin_ + offset, std::min(count, size_ - offset));
    }

 private:
    Ty* begin_ = nullptr;
    size_t size_ = 0;
};

template<typename Ty>
CONSTEXPR span<Ty> as_span(Ty* v, typename span<Ty>::size_type count) NOEXCEPT {
    return span<Ty>(v, count);
}

template<typename Ty, size_t N>
CONSTEXPR span<Ty> as_span(Ty (&v)[N]) NOEXCEPT {
    return span<Ty>(v, N);
}

template<typename Range>
CONSTEXPR auto as_span(Range&& r) NOEXCEPT -> span<std::remove_pointer_t<decltype(r.data() + r.size())>> {
    return span<std::remove_pointer_t<decltype(r.data())>>(r.data(), r.size());
}

}  // namespace uxs

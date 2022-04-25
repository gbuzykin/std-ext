#include "util/stringalg.h"

namespace util {

std::wstring from_utf8_to_wide(std::string_view s) {
    uint32_t code;
    std::wstring result;
    result.reserve(s.size());
#if defined(WCHAR_MAX) && WCHAR_MAX > 0xffff
    for (auto p = s.begin(); from_utf8(p, s.end(), p, code) != 0;) { result.push_back(static_cast<wchar_t>(code)); }
#else   // define(WCHAR_MAX) && WCHAR_MAX > 0xffff
    for (auto p = s.begin(); from_utf8(p, s.end(), p, code) != 0;) { to_utf16(code, std::back_inserter(result)); }
#endif  // define(WCHAR_MAX) && WCHAR_MAX > 0xffff
    return result;
}

std::string from_wide_to_utf8(std::wstring_view s) {
    std::string result;
    result.reserve(s.size());
#if defined(WCHAR_MAX) && WCHAR_MAX > 0xffff
    for (auto p = s.begin(); p != s.end(); ++p) { to_utf8(static_cast<uint32_t>(*p), std::back_inserter(result)); }
#else   // define(WCHAR_MAX) && WCHAR_MAX > 0xffff
    uint32_t code;
    for (auto p = s.begin(); from_utf16(p, s.end(), p, code) != 0;) { to_utf8(code, std::back_inserter(result)); }
#endif  // define(WCHAR_MAX) && WCHAR_MAX > 0xffff
    return result;
}

// --------------------------

template<typename CharT>
std::basic_string_view<CharT> basic_trim_string(std::basic_string_view<CharT> s) {
    auto p1 = s.begin(), p2 = s.end();
    while (p1 != p2 && is_space(*p1)) { ++p1; }
    while (p1 != p2 && is_space(*(p2 - 1))) { --p2; }
    return s.substr(p1 - s.begin(), p2 - p1);
}

std::string_view trim_string(std::string_view s) { return basic_trim_string(s); }
std::wstring_view trim_string(std::wstring_view s) { return basic_trim_string(s); }

// --------------------------

std::vector<std::string> unpack_strings(std::string_view s, char sep) {
    std::vector<std::string> result;
    unpack_strings(s, sep, nofunc(), std::back_inserter(result));
    return result;
}

std::vector<std::wstring> unpack_strings(std::wstring_view s, char sep) {
    std::vector<std::wstring> result;
    unpack_strings(s, sep, nofunc(), std::back_inserter(result));
    return result;
}

// --------------------------

template<typename CharT>
std::basic_string<CharT> basic_encode_escapes(std::basic_string_view<CharT> s, std::basic_string_view<CharT> symb,
                                              std::basic_string_view<CharT> code) {
    std::basic_string<CharT> result;
    result.reserve(s.size());
    auto p = s.begin(), p0 = p;
    for (; p != s.end(); ++p) {
        auto pos = symb.find(*p);
        if (pos != std::basic_string_view<CharT>::npos) {
            result.append(p0, p);
            result += '\\';
            result += code[pos];
            p0 = p + 1;
        }
    }
    result.append(p0, p);
    return result;
}

std::string encode_escapes(std::string_view s, std::string_view symb, std::string_view code) {
    return basic_encode_escapes(s, symb, code);
}

std::wstring encode_escapes(std::wstring_view s, std::wstring_view symb, std::wstring_view code) {
    return basic_encode_escapes(s, symb, code);
}

// --------------------------

template<typename CharT>
std::basic_string<CharT> basic_decode_escapes(std::basic_string_view<CharT> s, std::basic_string_view<CharT> symb,
                                              std::basic_string_view<CharT> code) {
    std::basic_string<CharT> result;
    result.reserve(s.size());
    auto p = s.begin(), p0 = p;
    for (; p != s.end(); ++p) {
        if (*p != '\\') { continue; }
        result.append(p0, p);
        p0 = p + 1;
        if (++p == s.end()) { break; }
        auto pos = code.find(*p);
        if (pos != std::basic_string_view<CharT>::npos) {
            result += symb[pos];
            p0 = p + 1;
        }
    }
    result.append(p0, p);
    return result;
}

std::string decode_escapes(std::string_view s, std::string_view symb, std::string_view code) {
    return basic_decode_escapes(s, symb, code);
}

std::wstring decode_escapes(std::wstring_view s, std::wstring_view symb, std::wstring_view code) {
    return basic_decode_escapes(s, symb, code);
}

// --------------------------

std::string make_quoted_text(std::string_view text) {
    std::string s;
    s.reserve(text.size() + 16);
    make_quoted_text(text, std::back_inserter(s));
    return s;
}

std::wstring make_quoted_text(std::wstring_view text) {
    std::wstring s;
    s.reserve(text.size() + 16);
    make_quoted_text(text, std::back_inserter(s));
    return s;
}

// --------------------------

template<typename CharT>
std::pair<unsigned, unsigned> basic_parse_flag_string(
    std::basic_string_view<CharT> s, const std::vector<std::pair<std::basic_string_view<CharT>, unsigned>>& flag_tbl) {
    std::pair<unsigned, unsigned> flags(0, 0);
    separate_words(s, ' ', nofunc(), function_caller([&](std::basic_string_view<CharT> flag) {
                       bool add_flag = (flag[0] != '-');
                       if (flag[0] == '+' || flag[0] == '-') { flag = flag.substr(1); }
                       auto it = std::find_if(flag_tbl.begin(), flag_tbl.end(),
                                              [flag](decltype(*flag_tbl.begin()) el) { return el.first == flag; });
                       if (it == flag_tbl.end()) {
                       } else if (add_flag) {
                           flags.first |= it->second;
                       } else {
                           flags.second |= it->second;
                       }
                   }));
    return flags;
}

std::pair<unsigned, unsigned> parse_flag_string(std::string_view s,
                                                const std::vector<std::pair<std::string_view, unsigned>>& flag_tbl) {
    return basic_parse_flag_string(s, flag_tbl);
}

std::pair<unsigned, unsigned> parse_flag_string(std::wstring_view s,
                                                const std::vector<std::pair<std::wstring_view, unsigned>>& flag_tbl) {
    return basic_parse_flag_string(s, flag_tbl);
}

// --------------------------

template<typename CharT>
int basic_compare_strings_nocase(std::basic_string_view<CharT> lhs, std::basic_string_view<CharT> rhs) {
    auto p1_end = lhs.begin() + std::min(lhs.size(), rhs.size());
    for (auto p1 = lhs.begin(), p2 = rhs.begin(); p1 != p1_end; ++p1, ++p2) {
        char ch1 = to_lower(*p1), ch2 = to_lower(*p2);
        if (std::basic_string_view<CharT>::traits_type::lt(ch1, ch2)) {
            return -1;
        } else if (std::basic_string_view<CharT>::traits_type::lt(ch2, ch1)) {
            return 1;
        }
    }
    if (lhs.size() < rhs.size()) {
        return -1;
    } else if (rhs.size() < lhs.size()) {
        return 1;
    }
    return 0;
}

int compare_strings_nocase(std::string_view lhs, std::string_view rhs) {
    return basic_compare_strings_nocase(lhs, rhs);
}

int compare_strings_nocase(std::wstring_view lhs, std::wstring_view rhs) {
    return basic_compare_strings_nocase(lhs, rhs);
}

// --------------------------

std::string to_lower(std::string_view s) {
    std::string lower(s);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](int c) { return to_lower(c); });
    return lower;
}

std::wstring to_lower(std::wstring_view s) {
    std::wstring lower(s);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](int c) { return to_lower(c); });
    return lower;
}

// --------------------------

std::string to_upper(std::string_view s) {
    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), [](int c) { return to_upper(c); });
    return upper;
}

std::wstring to_upper(std::wstring_view s) {
    std::wstring upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), [](int c) { return to_upper(c); });
    return upper;
}

}  // namespace util

#include "uxs/db/xml_impl.h"
#include "uxs/utf.h"

namespace lex_detail {
#include "xml_lex_defs.h"
}
namespace lex_detail {
#include "xml_lex_analyzer.inl"
}

static uint8_t g_spec_chars[256] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

namespace uxs {
namespace db {

xml::reader::reader(iobuf& input) : input_(input), name_cache_(16) { state_stack_.push_back(lex_detail::sc_initial); }

std::pair<xml::token_t, std::string_view> xml::reader::read_next() {
    if (is_end_element_pending_) {
        is_end_element_pending_ = false;
        return {token_t::kEndElement, name_cache_.front()};
    }
    if (!input_) { return {token_t::kEof, {}}; }
    while (input_.avail() || input_.peek() != std::char_traits<char>::eof()) {
        const char *first = input_.first_avail(), *last = input_.last_avail();
        std::string_view lval;
        if (*first == '<') {  // found '<'
            int tt = parse_token(lval);
            auto name_cache_it = name_cache_.begin(), name_cache_prev_it = name_cache_it;
            auto read_attribute = [this, &name_cache_it, &name_cache_prev_it](std::string_view lval) {
                int tt = 0;
                if (name_cache_it != name_cache_.end()) {
                    name_cache_it->assign(lval.data(), lval.size());
                } else {
                    name_cache_it = name_cache_.emplace_after(name_cache_prev_it, lval);
                }
                if ((tt = parse_token(lval)) != '=') { throw exception(format("{}: expected `=`", n_ln_)); }
                if ((tt = parse_token(lval)) != kString) {
                    throw exception(format("{}: expected valid attribute value", n_ln_));
                }
                name_cache_prev_it = name_cache_it;
                attrs_.emplace(*name_cache_it++, lval);
            };
            switch (tt) {
                case kStartElementOpen: {  // <name n1=v1 n2=v2...> or <name n1=v1 n2=v2.../>
                    attrs_.clear();
                    name_cache_it->assign(lval.data(), lval.size());
                    ++name_cache_it;
                    while (true) {
                        if ((tt = parse_token(lval)) == kName) {
                            read_attribute(lval);
                        } else if (tt == '>') {
                            return {token_t::kStartElement, name_cache_.front()};
                        } else if (tt == kEndElementClose) {
                            is_end_element_pending_ = true;
                            return {token_t::kStartElement, name_cache_.front()};
                        } else {
                            throw exception(format("{}: expected name, `>` or `/>`", n_ln_));
                        }
                    }
                } break;
                case kEndElementOpen: {  // </name>
                    if ((tt = parse_token(lval)) != '>') { throw exception(format("{}: expected `>`", n_ln_)); }
                    return {token_t::kEndElement, lval};
                } break;
                case kPiOpen: {  // <?xml n1=v1 n2=v2...?>
                    attrs_.clear();
                    if (uxs::compare_strings_nocase(lval, "xml") != 0) {
                        throw exception(format("{}: invalid document declaration", n_ln_));
                    }
                    name_cache_it->assign(lval.data(), lval.size());
                    ++name_cache_it;
                    while (true) {
                        if ((tt = parse_token(lval)) == kName) {
                            read_attribute(lval);
                        } else if (tt == kPiClose) {
                            return {token_t::kPreamble, name_cache_.front()};
                        } else {
                            throw exception(format("{}: expected name or `?>`", n_ln_));
                        }
                    }
                } break;
                case kComment: {  // comment <!--....-->: skip till `-->`
                    int ch = input_.get();
                    do {
                        while (input_ && ch != '-') {
                            if (ch == '\0') { return {token_t::kEof, {}}; }
                            if (ch == '\n') { ++n_ln_; }
                            ch = input_.get();
                        }
                        ch = input_.get();
                    } while (input_ && (ch != '-' || input_.peek() != '>'));
                    if (!input_) { break; }
                    input_.bump(1);
                } break;
                default: UNREACHABLE_CODE;
            }
        } else if (*first == '&') {  // parse entity
            if (parse_token(lval) == kPredefEntity) { return {token_t::kPlainText, lval}; }
            return {token_t::kEntity, lval};
        } else if (*first != 0) {
            last = std::find_if(first, last, [this](char ch) {
                if (ch != '\n') { return !!g_spec_chars[static_cast<unsigned char>(ch)]; }
                ++n_ln_;
                return false;
            });
            input_.bump(last - first);
            return {token_t::kPlainText, std::string_view(first, last - first)};
        } else {
            return {token_t::kEof, {}};
        }
    }
    return {token_t::kEof, {}};
}

int xml::reader::parse_token(std::string_view& lval) {
    while (true) {
        int pat = 0;
        unsigned llen = 0;
        const char* first = input_.first_avail();
        while (true) {
            bool stack_limitation = false;
            const char* last = input_.last_avail();
            if (state_stack_.avail() < static_cast<size_t>(last - first)) {
                last = first + state_stack_.avail();
                stack_limitation = true;
            }
            pat = lex_detail::lex(first, last, state_stack_.p_curr(), &llen, stack_limitation || input_);
            if (pat >= lex_detail::predef_pat_default) {
                break;
            } else if (stack_limitation) {  // enlarge state stack and continue analysis
                state_stack_.reserve_at_curr(llen);
                first = last;
                continue;
            } else if (!input_) {  // end of sequence, first_ == last_
                return kEof;
            }
            if (input_.avail()) {  // append read buffer to stash
                stash_.append(input_.first_avail(), input_.last_avail());
                input_.bump(input_.avail());
            }
            // read more characters from input
            input_.peek();
            first = input_.first_avail();
        }
        const char* lexeme = input_.first_avail();
        if (stash_.empty()) {  // the stash is empty
            input_.bump(llen);
        } else {
            if (llen >= stash_.size()) {  // all characters in stash buffer are used
                // concatenate full lexeme in stash
                size_t len_rest = llen - stash_.size();
                stash_.append(input_.first_avail(), input_.first_avail() + len_rest);
                input_.bump(len_rest);
            } else {  // at least one character in stash is yet unused
                      // put unused chars back to `iobuf`
                for (const char* p = stash_.last(); p != stash_.curr(); --p) { input_.unget(*(p - 1)); }
            }
            lexeme = stash_.data();
            stash_.clear();  // it resets end pointer, but retains the contents
        }
        switch (pat) {
            // ------ specials
            case lex_detail::pat_amp: {
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view("&", 1);
                    return kPredefEntity;
                }
                str_.push_back('&');
            } break;
            case lex_detail::pat_lt: {
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view("<", 1);
                    return kPredefEntity;
                }
                str_.push_back('<');
            } break;
            case lex_detail::pat_gt: {
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view(">", 1);
                    return kPredefEntity;
                }
                str_.push_back('>');
            } break;
            case lex_detail::pat_apos: {
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view("\'", 1);
                    return kPredefEntity;
                }
                str_.push_back('\'');
            } break;
            case lex_detail::pat_quot: {
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view("\"", 1);
                    return kPredefEntity;
                }
                str_.push_back('\"');
            } break;
            case lex_detail::pat_entity: {
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view(lexeme + 1, llen - 2);
                    return kEntity;
                }
                throw exception(format("{}: unknown entity name", n_ln_));
            } break;
            case lex_detail::pat_dcode: {
                unsigned unicode = 0;
                for (char ch : std::string_view(lexeme + 2, llen - 3)) { unicode = 10 * unicode + uxs::dig_v<10>(ch); }
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view(str_.first(), to_utf8(unicode, str_.first()));
                    return kEntity;
                }
                to_utf8(unicode, std::back_inserter(str_));
            } break;
            case lex_detail::pat_hcode: {
                unsigned unicode = 0;
                for (char ch : std::string_view(lexeme + 3, llen - 4)) {
                    unicode = (unicode << 4) + uxs::dig_v<16>(ch);
                }
                if (state_stack_.back() == lex_detail::sc_initial) {
                    lval = std::string_view(str_.first(), to_utf8(unicode, str_.first()));
                    return kEntity;
                }
                to_utf8(unicode, std::back_inserter(str_));
            } break;

            // ------ strings
            case lex_detail::pat_string_nl: {
                ++n_ln_;
                str_.push_back('\n');
            } break;
            case lex_detail::pat_string_other: return kEof;
            case lex_detail::pat_string_seq_quot:
            case lex_detail::pat_string_seq_apos: {
                str_.append(lexeme, lexeme + llen);
            } break;
            case lex_detail::pat_string_close_quot:
            case lex_detail::pat_string_close_apos: {
                if (str_.empty()) {
                    lval = std::string_view(lexeme, llen - 1);
                } else {
                    str_.append(lexeme, lexeme + llen - 1);
                    lval = std::string_view(str_.data(), str_.size());
                    str_.clear();  // it resets end pointer, but retains the contents
                }
                state_stack_.pop_back();
                return kString;
            } break;

            case lex_detail::pat_name: lval = std::string_view(lexeme, llen); return kName;
            case lex_detail::pat_start_element_open: {
                lval = std::string_view(lexeme + 1, llen - 1);
                return kStartElementOpen;
            } break;
            case lex_detail::pat_end_element_open: {
                lval = std::string_view(lexeme + 2, llen - 2);
                return kEndElementOpen;
            } break;
            case lex_detail::pat_end_element_close: return kEndElementClose;
            case lex_detail::pat_pi_open: lval = std::string_view(lexeme + 2, llen - 2); return kPiOpen;
            case lex_detail::pat_pi_close: return kPiClose;
            case lex_detail::pat_comment: return kComment;

            case lex_detail::pat_whitespace: break;
            case lex_detail::predef_pat_default: {
                switch (lexeme[0]) {
                    case '\"': state_stack_.push_back(lex_detail::sc_string_quot); break;
                    case '\'': state_stack_.push_back(lex_detail::sc_string_apos); break;
                    default: return static_cast<unsigned char>(*first);
                }
            } break;
            default: UNREACHABLE_CODE;
        }
    }
}

/*static*/ xml::reader::string_class xml::reader::classify_string(const std::string_view& sval) {
    int state = lex_detail::sc_value;
    for (unsigned char ch : sval) {
        state = lex_detail::Dtran[lex_detail::dtran_width * state + lex_detail::symb2meta[ch]];
    }
    switch (lex_detail::accept[state]) {
        case lex_detail::pat_null: return string_class::kNull;
        case lex_detail::pat_true: return string_class::kTrue;
        case lex_detail::pat_false: return string_class::kFalse;
        case lex_detail::pat_decimal: return string_class::kInteger;
        case lex_detail::pat_neg_decimal: return string_class::kNegInteger;
        case lex_detail::pat_real: return string_class::kDouble;
        case lex_detail::pat_ws_with_nl: return string_class::kWsWithNl;
        case lex_detail::pat_other_value: return string_class::kOther;
        default: UNREACHABLE_CODE;
    }
}

namespace xml {
template basic_value<char> reader::read(std::string_view, const std::allocator<char>&);
template basic_value<wchar_t> reader::read(std::string_view, const std::allocator<wchar_t>&);
template void writer::write(const basic_value<char>&, std::string_view, unsigned);
template void writer::write(const basic_value<wchar_t>&, std::string_view, unsigned);
}  // namespace xml
}  // namespace db
}  // namespace uxs

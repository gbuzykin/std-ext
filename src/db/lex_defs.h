// Lexegen autogenerated definition file - do not edit!

enum {
    err_end_of_input = -1,
    predef_pat_default = 0,
    pat_escape_unicode,
    pat_escape_a,
    pat_escape_b,
    pat_escape_f,
    pat_escape_r,
    pat_escape_n,
    pat_escape_t,
    pat_escape_v,
    pat_escape_other,
    pat_string_seq,
    pat_string_nl,
    pat_string_close,
    pat_null_literal,
    pat_true_literal,
    pat_false_literal,
    pat_dec_literal,
    pat_real_literal,
    pat_comment,
    pat_whitespace,
    pat_nl,
    pat_string,
    pat_other_char,
    total_pattern_count,
};

enum {
    sc_initial = 0,
    sc_string,
};

int lex(const char* first, const char* last, std::vector<int>& state_stack, unsigned& llen, bool has_more);

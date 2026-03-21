#pragma once
#include "../core.h"

// Regular Expression: [0-9]*(.)[0-9]+
struct TokenNumber {
    int64_t value_int;
    double  value_float;
};

// Regular Expression: [a-zA-Z_]+
struct TokenIdentifier {
    string_view value;
};

// Everything that is not a number or identifier, will be a symbol
struct TokenSymbol {
    u32 value;
};

struct Token {
    enum class Type {
        End, Number, Identifier, Symbol,
        Newline, // Only available if newlines are enabled
    };
    
    Type type;
    uint32_t line;
    string_view expression;
    union {
        TokenNumber     number;
        TokenIdentifier identifier;
        TokenSymbol     symbol;
    };

    bool is_end()        { return type == Type::End; }
    bool is_number()     { return type == Type::Number; }
    bool is_identifier() { return type == Type::Identifier; }
    bool is_symbol()     { return type == Type::Symbol; }
    bool is_newline()    { return type == Type::Newline; }

    operator bool() const {
        return type != Type::End;
    }
    bool is_end_or_newline() const {
        return type == Type::End || type == Type::Newline;
    }
};

struct Tokenizer {
    struct Location {
        uint64_t  offset      { 0 };
        uint32_t  line_number { 1 };
        bool      peeking     { false };
        Token     curr        {};
        Token     next        {};
    };

    string_view filename;
    string_view source;
    Location    location;
    bool        enable_newline;

    Tokenizer(string_view filename, string_view source, bool enable_newline = false);

    bool has_tokens() const { return bool(location.curr); }

    Result Error(const char* message);

    Token  get();
    Token  peek();
    Result get_symbol(u32 &out);
    Result get_int(u64 &out);
    Result get_float(double &out);
    Result get_identifier(string_view &out);

    Token  next();
    Result next_symbol(u32 &out);
    Result next_int(u64 &out);
    Result next_float(double &out);
    Result next_identifier(string_view &out);

private:
    Token generate_token();
};


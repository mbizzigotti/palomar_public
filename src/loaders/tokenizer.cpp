#include "tokenizer.h"

template<> void print<Token>(const Token &token) {
    switch (token.type) {
        default: {
            printf("Token(Undefined)");
        }
        break;case Token::Type::End: {
            printf("Token(End)");
        }
        break;case Token::Type::Newline: {
            printf("Token(Newline)");
        }
        break;case Token::Type::Identifier: {
            printf("Token(Identifier \"%.*s\")", FORMAT_STRING(token.identifier.value));
        }
        break;case Token::Type::Number: {
            printf("Token(Number \"%.6f\")", token.number.value_float);
        }
        break;case Token::Type::Symbol: {
            printf("Token(Symbol \"%.*s\")", FORMAT_STRING(token.expression));
        }
    }
}

Tokenizer::Tokenizer(string_view filename, string_view source, bool enable_newline)
:   filename(filename), source(source), enable_newline(enable_newline)
{
    location.curr = generate_token();
}

Result Tokenizer::get_identifier(string_view &out) {
    if (location.curr.type != Token::Type::Identifier)
        return Failed;
    out = location.curr.identifier.value;
    return Success;
}

Result Tokenizer::Error(const char *message) {
    return ::Error(filename, location.line_number, message);
}

Token Tokenizer::get() {
    return location.curr;
}

Token Tokenizer::peek() {
    if (!location.peeking) {
        location.next = generate_token();
        location.peeking = true;
    }
    return location.next;
}

Token Tokenizer::next() {
    if (location.peeking) {
        location.peeking = false;
        return location.curr = location.next;
    }
    return location.curr = generate_token();
}

Result Tokenizer::next_symbol(u32 &out) {
    if (next().type != Token::Type::Symbol)
        return Error("expected symbol here");
    out = location.curr.symbol.value;
    return Success;
}

Result Tokenizer::next_int(u64 &out) {
    if (next().type != Token::Type::Number)
        return Error("expected int here");
    out = location.curr.number.value_int;
    return Success;
}

Result Tokenizer::next_float(double &out) {
    if (next().type != Token::Type::Number)
        return Error("expected float here");
    out = location.curr.number.value_float;
    return Success;
}

Result Tokenizer::next_identifier(string_view &out) {
    if (next().type != Token::Type::Identifier)
        return Error("expected identifier here");
    out = location.curr.identifier.value;
    return Success;
}


static bool tok_is_lower_case(char c) { return ((c >= 'a') && (c <= 'z')); }
static bool tok_is_upper_case(char c) { return ((c >= 'A') && (c <= 'Z')); }
static bool tok_is_number(char c) { return ((c >= '0') && (c <= '9')); }
static bool tok_is_white_space(char c) { return (c <= ' '); }
static bool tok_is_ident_character(char c) {
    return (tok_is_lower_case(c) || tok_is_upper_case(c) || tok_is_number(c) || (c == '_'));
}

Token Tokenizer::generate_token() {
    auto& [
        offset,
        line_number,
        peeking,
        curr,
        next
    ] = location;

    while (offset < source.size()) {
        uint8_t ch = source[offset];
        u64 start = offset;

        if (tok_is_white_space(ch)) {
            offset++;
            uint32_t prev_line_number = line_number;
            if (ch == '\r') {
                if (offset < source.size() && source[offset] == '\n')
                    offset += 1; // Handle Windows: CR LF
                line_number += 1;
            }
            else if (ch == '\n') {
                line_number += 1;
            }
            if (enable_newline && prev_line_number != line_number) {
                return Token {
                    .type = Token::Type::Newline,
                    .line = prev_line_number,
                    .expression = source.substr(start, 0),
                };
            }
            continue;
        }

        switch (ch)
        {
        default: {
            if (!tok_is_lower_case(ch) && !tok_is_upper_case(ch)) {
                offset++;
                return Token {
                    .type = Token::Type::Symbol,
                    .line = line_number,
                    .expression = source.substr(start, 1),
                    .symbol = { .value = ch },
                };
            }

            offset++;
            size_t count = 1;
            while (offset < source.size() && tok_is_ident_character(source[offset])) {
                offset++;
                count++;
            }
            return Token {
                .type = Token::Type::Identifier,
                .line = line_number,
                .expression = source.substr(start, count),
                .identifier = { .value = source.substr(start, count) },
            };
        }

        break;
        case '.': {
            if (offset+1 < source.size() && tok_is_number(source[offset+1])) {
                goto parse_number;
            }
            
            offset++;
            return Token {
                .type = Token::Type::Symbol,
                .line = line_number,
                .expression = source.substr(start, 1),
                .symbol = { .value = ch },
            };
        }

        break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-': parse_number: {
            if (ch == '-' && (offset+1 < source.size()) && !tok_is_number(source[offset+1])) {
                offset++;
                return Token {
                    .type = Token::Type::Symbol,
                    .line = line_number,
                    .expression = source.substr(start, 1),
                    .symbol = { .value = '-' },
                };
            }
            const char* end = source.data() + start;
            TokenNumber number = {
                .value_int = strtoll(source.data() + start, nullptr, 10),
                .value_float = strtod(source.data() + start, (char**)&end),
            };
            u64 count = (end - (source.data() + start));
            offset += count;
            return Token {
                .type = Token::Type::Number,
                .line = line_number,
                .expression = source.substr(start, count),
                .number = number,
            };
        }
        }
    }
    return Token {
        .type = Token::Type::End,
        .line = line_number,
    };
}

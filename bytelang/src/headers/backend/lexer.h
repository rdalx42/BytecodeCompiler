
#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>

extern const std::string digits ;
extern const std::string characters;

extern const std::vector<std::string> keywords;
extern const std::vector<std::string> bytecode_keywords;

enum TOKEN_T{
    INT,
    FLOAT,
    OP,
    LP,
    RP,
    KEYWORD,
    IDENTIFIER,
    NEWLINE,
    
    // bytecode

    BYTECODE_NEG,
    BYTECODE_PUSH,
    BYTECODE_POP,
    BYTECODE_ADD,
    BYTECODE_SUB,
    BYTECODE_TOP,
    BYTECODE_MUL,
    BYTECODE_DIV,
    BYTECODE_STORE,
    BYTECODE_LOAD,
    BYTECODE_POP_ALL,
    BYTECODE_CLEANUP
};

struct TOKEN{
    std::string value; 
    TOKEN_T type;
};

struct LEXER{
    bool in_bytecode=false;
    std::string content;
    size_t pos = 0;
    std::vector<TOKEN> tokens;
    LEXER(const std::string& c) : content(c), pos(0) {}
};

char peek(LEXER& lex);

void doidentifier(LEXER& lexer, int& index);
void donum(LEXER& lexer, int& index);
void dolex(LEXER&lexer);

void display_lex(LEXER& lexer);

#endif
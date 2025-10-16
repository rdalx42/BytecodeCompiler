
#ifndef LEXER_H
#define LEXER_H

#include <unordered_map>
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
    GOTO_LABEL,

    // bytecode
    BYTECODE_GOTO_LABEL,
    BYTECODE_GOTO,
    BYTECODE_NEG,
    BYTECODE_PUSH,
    BYTECODE_POP,
    BYTECODE_ADD,
    BYTECODE_GT,
    BYTECODE_LT,
    BYTECODE_LTE,
    BYTECODE_GTE,
    BYTECODE_EQ,
    BYTECODE_NOTEQ,
    BYTECODE_SUB,
    BYTECODE_TOP,
    BYTECODE_MUL,
    BYTECODE_DIV,
    BYTECODE_STORE,
    BYTECODE_LOAD,
    BYTECODE_POP_ALL,
    BYTECODE_CLEANUP,
    BYTECODE_MAKE_BLOCK,
    BYTECODE_DEL_BLOCK,
    BYTECODE_SAFETY,
    BYTECODE_GOTO_IF_ZERO,
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
    
    std::unordered_map<std::string,int>goto_positions;

    LEXER(const std::string& c) : content(c), pos(0) {}
};

char peek(LEXER& lex,int&index);

void doidentifier(LEXER& lexer, int& index);
void donum(LEXER& lexer, int& index);
void dolex(LEXER&lexer);

void display_lex(LEXER& lexer);

#endif

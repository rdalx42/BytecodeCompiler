
#ifndef LEXER_H
#define LEXER_H

#include <unordered_map>
#include <optional>
#include <vector>
#include <string>


extern const std::string digits ;
extern const std::string characters;

extern const std::vector<std::string> keywords;
extern const std::vector<std::string> bytecode_keywords;
extern const std::vector<std::string> builtins;

enum TOKEN_T{

    INT,
    FLOAT,
    STRING,
    OP,
    LP,
    RP,
    LPSQ,
    RPSQ,
    KEYWORD,
    IDENTIFIER,
    NEWLINE,
    GOTO_LABEL,
    COMMA,
    ACCESS_IDENTIFIER,
    
    // bytecode
    BYTECODE_RET,
    BYTECODE_GOTO_LABEL,
    BYTECODE_SET_AT,
    BYTECODE_LOAD_AT,
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
    BYTECODE_ADD_NEG,
    BYTECODE_NOT,
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

struct TOKEN {
    std::string value;
    TOKEN_T type;

    // std::optional variables to help during runtime
    std::optional<int>int_val;
    std::optional<double>double_val;
    std::optional<std::string>str_val;
    std::optional<int>jump_pos;
    bool used_function=false; // if false we don't add to memory, this we'll also be toggled via the AST, since a function won't be unused if passed as a parameter
    bool is_function=false;
  //  int dispatch_index = -1;
   // std::optional<size_t>pair_token_jump_pos;
    std::optional<int>scope_level;
    std::optional<int>var_id; // for STORE and LOAD 
    std::optional<int>variable_scope_level; // for debugging mainly
    std::optional<bool>is_new_val;
    std::optional<int>function_base_scope;


    union goto_check_union{
        bool is_function_start = false;
        size_t pair_token_jump_pos;
        
    }goto_check_union;

};


struct SCOPE_COUNT{
    int scope_level=0;
   // int start_pos=0;
};

struct PRE_CALC_STACK_VALUE{
    int id=0;
    std::vector<std::string>var_names;
};

struct PRE_CALC_VAR_DATA{
    int id=0;
    int scope_level=0;
};

struct LEXER{
    
    bool in_bytecode=false;
    bool in_bytecode_pre_compiled=false;
    std::string content;
    size_t pos = 0;
    std::vector<TOKEN> tokens;
    std::vector<bool>unused_load_function_indicies_info;
    
    std::vector<PRE_CALC_STACK_VALUE>pre_calc_stack;

    int declared_pre_calcs=0; 

    std::unordered_map<std::string,PRE_CALC_VAR_DATA>declared_variables; // variable name to id pre-runtime
    std::unordered_map<std::string,int>goto_positions;
    std::unordered_map<std::string,SCOPE_COUNT>goto_scope_count;

    LEXER(const std::string& c) : content(c), pos(0) {}
};

int get_dispatch_index(const TOKEN_T& type);
//char peek(LEXER& lex,int&index);

void doidentifier(LEXER& lexer, int& index);
void donum(LEXER& lexer, int& index);
void dolex(LEXER&lexer);
void dostring(LEXER& lexer, int& index);
void display_lex(LEXER& lexer);

#endif

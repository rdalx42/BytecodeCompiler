#ifndef COMPILER_H
#define COMPILER_H

#pragma GCC optimize("O3","unroll-loops")

#include "../headers/backend/ast.h"
#include "../headers/backend/lexer.h"
#include "../headers/runtime/memory.h"

#include <algorithm>
#include <string>
#include <vector>

struct COMPILER {
    std::string bytecode;
    MEMORY memory;
    
    CALL_STACK function_call_stack;
   // UTILS& ut;
    AST& ast;              
    LEXER& lex;           

    std::vector<FAST_STRING_COMPONENT>preloaded_strings_per_token;
    
    COMPILER(AST& a, LEXER& l) : ast(a), lex(l){}
};

COMPILER init(AST& ast, LEXER& lex);

void pre_init_data(COMPILER& comp);
void generate_bytecode(COMPILER& comp, AST_NODE* nd);
void compile_ast_to_bytecode(COMPILER& comp);
void compile(COMPILER& comp);
void print_bytecode(COMPILER& comp) ;


#endif

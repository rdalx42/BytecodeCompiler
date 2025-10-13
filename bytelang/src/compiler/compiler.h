#ifndef COMPILER_H
#define COMPILER_H

#include "../headers/backend/ast.h"
#include "../headers/backend/lexer.h"
#include "../headers/runtime/memory.h"

#include <string>
#include <vector>

struct COMPILER {
    std::string bytecode;
    MEMORY memory;
    AST& ast;              
    LEXER& lex;           
    
    COMPILER(AST& a, LEXER& l) : ast(a), lex(l){}
};

COMPILER init(AST& ast, LEXER& lex);

void generate_bytecode(COMPILER& comp, AST_NODE* nd);
void compile_ast_to_bytecode(COMPILER& comp);
void compile(COMPILER& comp);


#endif


#ifndef AST_H
#define AST_H

#include "../backend/lexer.h"
#include <vector>
#include <string>
#include <algorithm>

enum NODE_TYPE{
    AST_TOP,
    AST_UN_OP,
    AST_BIN_OP,
    AST_INT,
    AST_FLOAT,
    AST_STRING,
    AST_IDENTIFIER,
    AST_FOR,
    AST_VAR_DECL,
    AST_VAR_ASSIGN,
    AST_VAR_ACCESS,
    AST_PROGRAM,
   
    AST_GOTO_LABEL,
    AST_GOTO,
    AST_BLOCK_START,
    AST_BLOCK_END,
    AST_NONE,
    AST_NEWLINE,
    AST_IF,
    AST_ELSE,
    AST_WHILE
    
};

struct AST_NODE{
    NODE_TYPE type;
    std::string value;
    std::vector<AST_NODE*> children;
};

struct AST{
    std::vector<TOKEN>tokens;
    AST_NODE* root;
    AST() : root(nullptr) {}
};  

// debug

void print_ast_node(AST_NODE* node, int depth = 0);
void print_ast(AST& ast);
std::string node_type_to_string(NODE_TYPE& type);

// ast

AST_NODE* parse_comparison(AST& ast, int& index);
AST_NODE* parse_program(AST& ast);
AST_NODE* create_node(NODE_TYPE type, std::string value);

AST_NODE* parse_primary(AST& ast, int& index);   // VAR_DECL, VAR_ASSIGN, VAR_ACCESS, expressions with parentheses
AST_NODE* parse_factor(AST& ast, int& index);    // INT, FLOAT, IDENTIFIER, (expr)
AST_NODE* parse_term(AST& ast, int& index);      // *, /
AST_NODE* parse_expression(AST& ast, int& index); // +, -

#endif

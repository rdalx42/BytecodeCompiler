
#include "../headers/runtime/shell.h"
#include "../headers/backend/lexer.h"
#include "../headers/backend/ast.h"
#include "../compiler/compiler.h"

#include <fstream>
#include <iostream>

void doshell(SHELL& sh) {
    LEXER lexer(sh.shell_content);
    dolex(lexer);
    std::cout << "\nTokens:\n";
    display_lex(lexer);

    AST ast;
    ast.tokens = lexer.tokens;
    ast.root = parse_program(ast);

    std::cout << "\nAST:\n";
    print_ast_node(ast.root);

    LEXER lex2("//nothing");
    COMPILER compiler = init(ast,lex2);
    compile_ast_to_bytecode(compiler);
    
    compiler.bytecode+="CLEANUP\n";
    lex2.content = compiler.bytecode;
    lex2.in_bytecode=true;
    dolex(lex2);
    display_lex(lex2);
    compiler.lex=lex2;
    std::cout << "\nBytecode:\n" << compiler.bytecode << "\n";

    compile(compiler);
}

int main(void){
    SHELL sh;
    sh.shell_content = "";

    std::ifstream in("examples/main.byt");
    if (!in.is_open()) {
        std::cerr << "Failed to open input file\n";
        return 1;
    }

    std::string ln;
    while (std::getline(in, ln)) {
        sh.shell_content += ln + '\n';
    }
    
    std::cout << "File content loaded:\n" << sh.shell_content << "\n";

    doshell(sh);

    
    return 0;
}
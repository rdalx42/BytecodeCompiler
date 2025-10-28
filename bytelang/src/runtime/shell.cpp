
#include "../headers/runtime/shell.h"
#include "../headers/backend/lexer.h"
#include "../headers/backend/ast.h"
#include "../compiler/compiler.h"

#include <ctime>
#include <fstream>
#include <iostream>

size_t total_bytes_allocated = 0;

void* operator new(size_t size) {
    total_bytes_allocated += size;
    return malloc(size);
}

void operator delete(void* ptr, size_t size) noexcept {
    total_bytes_allocated -= size; 
    free(ptr);
}

void doshell(SHELL& sh) {
    
    std::cout<<"\033[2J"; // clear screen
    
    std::cout<<"Compiling to bytecode...\n";

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
    compiler.lex.tokens=lex2.tokens;

    
    print_bytecode(compiler);

    time_t start_time = clock();
    size_t total_bytes_allocated_before_execution = total_bytes_allocated;

    compile(compiler);

    time_t end_time = clock();

    std::cout<<"\nExecution finished in " << double(end_time - start_time) / CLOCKS_PER_SEC << " seconds.\n";
    std::cout<<"Bytes allocated during execution: " << (total_bytes_allocated - total_bytes_allocated_before_execution) << " bytes.\n";
}

int main(void){
    
    SHELL sh;
    sh.shell_content = "";

    std::ifstream in("examples/main.vexa");
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

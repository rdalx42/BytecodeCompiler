
#include "../headers/runtime/shell.h"
#include "../headers/backend/lexer.h"
#include "../headers/backend/ast.h"
#include "../compiler/compiler.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

#define VERSION_CURRENT "1.1.0"

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

    std::string do_shell_content ="";

    std::ifstream sh_in(sh.filename);
    
    if(!sh_in.is_open()){
        display_non_fatal_err("Failed to open file of name: " + sh.filename);
        return;
    }

    std::string sln="";

    while(std::getline(sh_in,sln)){
        do_shell_content+=sln + '\n';
    }

    LEXER lexer(do_shell_content);
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
    pre_init_data(compiler);

    time_t start_time = clock();
    size_t total_bytes_allocated_before_execution = total_bytes_allocated;

    compile(compiler);

    time_t end_time = clock();

    std::cout<<"\nExecution finished in " << double(end_time - start_time) / CLOCKS_PER_SEC << " seconds.\n";
    std::cout<<"Bytes allocated during execution: " << (total_bytes_allocated - total_bytes_allocated_before_execution) << " bytes.\n";
}

void prompt(SHELL& sh){

    std::string line;
    std::cout<<"Vexa Programming Language made by @rdalx42 on GitHub, type 'help;' if you need any\n";
    std::getline(std::cin,line);


    size_t semicolon_pos = line.find(';');
    if (semicolon_pos == std::string::npos) {
        std::cout << "\033[31m[Error: Expected ';' at end of command]\033[0m\n";
        return;
    }

    std::string command_str = line.substr(0, semicolon_pos);
    std::stringstream ss(command_str);

    std::string command, filename;
    ss >> command >> filename;

    std::cout << "\033[36mCommand: \033[0m";
    if (command == "run" || command == "clear" || command == "help" || command == "version") {
        std::cout << "\033[32m" << command << "\033[0m";
    } else {
        std::cout << "\033[31m" << command << "\033[0m";
    }

    if (!filename.empty()) {
        std::cout << " \033[33m" << filename << "\033[0m";
    }
    std::cout << "\n";

    if (command != "run" && command != "clear" && command != "help" && command != "version") {
        std::cout << "\033[31m[Error: Unknown command '" << command << "']\033[0m\n";
        return;
    }

    if (command == "clear") {
        std::cout << "\033[2J\033[H";
        return;
    }

    if (command == "version") {
        std::cout << "\033[35m" << VERSION_CURRENT << "\033[0m\n";
        return;
    }

    if (command == "help") {
        std::cout << "\033[34m --- Vexa Terminal Instruction Menu --- \033[0m\n";
        std::cout << "\033[36m --- Commands --- \033[0m\n";
        std::cout << "\033[32mclear;\033[0m // clears terminal\n";
        std::cout << "\033[32mrun [filepath];\033[0m // runs .vexa file\n";
        std::cout << "\033[32mhelp;\033[0m // shows this help menu\n";
        std::cout << "\033[32mversion;\033[0m // displays current version\n";
        return;
    }

    size_t dot_pos = filename.rfind('.');
    if (dot_pos == std::string::npos || filename.substr(dot_pos) != ".vexa") {
        std::cout << "\033[31m[Error: File must have .vexa extension]\033[0m\n";
        return;
    }

    sh.filename=filename;
    std::cout << "\033[32m[Command accepted: running " << filename << "]\033[0m\n";
}

int main(void){
    
    SHELL sh;
  
    while(true){
        prompt(sh);
        if(!sh.filename.empty()){
            doshell(sh);
            sh.filename="";
        }
    }
    
    return 0;
}

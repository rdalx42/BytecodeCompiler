#include "../headers/runtime/err.h"
#include "compiler.h"
#include <iostream>

COMPILER init(AST& ast, LEXER& lex) {
    COMPILER comp(ast, lex);
    comp.memory.operation_stack.clear();
    comp.memory.variables.clear();
    comp.memory.variables.reserve(100);
    comp.memory.operation_stack.reserve(10);
    comp.bytecode.clear();
    return comp;
}

void generate_bytecode(COMPILER& comp, AST_NODE* nd) {
    if (!nd) return;

    switch(nd->type) {
        case AST_PROGRAM:
            comp.bytecode += "//PROGRAM START\n";
            break;
        case AST_INT:
        case AST_FLOAT:
            comp.bytecode += "PUSH " + nd->value + "\n";
            break;
        case AST_VAR_ACCESS:
            comp.bytecode += "LOAD " + nd->value + "\n";
            break;
        case AST_TOP:
            comp.bytecode += "TOP\n";
            break;
        case AST_UN_OP:
            if (!nd->children.empty()) {
                generate_bytecode(comp, nd->children[0]);
                comp.bytecode += "NEG\n";
            }
            return;
        case AST_VAR_ASSIGN:
            if (nd->children.size() == 2) {
                generate_bytecode(comp, nd->children[1]);
                comp.bytecode += "STORE " + nd->children[0]->value + "\n";
            }
            return;
        case AST_BIN_OP:
            if (nd->children.size() == 2) {
                if (nd->value == "-" && nd->children[0]->type == AST_NONE) {
                    generate_bytecode(comp, nd->children[1]);
                    comp.bytecode += "NEG\n";
                } else {
                    generate_bytecode(comp, nd->children[0]);
                    generate_bytecode(comp, nd->children[1]);
                    if (nd->value == "+") comp.bytecode += "ADD\n";
                    else if (nd->value == "-") comp.bytecode += "SUB\n";
                    else if (nd->value == "*") comp.bytecode += "MUL\n";
                    else if (nd->value == "/") comp.bytecode += "DIV\n";
                }
            }
            return;
        default:
            break;
    }

    for (AST_NODE* child : nd->children) {
        if (nd->type != AST_BIN_OP && nd->type != AST_VAR_ASSIGN && nd->type != AST_UN_OP)
            generate_bytecode(comp, child);
    }
}

void print_value(const VALUE& val) {
    std::visit([](auto&& arg){
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::vector<VALUE>>) {
            std::cout << "[";
            for (size_t i = 0; i < arg.size(); ++i) {
                print_value(arg[i]);
                if (i + 1 < arg.size()) std::cout << ", ";
            }
            std::cout << "]";
        } else {
            std::cout << arg;
        }
    }, val.data);
}

void compile_ast_to_bytecode(COMPILER& comp) {
    generate_bytecode(comp, comp.ast.root);
}

void compile(COMPILER& comp) {
    for (size_t i = 0; i < comp.lex.tokens.size(); i++) {
        TOKEN& tok = comp.lex.tokens[i];
       // std::cout<<tok.value<<">>\n";
        switch (tok.type) {
            
            
            case TOKEN_T::BYTECODE_TOP:
                std::cout << "Stack top: ";
                if (!comp.memory.operation_stack.empty())
                    print_value(comp.memory.operation_stack.back());
                std::cout << "\n";
                break;
            case TOKEN_T::BYTECODE_PUSH:
                if (i + 1 < comp.lex.tokens.size()) {
                    TOKEN& next = comp.lex.tokens[i + 1];
                    VALUE val;
                    if (next.type == TOKEN_T::INT) val = VALUE(std::stoi(next.value));
                    else if (next.type == TOKEN_T::FLOAT) val = VALUE(std::stod(next.value));
                    else if (next.type == TOKEN_T::IDENTIFIER) {
                        if (comp.memory.exists(next.value)) val = comp.memory.get(next.value);
                        else display_err("Variable '" + next.value + "' not defined");
                    } else display_err("Expected value after PUSH");
                    comp.memory.operation_stack.push_back(val);
                    i++;
                } else display_err("Expected value after PUSH");
                break;
            case TOKEN_T::BYTECODE_STORE:
                if (i + 1 < comp.lex.tokens.size()) {
                    std::string var_name = comp.lex.tokens[i + 1].value;
                    if (!comp.memory.operation_stack.empty()) {
                        VALUE val = comp.memory.operation_stack.back();
                        comp.memory.operation_stack.pop_back();
                        comp.memory.set(var_name, val);
                    } else display_err("Stack underflow on STORE");
                    i++;
                } else display_err("STORE missing variable name");
                break;
            case TOKEN_T::BYTECODE_LOAD:
                if (i + 1 < comp.lex.tokens.size()) {
                    std::string var_name = comp.lex.tokens[i + 1].value;
                    if (comp.memory.exists(var_name))
                        comp.memory.operation_stack.push_back(comp.memory.get(var_name));
                    else display_err("Variable '" + var_name + "' not defined");
                    i++;
                } else display_err("LOAD missing variable name");
                break;
            case TOKEN_T::BYTECODE_ADD:
            case TOKEN_T::BYTECODE_SUB:
            case TOKEN_T::BYTECODE_MUL:
            case TOKEN_T::BYTECODE_DIV:
                if (comp.memory.operation_stack.size() < 2&&comp.lex.tokens[i].value!="NEG") {display_err("Stack underflow on operation");}
              //  std::cout<<tok.value<<"\n";
                if (tok.value == "NEG") {
                    if (comp.memory.operation_stack.empty())
                        display_err("Stack underflow on NEG");
                    else {
                        
                        VALUE& val = comp.memory.operation_stack.back();
                        
                        std::visit([&](auto&& arg){
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T,int> || std::is_same_v<T,double>)
                                
                                arg = -arg;
                            else
                                display_err("NEG applied to non-numeric type");
                        }, val.data);
                    }
                    break;
                }

                else {
                    VALUE b = comp.memory.operation_stack.back(); comp.memory.operation_stack.pop_back();
                    VALUE a = comp.memory.operation_stack.back(); comp.memory.operation_stack.pop_back();
                    std::visit([&](auto&& arg_a, auto&& arg_b){
                    using TA = std::decay_t<decltype(arg_a)>;
                    using TB = std::decay_t<decltype(arg_b)>;
                    if constexpr ((std::is_same_v<TA,int> || std::is_same_v<TA,double>) &&
                                (std::is_same_v<TB,int> || std::is_same_v<TB,double>)) {
                        double result = 0;
                        if (tok.type == TOKEN_T::BYTECODE_ADD) result = arg_a + arg_b;
                        else if (tok.type == TOKEN_T::BYTECODE_SUB) result = arg_a - arg_b;
                        else if (tok.type == TOKEN_T::BYTECODE_MUL) result = arg_a * arg_b;
                        else if (tok.type == TOKEN_T::BYTECODE_DIV) {
                            if (arg_b == 0) display_err("Division by zero");
                            result = arg_a / arg_b;
                        }
                        comp.memory.operation_stack.push_back(VALUE(result));
                    } else display_err("Type mismatch in operation");
                }, a.data, b.data);

                }
                break;

            case TOKEN_T::BYTECODE_POP:
                if (!comp.memory.operation_stack.empty()) comp.memory.operation_stack.pop_back();
                else display_err("Stack underflow on POP");
                break;
            case TOKEN_T::BYTECODE_POP_ALL:
            case TOKEN_T::BYTECODE_CLEANUP:
                comp.memory.operation_stack.clear();
                comp.memory.variables.clear();
                break;
            default:
                break;
        }
    }
}

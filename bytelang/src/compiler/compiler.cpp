#include "../headers/runtime/err.h"
#include "compiler.h"
#include <iostream>

COMPILER init(AST& ast, LEXER& lex) {
    COMPILER comp(ast, lex);
    comp.memory.stack_vals.clear();
    comp.memory.stack_vals.reserve(100);
    comp.memory.operation_stack.clear();
    comp.memory.values.clear();
    comp.memory.id_for_value.clear();
    comp.memory.id_for_value.reserve(100);
    comp.memory.values.reserve(100);
    comp.memory.operation_stack.reserve(10);
    comp.bytecode.clear();
    return comp;
}
// generate_bytecode

void generate_bytecode(COMPILER& comp, AST_NODE* nd) {
    
    static int builtin_goto_counter = 0;
    
    if (!nd) return;

    switch(nd->type) {
        case AST_PROGRAM:
            comp.bytecode += "//PROGRAM START\n";
            break;

        case AST_GOTO_LABEL:
            comp.bytecode += ">" + nd->value + "\n";
            comp.bytecode+="SAFETY_LABEL\n";
            return;

        case AST_GOTO:
            comp.bytecode += "GOTO_" + nd->value + "\n";  // keep label in bytecode string
            return;

        case AST_INT:
        case AST_FLOAT:
            comp.bytecode += "PUSH " + nd->value + "\n";
            
            break;

        case AST_BLOCK_START:
            if(nd->value=="don't add"){
                break;
            }
            comp.bytecode += "BLOCK_START\n";
            break;
        case AST_BLOCK_END:
            comp.bytecode += "BLOCK_END\n";
            break;

        case AST_VAR_ACCESS:
            comp.bytecode += "LOAD " + nd->value + "\n";
            break;

        case AST_IF: {
            int this_if = builtin_goto_counter++;
            AST_NODE* condition = nd->children[0];
            AST_NODE* then_block = nd->children[1];
            AST_NODE* else_block = (nd->children.size() > 2) ? nd->children[2] : nullptr;

            std::string else_label = "BUILTIN_IF_ELSE_" + std::to_string(this_if);
            std::string end_label  = "BUILTIN_IF_END_" + std::to_string(this_if);

            generate_bytecode(comp, condition);
            comp.bytecode += "GOTOZERO_" + else_label + "\n";

            // THEN branch
            comp.bytecode += "BLOCK_START\n";
            generate_bytecode(comp, then_block);
            comp.bytecode += "BLOCK_END\n";

            if (else_block) {
                comp.bytecode += "GOTO_" + end_label + "\n";
                comp.bytecode += ">" + else_label + "\n";
                comp.bytecode += "SAFETY_LABEL\n";
                comp.bytecode += "BLOCK_START\n";
                generate_bytecode(comp, else_block);
                comp.bytecode += "BLOCK_END\n";
                comp.bytecode += ">" + end_label + "\n";
                comp.bytecode += "SAFETY_LABEL\n";
            } else {
                comp.bytecode += ">" + else_label + "\n";
                comp.bytecode += "SAFETY_LABEL\n";
            }
            return;
        }

        case AST_WHILE:{

            int this_while = builtin_goto_counter++;
            AST_NODE* condition = nd->children[0];
            AST_NODE* body = nd->children[1];

            std::string start_label = "BUILTIN_WHILE_START_" + std::to_string(this_while);
            std::string end_label = "BUILTIN_WHILE_END_" + std::to_string(this_while);

            comp.bytecode += ">" + start_label + "\n";
            comp.bytecode += "SAFETY_LABEL\n";
            
            generate_bytecode(comp, condition);
            comp.bytecode+="BLOCK_START\n";
            comp.bytecode += "GOTOZERO_" + end_label + "\n";

            generate_bytecode(comp, body);
            comp.bytecode += "GOTO_" + start_label + "\n";
              

            comp.bytecode += ">" + end_label + "\n";
            comp.bytecode += "SAFETY_LABEL\n";
            
            return;
        }


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
                    else if (nd->value == "!=") comp.bytecode += "NOTEQ\n";
                    else if (nd->value == "==") comp.bytecode += "EQ\n";
                    else if (nd->value == ">") comp.bytecode += "GT\n";
                    else if (nd->value == "<") comp.bytecode += "LT\n";
                    else if (nd->value == ">=") comp.bytecode += "GTE\n";
                    else if (nd->value == "<=") comp.bytecode += "LTE\n";
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
    size_t i = 0;
    while (i < comp.lex.tokens.size()) {
        TOKEN& tok = comp.lex.tokens[i];
       // if(tok.value!="newline"){
       //     std::cout<<tok.value<<"\n";}
        switch (tok.type) {
            
            case TOKEN_T::BYTECODE_TOP:
                std::cout << "Stack top: ";
                if (!comp.memory.operation_stack.empty())
                    print_value(comp.memory.operation_stack.back());
                std::cout << "\n";
                i++;
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
                    i += 2;
                } else display_err("Expected value after PUSH");
                break;

            case TOKEN_T::BYTECODE_DEL_BLOCK:{
                if(comp.memory.current_stack_amount<=0){
                    display_err("Cannot delete block, block already stack empty");
                }else if(comp.memory.current_stack_amount>0){
                    comp.memory.del_stack();
                    comp.memory.current_stack_amount--;
                }i++;
                break;
            }

            case TOKEN_T::BYTECODE_NEG:{
                
                if(comp.memory.operation_stack.empty()){
                    display_err("Stack underflow on NEGATE operation");
                }

                VALUE val = comp.memory.operation_stack.back();
                comp.memory.operation_stack.pop_back();
                std::visit([&](auto&& arg){
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr(std::is_same_v<T, int>) {
                        comp.memory.operation_stack.push_back(VALUE(-arg));
                    } else if constexpr(std::is_same_v<T, double>) {
                        comp.memory.operation_stack.push_back(VALUE(-arg));
                    } else {
                        display_err("Invalid type for NEGATE operation");
                        return;
                    }
                }, val.data);
            }

            case TOKEN_T::BYTECODE_MAKE_BLOCK:{
                comp.memory.current_stack_amount++;
                
                i++;
                break;
            }// problem with got

            case TOKEN_T::BYTECODE_GOTO_IF_ZERO: {
                if (comp.memory.operation_stack.empty()) {
                    display_err("Stack underflow on GOTOZERO");
                }
                VALUE val = comp.memory.operation_stack.back();
             //   std::cout<<"popping in gotoifzero: ";
                //print_value(comp.memory.operation_stack.back());
              //  std::cout<<'\n';
                comp.memory.operation_stack.pop_back();
               // print_value(val);
                
                bool is_zero = false;
                std::visit([&](auto&& arg){
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr(std::is_same_v<T, int> || std::is_same_v<T, double>) {
                        is_zero = (arg == 0);
                    } else {
                        display_err("Invalid type for GOTOZERO");
                    }
                }, val.data);

                if (is_zero) {
                    auto it = comp.lex.goto_positions.find(tok.value);
                    if (it != comp.lex.goto_positions.end()) {
                        i = it->second;
                    } else {
                        display_err("Label '" + tok.value + "' not found for GOTOZERO");
                    }
                } else {
                    i++;
                }
                break;
            }


            case TOKEN_T::BYTECODE_STORE:
                if (i + 1 < comp.lex.tokens.size()) {
                    std::string var_name = comp.lex.tokens[i + 1].value;
                    if (!comp.memory.operation_stack.empty()) {
                        VALUE val = comp.memory.operation_stack.back();
                     //   std::cout<<"popping back on store:";
                     //   print_value(comp.memory.operation_stack.back());
                     //   std::cout<<"\n";
                        comp.memory.operation_stack.pop_back();
                        comp.memory.set(var_name, val);
                    } else display_err("Stack underflow on STORE");
                    i += 2;
                    
                } else display_err("STORE missing variable name");
                break;

            case TOKEN_T::BYTECODE_LOAD:
                if (i + 1 < comp.lex.tokens.size()) {
                    std::string var_name = comp.lex.tokens[i + 1].value;
                    if (comp.memory.exists(var_name)){
                        comp.memory.operation_stack.push_back(comp.memory.get(var_name));
                     //   print_value(comp.memory.get(var_name));  
                        
                    }
                    else {display_err("Variable '" + var_name + "' not defined");}
                    i += 2;
                } else display_err("LOAD missing variable name");
                break;

            case TOKEN_T::BYTECODE_GOTO: {
            //    std::cout<<"goto\n";
                auto it = comp.lex.goto_positions.find(tok.value);
                if (it != comp.lex.goto_positions.end()) {
                    i = it->second+1;  // jump directly to label
                   
                    break;
                } else display_err("Label '" + tok.value + "' not found");
                i++;
                break;
            }

            case TOKEN_T::BYTECODE_SAFETY:
                i++;
                break;

            case TOKEN_T::BYTECODE_GOTO_LABEL:
                i++; // just skip the label token
                break;

            case TOKEN_T::BYTECODE_ADD:
            case TOKEN_T::BYTECODE_SUB:
            case TOKEN_T::BYTECODE_MUL:
            case TOKEN_T::BYTECODE_DIV:
                
                
                
                if (comp.memory.operation_stack.size() < 2) {
                    
                    print_value(comp.memory.operation_stack.back());
                    display_err("Stack underflow on +,-,*,/ operation, expected two elements on stack got: "+std::to_string(comp.memory.operation_stack.size()));
                    i++;
                    break;
                }
                {
                    VALUE b = comp.memory.operation_stack.back(); comp.memory.operation_stack.pop_back();
                    VALUE a = comp.memory.operation_stack.back(); comp.memory.operation_stack.pop_back();

                    std::visit([&](auto&& arg_a, auto&& arg_b) {
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
                        } else display_err("Type mismatch in arithmetic operation");
                    }, a.data, b.data);

                    i++;
                }
                break;

            case TOKEN_T::BYTECODE_GTE:
            case TOKEN_T::BYTECODE_EQ:
            case TOKEN_T::BYTECODE_NOTEQ:
            case TOKEN_T::BYTECODE_LTE:
            case TOKEN_T::BYTECODE_LT:
            case TOKEN_T::BYTECODE_GT:
            {
                auto do_comparison = [&](TOKEN_T tok_type) {
                    if (comp.memory.operation_stack.size() < 2) {
                        print_value(comp.memory.operation_stack.back());    
                        display_err("Stack underflow on comparison operation");
                        return;
                    }

                    VALUE b = comp.memory.operation_stack.back(); comp.memory.operation_stack.pop_back();
                    VALUE a = comp.memory.operation_stack.back(); comp.memory.operation_stack.pop_back();

                    

                    bool result = false;

                    std::visit([&](auto&& arg_a, auto&& arg_b) {
                        using TA = std::decay_t<decltype(arg_a)>;
                        using TB = std::decay_t<decltype(arg_b)>;

                        // Numeric comparison
                        if constexpr ((std::is_same_v<TA,int> || std::is_same_v<TA,double>) &&
                                    (std::is_same_v<TB,int> || std::is_same_v<TB,double>)) {
                            double val_a = static_cast<double>(arg_a);
                            double val_b = static_cast<double>(arg_b);

                         //   std::cout<<"\ncomp: "<<val_a<<" "<<val_b<<"\n";

                            switch(tok_type) {
                                case TOKEN_T::BYTECODE_EQ:      result = val_a == val_b; break;
                                case TOKEN_T::BYTECODE_NOTEQ:   result = val_a != val_b; break;
                                case TOKEN_T::BYTECODE_GT:      result = val_a >  val_b; break;
                                case TOKEN_T::BYTECODE_GTE:     result = val_a >= val_b; break;
                                case TOKEN_T::BYTECODE_LT:      result = val_a <  val_b; break;
                                case TOKEN_T::BYTECODE_LTE:     result = val_a <= val_b; break;
                                default: break;
                            }
                        }
                        // String comparison
                        else if constexpr (std::is_same_v<TA,std::string> && std::is_same_v<TB,std::string>) {
                            switch(tok_type) {
                                case TOKEN_T::BYTECODE_EQ:      result = arg_a == arg_b; break;
                                case TOKEN_T::BYTECODE_NOTEQ:   result = arg_a != arg_b; break;
                                case TOKEN_T::BYTECODE_GT:      result = arg_a >  arg_b; break;
                                case TOKEN_T::BYTECODE_GTE:     result = arg_a >= arg_b; break;
                                case TOKEN_T::BYTECODE_LT:      result = arg_a <  arg_b; break;
                                case TOKEN_T::BYTECODE_LTE:     result = arg_a <= arg_b; break;
                                default: break;
                            }
                        }
                        else {
                            display_err("Type mismatch in comparison operation");
                        }
                    }, a.data, b.data);

                    comp.memory.operation_stack.push_back(VALUE(result ? 1 : 0));
                };

                do_comparison(tok.type); 
                i++;
                break;
            }


            case TOKEN_T::BYTECODE_POP:
                if (!comp.memory.operation_stack.empty())
                    comp.memory.operation_stack.pop_back();
                else display_err("Stack underflow on POP");
                i++;
                break;

            case TOKEN_T::BYTECODE_POP_ALL:
                comp.memory.operation_stack.clear();
                i++;
                break;
            case TOKEN_T::BYTECODE_CLEANUP:
                comp.memory.stack_vals.clear();
                comp.memory.operation_stack.clear();
                comp.memory.values.clear();
                comp.memory.id_for_value.clear();
                i++;
                break;

            default:
                i++;
                break;
        }
    }
}

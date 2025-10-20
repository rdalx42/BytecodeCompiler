#include "../headers/runtime/err.h"
#include "compiler.h"
#include <iostream>


COMPILER init(AST& ast, LEXER& lex) {
    COMPILER comp(ast, lex);
    //comp.memory.stack_vals.clear();
    comp.memory.operation_stack.init(MAX_OP_STACK);
    comp.lex.pre_calc_stack.resize(MAX_VALS);
    comp.lex.pre_calc_stack.reserve(MAX_VALS);

    comp.memory.fast_operation_temporary_slots.fots_str.value =  new char[MAX_STRING_LEN];
    comp.memory.fast_operation_temporary_slots.fots_str.value[0] = '\0';
   
    comp.bytecode.clear();
    
    
    return comp;
}
// generate_bytecode

/*
Generate bytecode && precompute builtin control flow structures
*/
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

        case AST_STRING:
            comp.bytecode += "PUSH \"" + nd->value + "\"\n";
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

        case AST_FOR: {
            int this_for = builtin_goto_counter++;

            AST_NODE* loop_var = nd->children[0];  // i
            AST_NODE* start_val = nd->children[1]; // start
            AST_NODE* end_val = nd->children[2];   // end
            AST_NODE* body = nd->children[3];      // BLOCK_START

            std::string start_label = "BUILTIN_FOR_START_" + std::to_string(this_for);
            std::string end_label   = "BUILTIN_FOR_END_" + std::to_string(this_for);

            // Initialize loop variable
            generate_bytecode(comp, start_val);
            comp.bytecode += "STORE " + loop_var->value + "\n";

            // Loop start label
            comp.bytecode += ">" + start_label + "\n";
            comp.bytecode += "SAFETY_LABEL\n";

            // Push loop variable and end value to compare
            comp.bytecode += "LOAD " + loop_var->value + "\n";
            generate_bytecode(comp, end_val);
            comp.bytecode += "LTE\n";                 // if i <= end
            comp.bytecode += "GOTOZERO_" + end_label + "\n"; // exit loop if false

            // Loop body
            comp.bytecode += "BLOCK_START\n";
            generate_bytecode(comp, body);
            comp.bytecode += "BLOCK_END\n";

            // Increment loop variable
            comp.bytecode += "LOAD " + loop_var->value + "\n";
            comp.bytecode += "PUSH 1\n";
            comp.bytecode += "ADD\n";
            comp.bytecode += "STORE " + loop_var->value + "\n";

            // Jump back to start
            comp.bytecode += "GOTO_" + start_label + "\n";

            // End label
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
        if (nd->type != AST_BIN_OP && nd->type != AST_VAR_ASSIGN && nd->type != AST_UN_OP){
            generate_bytecode(comp, child);
        }
    }

    
}

void print_value(const VALUE& val) {
    
    switch (val.type) {
        case INT_VAL:
            std::cout << val.int_val;
            break;
        case FLOAT_VAL:
            std::cout << val.float_val;
            break;
        case STR_VAL:
          //  if(val.str_component.has_value()){
                std::cout<<val.str_val->value;
          //  }else{
          //      display_err("String value doesn't have a string component");
         //       return;
           // }
            break;
        case VEC_VAL:
            val.arr_val->print_arr();
            break;
        default:
            std::cout << "unknown";
            break;
    }
}

void compile_ast_to_bytecode(COMPILER& comp) {
    generate_bytecode(comp, comp.ast.root);
}

void compile(COMPILER& comp) {
    size_t i = 0;
   // std::cout<<"ALL TOKENS:\n";
    //display_lex(comp.lex);
    while (i < comp.lex.tokens.size()) {
        TOKEN& tok = comp.lex.tokens[i];
       // if(tok.value!="newline"){
       //     std::cout<<tok.value<<"\n";}

        switch (tok.type) {
            
            case TOKEN_T::BYTECODE_TOP:
                std::cout << "Stack top: ";
                if (!comp.memory.operation_stack.is_empty()==true){
                    print_value(comp.memory.operation_stack.get_back());
                    comp.memory.operation_stack.pop_no_return();
                }

                
                std::cout << "\n";
                i++;
                break;

            case TOKEN_T::BYTECODE_PUSH: {
                if (i + 1 >= comp.lex.tokens.size()) {
                    display_err("Expected value after PUSH");
                }

                TOKEN& next = comp.lex.tokens[i + 1];
                VALUE val;

                switch (next.type) {
                    case TOKEN_T::INT:
                        val.type = INT_VAL;
                        val.int_val=*next.int_val;
                        break;

                    case TOKEN_T::FLOAT:
                        val.type = FLOAT_VAL;
                       // i//f (next.double_val.has_value()) {
                            val.float_val = *next.double_val;
              //   } else {
                //            display_err("Invalid float value for PUSH");
                //            return;
                //        }
                        break;

                    case TOKEN_T::STRING:
                       // if (next.str_val.has_value()) {
                            val.type = STR_VAL;
                            val.str_val = new FAST_STRING_COMPONENT();
                            val.str_val->value = strdup(next.str_val->c_str());
                            
                     //   } else {
                     //       display_err("Invalid string value for PUSH");
                     //       return;
                     //   }
                        break;

                    case TOKEN_T::IDENTIFIER:
                        val = comp.memory.get(tok);
                        break;

                    default:
                        display_err("Expected value after PUSH");
                        return;
                }

                comp.memory.operation_stack.push(val);
                i += 2;
                break;
            }



            case TOKEN_T::BYTECODE_DEL_BLOCK:{
            //    std::cout<<"DEL\n";
                if(comp.memory.current_stack_amount<=0){
                    display_err("Cannot delete block, block already stack empty");
                }else if(comp.memory.current_stack_amount>0){
                    comp.memory.del_stack();
                    comp.memory.current_stack_amount--;
                }i++;
                break;
            }


            case TOKEN_T::BYTECODE_MAKE_BLOCK:{
               // std::cout<<"BLOCK MADE\n";
                comp.memory.current_stack_amount++;
                
                i++;
                break;
            }// problem with got

            case TOKEN_T::BYTECODE_GOTO_IF_ZERO: {
                if (comp.memory.operation_stack.is_empty()) {
                    display_err("Stack underflow on GOTOZERO");
                }
                VALUE val = comp.memory.operation_stack.get_back();
             //   std::cout<<"popping in gotoifzero: ";
                //print_value(comp.memory.operation_stack.back());
              //  std::cout<<'\n';
              //  comp.memory.free_value(val);
                comp.memory.operation_stack.pop();
               // print_value(val);
                
                bool is_zero = false;

                if (val.type == INT_VAL) {
                    is_zero = (val.int_val == 0);
                } else if (val.type == FLOAT_VAL) {
                    is_zero = (val.float_val == 0.0);
                } else {
                    display_err("Invalid type for GOTOZERO");
                }


                if (is_zero) {
                    int pos = *tok.jump_pos;
                    i=pos; // has already been validated to exist
                    
                } else {
                    i++;
                }
                break;
            }


            case TOKEN_T::BYTECODE_STORE:
                if (i + 1 < comp.lex.tokens.size()) {
                   
                    if (!comp.memory.operation_stack.is_empty()) {
                        VALUE val = comp.memory.operation_stack.pop(); // pop returns VALUE
                        comp.memory.set(tok, val);
                    } else {
                        display_err("Stack underflow on STORE");
                    }
                    i += 2;
                    
                } else display_err("STORE missing variable name");
                break;

            case TOKEN_T::BYTECODE_LOAD:{
                //if (i + 1 < comp.lex.tokens.size()) {
                   // std::string var_name = comp.lex.tokens[i + 1].value;
                    //if (comp.memory.exists(var_name)){
                        comp.memory.operation_stack.push(comp.memory.get(comp.lex.tokens[i]));
                     //   print_value(comp.memory.get(var_name));  
                        
                  //  }
                  //  else {display_err("Variable '" + var_name + "' not defined");}
                    i += 2;
               // } else display_err("LOAD missing variable name");
                break;
            }

            case TOKEN_T::BYTECODE_GOTO: {
            //    std::cout<<"goto\n";
                
                    int pos = *tok.jump_pos; //
                    int target_depth = *tok.scope_level;
                   // std::cout<<comp.memory.current_stack_amount<<"  "<<target_depth<<"\n";
                    while (comp.memory.current_stack_amount > target_depth) { // delete scopes
                        comp.memory.del_stack();
                        comp.memory.current_stack_amount--;
                    }

                    i = pos+1;  // jump directly to label
                   
                    break;
                
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
                
                if (comp.memory.operation_stack.sz() < 2) {
                    
                    print_value(comp.memory.operation_stack.get_back());
                    display_err("Stack underflow on +,-,*,/ operation, expected two elements on stack got: "+std::to_string(comp.memory.operation_stack.sz()));
                    i++;
                    break;
                }
                {
                    VALUE b = comp.memory.operation_stack.get_back(); comp.memory.operation_stack.pop_no_return();
                    VALUE a = comp.memory.operation_stack.get_back(); comp.memory.operation_stack.pop_no_return();

                   // comp.memory.free_value(a);
                   // comp.memory.free_value(b);

                    double result = 0;

                    if ((a.type == INT_VAL || a.type == FLOAT_VAL) &&
                        (b.type == INT_VAL || b.type == FLOAT_VAL)) {
                        
                        double val_a = (a.type == INT_VAL) ? a.int_val : a.float_val;
                        double val_b = (b.type == INT_VAL) ? b.int_val : b.float_val;

                        switch(tok.type) {
                            case TOKEN_T::BYTECODE_ADD: result = val_a + val_b; break;
                            case TOKEN_T::BYTECODE_SUB: result = val_a - val_b; break;
                            case TOKEN_T::BYTECODE_MUL: result = val_a * val_b; break;
                            case TOKEN_T::BYTECODE_DIV:
                                if (val_b == 0) display_err("Division by zero");
                                result = val_a / val_b;
                                break;
                            default: break;
                        }

                        VALUE res_val;
                        if (a.type == FLOAT_VAL || b.type == FLOAT_VAL) {
                            res_val.type = FLOAT_VAL;
                            res_val.float_val = result;
                        } else {
                            res_val.type = INT_VAL;
                            res_val.int_val = (int)result;
                        }

                        comp.memory.operation_stack.push(res_val);

                    }else if(a.type==STR_VAL && b.type==STR_VAL && tok.type==TOKEN_T::BYTECODE_ADD) {
                        VALUE res_val;
                        res_val.type = STR_VAL;
                        res_val.str_val = new FAST_STRING_COMPONENT();
                        res_val.str_val->value = new char[MAX_STRING_LEN]; // allocate memory

                        comp.memory.concat_fast_string_safe(*res_val.str_val, *a.str_val, *b.str_val); // copy+concat

                        comp.memory.operation_stack.push(res_val); // safe

                    }else {
                        display_err("Type mismatch in arithmetic operation");
                    }

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
                VALUE b = comp.memory.operation_stack.get_back(); comp.memory.operation_stack.pop_no_return();
                VALUE a = comp.memory.operation_stack.get_back(); comp.memory.operation_stack.pop_no_return();
             //   comp.memory.free_value(a);
               // comp.memory.free_value(b);

                bool result = false;

                if ((a.type == INT_VAL || a.type == FLOAT_VAL) && (b.type == INT_VAL || b.type == FLOAT_VAL)) {
                    double val_a = (a.type == INT_VAL) ? a.int_val : a.float_val;
                    double val_b = (b.type == INT_VAL) ? b.int_val : b.float_val;

                    switch(tok.type) {
                        case TOKEN_T::BYTECODE_EQ:    result = val_a == val_b; break;
                        case TOKEN_T::BYTECODE_NOTEQ: result = val_a != val_b; break;
                        case TOKEN_T::BYTECODE_GT:    result = val_a >  val_b; break;
                        case TOKEN_T::BYTECODE_GTE:   result = val_a >= val_b; break;
                        case TOKEN_T::BYTECODE_LT:    result = val_a <  val_b; break;
                        case TOKEN_T::BYTECODE_LTE:   result = val_a <= val_b; break;
                        default: break;
                    }
                }
                // String comparison
                else if (a.type == STR_VAL && b.type == STR_VAL) {
                    const std::string& val_a = a.str_val->value;
                    const std::string& val_b = b.str_val->value;

                    switch(tok.type) {
                        case TOKEN_T::BYTECODE_EQ:    result = val_a == val_b; break;
                        case TOKEN_T::BYTECODE_NOTEQ: result = val_a != val_b; break;
                        case TOKEN_T::BYTECODE_GT:    result = val_a >  val_b; break;
                        case TOKEN_T::BYTECODE_GTE:   result = val_a >= val_b; break;
                        case TOKEN_T::BYTECODE_LT:    result = val_a <  val_b; break;
                        case TOKEN_T::BYTECODE_LTE:   result = val_a <= val_b; break;
                        default: break;
                    }
                } 
                else {
                    display_err("Type mismatch in comparison operation");
                }

                VALUE res_val;
                res_val.type = INT_VAL; // comparisons return 0 or 1
                res_val.int_val = result ? 1 : 0;
                comp.memory.operation_stack.push(res_val);
                i++;

                break;
            }


            case TOKEN_T::BYTECODE_POP:
                if (!comp.memory.operation_stack.is_empty()){
                    comp.memory.operation_stack.pop_no_return();
                   // comp.memory.free_value(comp.memory.operation_stack.get_back());
                }
                else {display_err("Stack underflow on POP");}
                i++;
                break;

            case TOKEN_T::BYTECODE_POP_ALL:
                comp.memory.operation_stack.destroy();
                i++;
                break;
            case TOKEN_T::BYTECODE_CLEANUP:
              //  comp.memory.stack_vals.clear();
                comp.memory.operation_stack.destroy();
               // comp.memory.values.clear();
               // comp.memory.id_for_value.clear();
                i++;
                break;

            default:
                i++;
                break;
        }
    }
}

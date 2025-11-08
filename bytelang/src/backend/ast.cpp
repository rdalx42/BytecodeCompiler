#include "../headers/backend/ast.h"
#include "../headers/runtime/err.h"
#include <iostream>

AST_NODE* create_node(NODE_TYPE type, std::string value) {
    AST_NODE* node = new AST_NODE();
    node->type = type;
    node->value = value;
    return node;
}

AST_NODE* parse_comparison(AST& ast, int& index);
AST_NODE* parse_expression(AST& ast, int& index);
AST_NODE* parse_term(AST& ast, int& index);
AST_NODE* parse_factor(AST& ast, int& index);
AST_NODE* parse_primary(AST& ast, int& index);

AST_NODE* parse_program(AST& ast) {
    AST_NODE* program_node = create_node(AST_PROGRAM, "PROGRAM");
    int index = 0;
    while (index < ast.tokens.size()) {
        TOKEN& tok = ast.tokens[index];
        if (tok.type == NEWLINE) {
            index++;
            continue;
        }
        AST_NODE* stmt = parse_comparison(ast, index);
        if (stmt) program_node->children.push_back(stmt);
        else {
            AST_NODE* none_node = create_node(AST_NONE, "NONE");
            program_node->children.push_back(none_node);
            index++;
        }
    }
    return program_node;
}

AST_NODE* parse_primary(AST& ast, int& index) {
    
    static int function_call_depth_amount = 0;
    
    if (index >= ast.tokens.size()) return create_node(AST_NONE, "NONE");
    TOKEN& tok = ast.tokens[index];
    AST_NODE* node = nullptr;
    
    if (tok.type == INT) {
        node = create_node(AST_INT, tok.value);
        index++;
    }else if(tok.type == GOTO_LABEL){
        node = create_node(AST_GOTO_LABEL,tok.value);
        index++;
    }else if (tok.type == FLOAT) {
        node = create_node(AST_FLOAT, tok.value);
        index++;
    }else if(tok.type == STRING){
        node = create_node(AST_STRING,tok.value);
        index++;
    }else if (tok.type == IDENTIFIER) {
        node = create_node(AST_VAR_ACCESS, tok.value);
        index++;

        if (index < ast.tokens.size() && ast.tokens[index].type == LP) { // parse function call
            index++; 
            AST_NODE* call_node = create_node(AST_FUNC_CALL, tok.value);
            
            function_call_depth_amount++;
            
            
            while (index < ast.tokens.size() && ast.tokens[index].type != RP) {
                AST_NODE* arg = parse_comparison(ast, index);
                call_node->children.push_back(arg);

                if (index < ast.tokens.size() && ast.tokens[index].type == COMMA) {
                    index++;
                } else if (index < ast.tokens.size() && ast.tokens[index].type != RP) {
                    display_err("Expected ',' or ')' in argument list for function: " + tok.value);
                    return nullptr;
                }
            }
            if (index < ast.tokens.size() && ast.tokens[index].type == RP) {
                index++;
            } else {
                display_err("Expected ')' after function arguments for function: " + tok.value);
                return nullptr;
            }

            
            node = call_node; // replace node with call node
            if (function_call_depth_amount > 1) {  // <-- function call found within params has to not be garbage collected
                node->string_content = "unused_function_true";
            }//else if(function_call_depth_amount==1){ // <-- avoid from beeing called unused
                //node->string_content = "used_function_at_root";
           // }
            function_call_depth_amount--;
        }

        while (index < ast.tokens.size() && ast.tokens[index].value == "[") {
            index++; 
            AST_NODE* index_expr = parse_comparison(ast, index);
            if (index >= ast.tokens.size() || ast.tokens[index].value != "]") {
                display_err("Expected ']' after index expression for variable " + tok.value);
                return nullptr;
            }
            index++; 

            AST_NODE* array_node = nullptr;

            if (index < ast.tokens.size() && ast.tokens[index].value == "=") {
                array_node = create_node(AST_VAR_ASSIGN_AT, node->value);
            } else {
                array_node = create_node(AST_VAR_ACCESS_AT, node->value);
            }

            array_node->children.push_back(node);      // variable
            array_node->children.push_back(index_expr); // index
            node = array_node;
        }

        if (index < ast.tokens.size() && ast.tokens[index].value == "=") {
            index++;

            AST_NODE* rhs = parse_comparison(ast, index);

            if (node->type == AST_VAR_ASSIGN_AT) {
                
                AST_NODE* assign_expr_node = create_node(AST_VAR_ASSIGN, "ASSIGN_EXPR");
                //AST_NODE* expression_of_paren = create_node(AST_)
                assign_expr_node->children.push_back(rhs);
                node->children.push_back(assign_expr_node);
            } else {
                
                AST_NODE* assign_node = create_node(AST_VAR_ASSIGN, "=");
                assign_node->children.push_back(node);
                assign_node->children.push_back(rhs);
                node = assign_node;
            }
        }
    } else if (tok.type == LP) {
        index++;
        node = parse_comparison(ast, index);
        if (!node) node = create_node(AST_NONE, "NONE");
        if (index < ast.tokens.size() && ast.tokens[index].type == RP) index++;
    } else if (tok.type == KEYWORD) {
        if (tok.value == "top") {
            node = create_node(AST_TOP, "top");
            index++;
        }else if(tok.value == "goto"){
            node = create_node(AST_GOTO,"goto");
            index++;
            if(index < ast.tokens.size() && ast.tokens[index].type==GOTO_LABEL){
                AST_NODE* label_node = create_node(AST_GOTO_LABEL,ast.tokens[index].value);
                node->value = label_node->value;
                index++;
            }else{
                // error
                std::cout<<"Error: Expected label after 'goto'\n";
                node->children.push_back(create_node(AST_NONE,"NONE"));
            }
        }else if(tok.value=="do"){
            node = create_node(AST_BLOCK_START,"BLOCK_START");
            index++;
        }else if(tok.value=="end"){
            node = create_node(AST_BLOCK_END,"BLOCK_END");
            index++;
        }else if (tok.value == "if") {
            node = create_node(AST_IF, "IF");
            index++;

            // parse the condition
            node->children.push_back(parse_comparison(ast, index));

            
            if (index < ast.tokens.size() && ast.tokens[index].value == "do") {
                index++;
                AST_NODE* then_block = create_node(AST_BLOCK_START, "don't add");
                while (index < ast.tokens.size() && ast.tokens[index].value != "end" && ast.tokens[index].value != "else") {
                    then_block->children.push_back(parse_comparison(ast, index));
                }
                
                if(index==ast.tokens.size()&&ast.tokens[index].value!="end"&&ast.tokens[index].value!="else"){
                    display_err("Expected 'end' or 'else' branch after if block");
                    return nullptr;
                }

                node->children.push_back(then_block);
            }else{
                display_err("Expected 'do' after if condition");
                return nullptr;
            }

            
            if (index < ast.tokens.size() && ast.tokens[index].value == "else") {
                index++; 
                AST_NODE* else_block = create_node(AST_ELSE, "ELSE");
                while (index < ast.tokens.size() && ast.tokens[index].value != "end") {
                    else_block->children.push_back(parse_comparison(ast, index));
                }

                if(index==ast.tokens.size()&&ast.tokens[index].value!="end"){
                    display_err("Expected 'end' after 'else' branch block");
                    return nullptr;
                }

                node->children.push_back(else_block);
            }

         
            if (index < ast.tokens.size() && ast.tokens[index].value == "end") index++;
        }else if(tok.value=="while"){
            node=create_node(AST_WHILE,"WHILE");
            index++;
            node->children.push_back(parse_comparison(ast,index));

            if(index<ast.tokens.size() && ast.tokens[index].value=="do"){
                index++;
                AST_NODE* while_block = create_node(AST_BLOCK_START,"don't add");
                while(index < ast.tokens.size() && ast.tokens[index].value != "end"){
                    while_block->children.push_back(parse_comparison(ast,index));
                }
                if(index==ast.tokens.size()&&ast.tokens[index].value!="end"){
                    display_err("Expected 'end' after while block");
                    return nullptr;
                }
                node->children.push_back(while_block);
                index++; // skip 'end'
            }else{
                display_err("Expected 'do' after while condition");
                return nullptr;
            }

        }else if(tok.value=="for"){
            node=create_node(AST_FOR,"FOR");
            index++;

            if(index < ast.tokens.size() && ast.tokens[index].type == IDENTIFIER){
                AST_NODE* loop_var_node = create_node(AST_VAR_ACCESS, ast.tokens[index].value);
                node->children.push_back(loop_var_node);
                index++;
            } else {
                display_err("Expected loop variable after 'for' keyword");
                return nullptr;
            }

            AST_NODE* start_node = nullptr;
            if(index < ast.tokens.size() && ast.tokens[index].value == "="){
                index++; // skip '='
                start_node = parse_comparison(ast, index);
                node->children.push_back(start_node);
            } else {
            
                start_node = create_node(AST_INT, "0");
                node->children.push_back(start_node);
            }
            if(index < ast.tokens.size() && ast.tokens[index].type == COMMA){
                index++;
            } else {
                display_err("Expected comma after for loop start value");
                return nullptr;
            }
            if(index >= ast.tokens.size()){
                display_err("Expected end value for for loop");
                return nullptr;
            }
            AST_NODE* end_node = parse_comparison(ast, index);
            node->children.push_back(end_node);

            if(index < ast.tokens.size() && ast.tokens[index].value == "do"){
                index++;
            } else {
                display_err("Expected 'do' in for loop");
                return nullptr;
            }

            AST_NODE* block_node = create_node(AST_BLOCK_START, "don't add");
            while(index < ast.tokens.size() && ast.tokens[index].value != "end"){
                block_node->children.push_back(parse_comparison(ast, index));
            }

            if(index == ast.tokens.size() || ast.tokens[index].value != "end"){
                display_err("Expected 'end' after for block");
                return nullptr;
            }
            index++; // skip 'end'

            node->children.push_back(block_node);
        }else if(tok.value == "func"){
            
            node = create_node(AST_FUNCTION_DECL, "FUNC_DECL");
            index++;

            if (index < ast.tokens.size() && ast.tokens[index].type == IDENTIFIER) {
                node->value = ast.tokens[index].value;
                index++;
            } else {
                display_err("Expected identifier after 'func' keyword");
                return nullptr;
            }

            if (index < ast.tokens.size() && ast.tokens[index].type == LP) {
                index++;
            } else {
                display_err("Expected '(' after function name: " + node->value);
                return nullptr;
            }
            AST_NODE* params_node = create_node(AST_FUNC_PARAMS, "PARAMS");

            while (index < ast.tokens.size() && ast.tokens[index].type != RP) {
                if (ast.tokens[index].type == IDENTIFIER) {
                    AST_NODE* param = create_node(AST_IDENTIFIER, ast.tokens[index].value);
                    params_node->children.push_back(param);
                    index++;
                } else if (ast.tokens[index].type == COMMA) {
                    index++;
                } else {
                    display_err("Unexpected token in parameter list for function: " + node->value);
                    return nullptr;
                }
            }

            if (index < ast.tokens.size() && ast.tokens[index].type == RP) {
                index++;
            } else {
                display_err("Expected ')' after parameter list in function: " + node->value);
                return nullptr;
            }

            node->children.push_back(params_node);

            if (index < ast.tokens.size() && ast.tokens[index].value == "do") {
                index++;
            } else {
                display_err("Expected 'do' after function header: " + node->value);
                return nullptr;
            }

            AST_NODE* body_node = create_node(AST_BLOCK_START, "FUNC_BODY");
            while (index < ast.tokens.size() && ast.tokens[index].value != "end") {
                AST_NODE* stmt = parse_comparison(ast, index);
                if (stmt) body_node->children.push_back(stmt);
                else index++; 
            }

            if (index >= ast.tokens.size() || ast.tokens[index].value != "end") {
                display_err("Expected 'end' to close function: " + node->value);
                return nullptr;
            }

            index++;

            node->children.push_back(body_node);
        }else if(tok.value == "return"){
            node = create_node(AST_RETURN, "RETURN");
            index++;

            if(index >= ast.tokens.size() || 
            ast.tokens[index].type == NEWLINE || 
            ast.tokens[index].value == "end") 
            {
                return node;
            }

            AST_NODE* return_expr = parse_comparison(ast, index);
            node->children.push_back(return_expr);
        }



        } else {
            node = create_node(AST_NONE, "NONE");
            index++;
        }
        return node;
    }

AST_NODE* parse_factor(AST& ast, int& index) {
    if (index < ast.tokens.size() && ast.tokens[index].value == "-") {
        AST_NODE* neg_node = create_node(AST_UN_OP, "NEG");
        index++;
        neg_node->children.push_back(parse_factor(ast, index));
        return neg_node;
    }else if(index<ast.tokens.size()&&ast.tokens[index].value == "+"){
        AST_NODE* add_neg_node = create_node(AST_UN_OP,"ADD_NEG");
        index++;
        add_neg_node->children.push_back(parse_factor(ast,index));
        return add_neg_node;
    }else if(index<ast.tokens.size()&&ast.tokens[index].value == "!"){
        AST_NODE* add_neg_node = create_node(AST_UN_OP,"NOT");
        index++;
        add_neg_node->children.push_back(parse_factor(ast,index));
        return add_neg_node;
    }
    return parse_primary(ast, index);
}

AST_NODE* parse_term(AST& ast, int& index) {
    AST_NODE* node = parse_factor(ast, index);
    while (index < ast.tokens.size() && (ast.tokens[index].value == "*" || ast.tokens[index].value == "/")) {
        AST_NODE* op_node = create_node(AST_BIN_OP, ast.tokens[index].value);
        index++;
        op_node->children.push_back(node);
        op_node->children.push_back(parse_factor(ast, index));
        node = op_node;
    }
    return node;
}

AST_NODE* parse_expression(AST& ast, int& index) {
    AST_NODE* node = parse_term(ast, index);
    while (index < ast.tokens.size() && (ast.tokens[index].value == "+" || ast.tokens[index].value == "-")) {
        AST_NODE* op_node = create_node(AST_BIN_OP, ast.tokens[index].value);
        index++;
        op_node->children.push_back(node);
        op_node->children.push_back(parse_term(ast, index));
        node = op_node;
    }
    return node;
}

AST_NODE* parse_comparison(AST& ast, int& index) {
    AST_NODE* node = parse_expression(ast, index);
    while (index < ast.tokens.size()) {
        std::string op = ast.tokens[index].value;
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            AST_NODE* op_node = create_node(AST_BIN_OP, op);
            index++;
            op_node->children.push_back(node);
            op_node->children.push_back(parse_expression(ast, index));
            node = op_node;
        } else break;
    }
    return node;
}

AST* build_ast(AST& ast) {
    AST* tree = new AST();
    int index = 0;
    tree->root = parse_program(ast);
    return tree;
}

std::string node_type_to_string(NODE_TYPE& type) {
    switch (type) {
        case AST_VAR_ACCESS_AT: return "ACCESS_AT";
        case AST_TOP: return "TOP";
        case AST_FUNC_CALL: return "FUNCTION_CALL";
        case AST_UN_OP: return "UNARY_OP";
        case AST_BIN_OP: return "BINARY_OP";
        case AST_INT: return "INT";
        case AST_FUNCTION_DECL: return "FUNCTION_DECL";
        case AST_FUNC_PARAMS: return "PARAMS";
        case AST_RETURN: return "RETURN";
        case AST_STRING: return "STRING";
        case AST_FLOAT: return "FLOAT";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_VAR_DECL: return "VAR_DECL";
        case AST_VAR_ASSIGN: return "VAR_ASSIGN";
        case AST_NEWLINE: return ";";
        case AST_VAR_ACCESS: return "VAR_ACCESS";
        case AST_PROGRAM: return "PROGRAM";
        case AST_IF: return "IF";
        case AST_FOR: return "FOR";
        case AST_ELSE: return "ELSE";
        case AST_NONE: return "";
        case AST_VAR_ASSIGN_AT: return "ASSIGN_AT";
        case AST_GOTO_LABEL : return "GOTO_LABEL";
        case AST_BLOCK_END : return "BLOCK_END";
        case AST_BLOCK_START : return "BLOCK_START";
        case AST_BUILTIN_BYTECODE_NODE: return "BYTECODE SEQ";
        case AST_GOTO : return "GOTO";
        case AST_WHILE: return "WHILE";
        default: return "UNKNOWN";
    }
}

void print_ast_node(AST_NODE* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) std::cout << "  ";
    std::cout << node->value << " (" << node_type_to_string(node->type) << ")\n";
    for (AST_NODE* child : node->children) print_ast_node(child, depth + 1);
}

void print_ast(AST& ast) {
    print_ast_node(ast.root);
}





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
    } else if (tok.type == IDENTIFIER) {
        node = create_node(AST_VAR_ACCESS, tok.value);
        index++;
        if (index < ast.tokens.size() && ast.tokens[index].value == "=") {
            AST_NODE* assign_node = create_node(AST_VAR_ASSIGN, "=");
            assign_node->children.push_back(node);
            index++;
            assign_node->children.push_back(parse_comparison(ast, index));
            node = assign_node;
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
                AST_NODE* then_block = create_node(AST_BLOCK_START, "BLOCK_START");
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
            }else{
                display_err("Expected 'do' after while condition");
                return nullptr;
            }

        }else{
            node = create_node(AST_NONE, "NONE");
            index++;
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
        
        
        case AST_TOP: return "TOP";
        case AST_UN_OP: return "UNARY_OP";
        case AST_BIN_OP: return "BINARY_OP";
        case AST_INT: return "INT";
        case AST_FLOAT: return "FLOAT";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_VAR_DECL: return "VAR_DECL";
        case AST_VAR_ASSIGN: return "VAR_ASSIGN";
        case AST_NEWLINE: return ";";
        case AST_VAR_ACCESS: return "VAR_ACCESS";
        case AST_PROGRAM: return "PROGRAM";
        case AST_IF: return "IF";
        case AST_ELSE: return "ELSE";
        case AST_NONE: return "";
        case AST_GOTO_LABEL : return "GOTO_LABEL";
        case AST_BLOCK_END : return "BLOCK_END";
        case AST_BLOCK_START : return "BLOCK_START";
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




/*

x=10

while x > 0 do 
    x=x-1
    if x == 5 do 
        goto eol 
    end 
end

::eol::

// this is a comment

fun main() do 
    
    
    return something 
end

for i to 10 do 

end 

if x == 0 do  

else 

end

*/

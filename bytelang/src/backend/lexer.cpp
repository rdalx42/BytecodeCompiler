#include "../headers/backend/lexer.h"
#include "../headers/runtime/err.h"
#include <iostream>

const std::string digits = "0123456789";
const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
int bytecode_tok_cnt = 0;

const std::vector<std::string>bytecode_keywords={"PUSH", "POP", "ADD", "SUB", "MUL", "DIV", "STORE", "LOAD", "POP_ALL", "CLEANUP","TOP","NEG","NOTEQ","EQ","LT","LTE","GT","GTE","GOTO","BLOCK_END","BLOCK_START","SAFETY_LABEL"};
const std::vector<std::string>keywords={"top","goto","do","end","if","else"};

char peek(LEXER& lex,int&index){
    if(index+1 < lex.content.size()){
        return lex.content[index+1];
    }

    return '\0';
}

void donum(LEXER& lex, int& index) {
    int token_index = lex.tokens.size() - 1;
    bool dot_seen = false;
    while (index < lex.content.size()) {
        char current = lex.content[index];
        if (digits.find(current) != std::string::npos) {
            lex.tokens[token_index].value += current;
            index++;
        } else if (current == '.' && !dot_seen) {
            lex.tokens[token_index].value += current;
            lex.tokens[token_index].type = FLOAT;
            dot_seen = true;
            index++;
        } else {
            break;
        }
    }
}

void doidentifier(LEXER& lex, int& index) {
    int token_index = lex.tokens.size() - 1;
    while (index < lex.content.size()) {
        char current = lex.content[index];
        if (characters.find(current) != std::string::npos || digits.find(current) != std::string::npos) {
            lex.tokens[token_index].value += current;
            index++;
        } else {
            break;
        }
    }
}

void dolex(LEXER& lex) {
    int index = 0;
    while (index < lex.content.size()) {
        char current = lex.content[index];
        if (current == ' '  || current=='\t' ) {
            index++;
            continue;
        }if(current=='\n'){
            lex.tokens.push_back({"newline",NEWLINE});
            index++;
            continue;
        }
        switch (current) {
            case '(': lex.tokens.push_back({"(", LP}); index++; continue;
            case ')': lex.tokens.push_back({")", RP}); index++; continue;
            case '+': lex.tokens.push_back({"+", OP}); index++; continue;
            case '-': lex.tokens.push_back({"-", OP}); index++; continue;
            case '*': lex.tokens.push_back({"*", OP}); index++; continue;
            case '/': 
                if(peek(lex,index)=='/'){
                    // comment 
                    while(index < lex.content.size() && lex.content[index] != '\n'){
                        index++;
                    }
                    continue;
                }
                lex.tokens.push_back({"/", OP}); index++; continue;
            case '=': 
                if(peek(lex,index) == '='){
                    lex.tokens.push_back({"==", OP}); index+=2; continue;
                }
                lex.tokens.push_back({"=", OP}); index++; continue;
            
            case '>':
                if(peek(lex,index) == '='){
                    lex.tokens.push_back({">=", OP}); index+=2; continue;
                }

                if(lex.in_bytecode == true && peek(lex,index) != ' '){
                    int token_index = lex.tokens.size();
                    lex.tokens.push_back({"", BYTECODE_GOTO_LABEL}); 
                    lex.tokens[token_index].value.push_back('>');    
                    index++; 
                    
                    doidentifier(lex,index); 
                    lex.goto_positions[lex.tokens[token_index].value.substr(1)] = token_index;
                    std::cout << "Label found: " << lex.tokens[token_index].value.substr(1) << "\n";
                    continue;
                }


                lex.tokens.push_back({">", OP}); index++; continue;
            case '<':
                if(peek(lex,index) == '='){
                    lex.tokens.push_back({"<=", OP}); index+=2; continue;
                }
                lex.tokens.push_back({"<", OP}); index++; continue;
            case '!':
                if(peek(lex,index) == '='){
                    lex.tokens.push_back({"!=", OP}); index+=2; continue;
                }
                
                index++;
                continue;
            case ':':
                if(peek(lex,index)==':'){
                    // goto label
                    
                    index+=2;
                    lex.tokens.push_back({"",GOTO_LABEL});
                    doidentifier(lex,index);
                    
                    
                    continue;
                }
        }
        if (digits.find(current) != std::string::npos) {
            lex.tokens.push_back({"", INT});
            donum(lex, index);
            continue;
        }
        if (characters.find(current) != std::string::npos) {
            lex.tokens.push_back({"", IDENTIFIER});
            doidentifier(lex, index);
            
            if(lex.in_bytecode==true){

                // special bytecode tokens
                
                if(std::find(bytecode_keywords.begin(), bytecode_keywords.end(), (lex.tokens.back().value)) != bytecode_keywords.end()||(lex.tokens.back().value[0]=='>')||(lex.tokens.back().value.substr(0,4) == "GOTO")){
                    std::cout<<"Bytecode token found: "<<lex.tokens.back().value<<"\n";
                    bytecode_tok_cnt++;
                    if(lex.tokens.back().value == "PUSH"){
                        lex.tokens.back().type = BYTECODE_PUSH;
                    }else if(lex.tokens.back().value=="POP"){
                        lex.tokens.back().type = BYTECODE_POP;
                    }else if(lex.tokens.back().value=="ADD"){
                        lex.tokens.back().type = BYTECODE_ADD;
                    }else if(lex.tokens.back().value=="SUB"||lex.tokens.back().value=="NEG"){
                        lex.tokens.back().type = BYTECODE_SUB;
                    }else if(lex.tokens.back().value=="MUL"){
                        lex.tokens.back().type = BYTECODE_MUL;
                    }else if(lex.tokens.back().value=="DIV"){
                        lex.tokens.back().type = BYTECODE_DIV;
                    }else if(lex.tokens.back().value=="STORE"){
                        lex.tokens.back().type = BYTECODE_STORE;
                    }else if(lex.tokens.back().value=="LOAD"){
                        lex.tokens.back().type = BYTECODE_LOAD;
                    }else if(lex.tokens.back().value=="POP_ALL"){
                        lex.tokens.back().type = BYTECODE_POP_ALL;
                    }else if(lex.tokens.back().value=="CLEANUP"){
                        lex.tokens.back().type = BYTECODE_CLEANUP;
                    }else if( lex.tokens.back().value=="TOP"|| lex.tokens.back().value=="top"){
                        lex.tokens.back().type = BYTECODE_TOP;
                    }else if(lex.tokens.back().value == "EQ"){
                        lex.tokens.back().type = BYTECODE_EQ;
                    }else if(lex.tokens.back().value == "NOTEQ"){
                        lex.tokens.back().type = BYTECODE_NOTEQ;
                    }else if(lex.tokens.back().value == "LT"){
                        lex.tokens.back().type = BYTECODE_LT;
                    }else if(lex.tokens.back().value == "LTE"){
                        lex.tokens.back().type = BYTECODE_LTE;
                    }  else if(lex.tokens.back().value == "GT"){
                        lex.tokens.back().type = BYTECODE_GT;
                    }
                    else if(lex.tokens.back().value == "GTE"){
                        lex.tokens.back().type = BYTECODE_GTE;
                    }else if(lex.tokens.back().value[0]=='>'){
                        std::cout<<"Label found: "<<lex.tokens.back().value.substr(1)<<"\n";
                        lex.goto_positions[lex.tokens.back().value.substr(1)]=bytecode_tok_cnt+1;
                       // bytecode_tok_cnt++;
                        lex.tokens.back().type = BYTECODE_GOTO_LABEL;
                    }else if(lex.tokens.back().value.substr(0,4) == "GOTO"){
                        if(lex.tokens.back().value.substr(0,5)=="GOTOZ"){
                            
                            std::string label = lex.tokens.back().value.substr(9);
                            std::cout<<">"<<label<<"\n";
                            lex.tokens.back().value = label;
                            lex.tokens.back().type = BYTECODE_GOTO_IF_ZERO;

                            /*if(lex.goto_positions.find(label) != lex.goto_positions.end()){
                                display_err("Duplicate label found: " + label);
                                return;
                            } -- > we take care of this in the compiler */
                            
                           
                        }else{
                            std::string label = lex.tokens.back().value.substr(5); // skip "GOTO_"
                            lex.tokens.back().value = label;
                            
                            lex.tokens.back().type = BYTECODE_GOTO;                        
                            
                            /*if(lex.goto_positions.find(label) == lex.goto_positions.end()){
                                display_err("Label not found: " + label);
                                return;
                            } -- > we take care of this in the compiler */
                            
                           
                        }
                        
                    }else if(lex.tokens.back().value=="BLOCK_START"){
                        lex.tokens.back().type = BYTECODE_MAKE_BLOCK;
                    }else if(lex.tokens.back().value=="BLOCK_END"){
                        lex.tokens.back().type = BYTECODE_DEL_BLOCK;
                    }else if(lex.tokens.back().value=="SAFETY_LABEL"){
                        lex.tokens.back().type = BYTECODE_SAFETY;
                    }
                }
            }else{
                if(std::find(keywords.begin(), keywords.end(), lex.tokens.back().value) != keywords.end()){
                    lex.tokens.back().type=KEYWORD;
                }
            }
            
            continue;
        }
        display_err(std::string("Unknown character found: ") + current);
        index++;
    }
}

void display_lex(LEXER& lex) {
    std::cout << "[";
    for (size_t i = 0; i < lex.tokens.size(); i++) {
        std::cout << lex.tokens[i].value;
        if (i + 1 != lex.tokens.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    if(lex.in_bytecode==true){
        std::cout << "\nGoto positions:\n";
        for (const auto& pos : lex.goto_positions) {
            std::cout << "  Label: " << pos.first << " -> Token Index: " << pos.second << "\n";
        }
    }
}

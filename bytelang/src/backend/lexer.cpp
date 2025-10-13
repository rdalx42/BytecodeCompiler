#include "../headers/backend/lexer.h"
#include "../headers/runtime/err.h"
#include <iostream>

const std::string digits = "0123456789";
const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

const std::vector<std::string>bytecode_keywords={"PUSH", "POP", "ADD", "SUB", "MUL", "DIV", "STORE", "LOAD", "POP_ALL", "CLEANUP","TOP","NEG"};
const std::vector<std::string>keywords={"top"};

char peek(LEXER& lex){
    if(lex.pos < lex.content.size()){
        return lex.content[lex.pos];
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
                if(peek(lex)=='/'){
                    // comment 
                    while(index < lex.content.size() && lex.content[index] != '\n'){
                        index++;
                    }
                    continue;
                }
                lex.tokens.push_back({"/", OP}); index++; continue;
            case '=': 
                if(peek(lex) == '='){
                    lex.tokens.push_back({"==", OP}); index+=2; continue;
                }
                lex.tokens.push_back({"=", OP}); index++; continue;
            
            case '>':
                if(peek(lex) == '='){
                    lex.tokens.push_back({">=", OP}); index+=2; continue;
                }
                lex.tokens.push_back({">", OP}); index++; continue;
            case '<':
                if(peek(lex) == '='){
                    lex.tokens.push_back({"<=", OP}); index+=2; continue;
                }
                lex.tokens.push_back({"<", OP}); index++; continue;
            case '!':
                if(peek(lex) == '='){
                    lex.tokens.push_back({"!=", OP}); index+=2; continue;
                }
                display_err("Unexpected character '!'");
                index++;
                continue;
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
               
                if(std::find(bytecode_keywords.begin(), bytecode_keywords.end(), (lex.tokens.back().value)) != bytecode_keywords.end()){
                   
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
                    }
                }
            }else{
                if(std::find(keywords.begin(), keywords.end(), lex.tokens.back().value) != keywords.end()){
                    if(lex.tokens.back().value == "top"){
                        lex.tokens.back().type = KEYWORD;
                    }
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
}

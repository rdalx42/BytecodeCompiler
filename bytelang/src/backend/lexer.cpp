#include "../headers/backend/lexer.h"
#include "../headers/runtime/err.h"
#include <iostream>

const std::string digits = "0123456789";
const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
int bytecode_tok_cnt = 0;

const std::vector<std::string>bytecode_keywords={"PUSH", "POP", "ADD", "SUB", "MUL", "DIV", "STORE", "LOAD", "POP_ALL", "CLEANUP","TOP","NEG","NOTEQ","EQ","LT","LTE","GT","GTE","GOTO","BLOCK_END","BLOCK_START","SAFETY_LABEL","SET_AT","LOAD_AT","ADD_NEG","NOT","RET"};
const std::vector<std::string>keywords={"top","goto","do","end","if","else","while","for","func","return"};

std::vector<std::string>existing_function_names(10);

char peek(LEXER& lex,int&index){
    if(index+1 < lex.content.size()){
        return lex.content[index+1];
    }

    return '\0';
}

bool is_function(const std::string& name){
    return std::find(existing_function_names.begin(), existing_function_names.end(), name) != existing_function_names.end();
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

    if(dot_seen==false){ // integer
        int int_value = std::stoi(lex.tokens[token_index].value);
        lex.tokens[token_index].int_val = int_value; 
    }else{ // float
        double float_value = std::stod(lex.tokens[token_index].value);
        lex.tokens[token_index].double_val = float_value;  
    }
}

void dostring(LEXER& lex, int& index) {

    char initial_quote = lex.content[index - 1];

    while (index < lex.content.size()) {
        char current = lex.content[index];
        if (current == initial_quote) {
            index++;
            break;
        } else {
            lex.tokens.back().value += current; // for strings we don't need an actual value or anything like that
            index++;
        }
    }

    lex.tokens.back().type=STRING;
    lex.tokens.back().str_val=lex.tokens.back().value;
    
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

   // static std::vector<std::string>to_pair = {};
   // to_pair.resize(5);

    int index = 0;
    int current_scope_count=0;

    while (index < lex.content.size()) {
        char current = lex.content[index];
        if (current == ' '  || current=='\t' ) {
            index++;
            continue;
        }if(current=='\n'){

            if(lex.in_bytecode==false){
                lex.tokens.push_back({"newline",NEWLINE});
            }

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
                }else{
                    lex.tokens.push_back({"!",OP});
                    
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
            
            case '[':
                lex.tokens.push_back({"[",LPSQ});
                index++;
                continue;

            case ']':
                lex.tokens.push_back({"]",RPSQ});
                index++;
                continue;

            case '"':
                lex.tokens.push_back({"", STRING});
                index++;
                dostring(lex, index);
                continue;

            case ',':
                lex.tokens.push_back({",",COMMA});
                index++;
                continue;

            case ';': // skip semicols
                index++;
                continue;
            
            case '\'':
                lex.tokens.push_back({"", STRING});
                index++;
                dostring(lex, index);
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
                
                if(std::find(bytecode_keywords.begin(), bytecode_keywords.end(), (lex.tokens.back().value)) != bytecode_keywords.end()||(lex.tokens.back().value[0]=='>')||(lex.tokens.back().value.substr(0,4) == "GOTO")){
                 //   std::cout<<"Bytecode token found: "<<lex.tokens.back().value<<"\n";
                     
                    bytecode_tok_cnt++;
                    if(lex.tokens.back().value == "PUSH"){
                        lex.tokens.back().type = BYTECODE_PUSH;
                    }else if(lex.tokens.back().value=="POP"){
                        lex.tokens.back().type = BYTECODE_POP;
                    }else if(lex.tokens.back().value=="ADD"){
                        lex.tokens.back().type = BYTECODE_ADD;
                    }else if(lex.tokens.back().value=="SUB"){
                        lex.tokens.back().type = BYTECODE_SUB;
                    }else if(lex.tokens.back().value == "NEG"){
                        lex.tokens.back().type = BYTECODE_NEG;
                    }else if(lex.tokens.back().value == "ADD_NEG"){
                        lex.tokens.back().type=BYTECODE_ADD_NEG;
                    }else if(lex.tokens.back().value=="MUL"){
                        lex.tokens.back().type = BYTECODE_MUL;
                    }else if(lex.tokens.back().value=="DIV"){
                        lex.tokens.back().type = BYTECODE_DIV;
                    }else if(lex.tokens.back().value=="STORE"){
                        lex.tokens.back().type = BYTECODE_STORE;
                    }else if(lex.tokens.back().value=="LOAD"){
                        lex.tokens.back().type = BYTECODE_LOAD;
                    }else if(lex.tokens.back().value=="SET_AT"){
                        lex.tokens.back().type=BYTECODE_SET_AT;
                    }else if(lex.tokens.back().value=="LOAD_AT"){
                        lex.tokens.back().type=BYTECODE_LOAD_AT;    
                    }else if(lex.tokens.back().value == "NOT"){
                        lex.tokens.back().type = BYTECODE_NOT;
                    }else if(lex.tokens.back().value=="POP_ALL"){
                        lex.tokens.back().type = BYTECODE_POP_ALL;
                    }else if(lex.tokens.back().value == "RET"){
                     //   std::cout<<"found\n";
                        lex.tokens.back().type = BYTECODE_RET;
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

                        lex.goto_positions[lex.tokens.back().value.substr(1)]=bytecode_tok_cnt+1;
                       // lex.goto_scope_count[lex.tokens.back().value.substr(1)]=current_scope_count;
                        
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
                            if(lex.tokens.back().value.find("FUNCTION_START_LABEL")==0){
                                lex.tokens.back().goto_check_union.is_function_start=true;
                            }                     
                            
                            /*if(lex.goto_positions.find(label) == lex.goto_positions.end()){
                                display_err("Label not found: " + label);
                                return;
                            } -- > we take care of this in the compiler */
                            
                           
                        }
                        
                    }else if(lex.tokens.back().value=="BLOCK_START"){
                      //  current_scope_count++;
                    //    std::cout<<"add: "<<current_scope_count<<"\n";
                        lex.tokens.back().type = BYTECODE_MAKE_BLOCK;
                    }else if(lex.tokens.back().value=="BLOCK_END"){
                    //    current_scope_count--;
                    //    std::cout<<"del "<<current_scope_count<<"\n";
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

    int current_start_block = 0;

    if (lex.in_bytecode) {
        int current_scope_count = 0;
        int current_start_block = 0;

        for (size_t i = 0; i < lex.tokens.size(); ++i) {
            TOKEN& tok = lex.tokens[i];
            if (tok.type == BYTECODE_MAKE_BLOCK) {
                current_start_block = i;
                current_scope_count++;
                std::cout<<"block start: "<<current_scope_count<<"\n";
                lex.pre_calc_stack.push_back({0,{}});
            } else if (tok.type == BYTECODE_DEL_BLOCK) {
                
                current_scope_count--;
                std::cout<<"block end: "<<current_scope_count<<"\n";

            } else if (tok.type == BYTECODE_GOTO_LABEL) {
              //  std::cout<<"found label of value: "<<tok.value<<" ; "<<tok.value.substr(21)<<"\n";
                std::string label = tok.value.substr(1);
                lex.goto_scope_count[label].scope_level = current_scope_count;
                if(tok.value.size()>=21){
                    if(tok.value.find(">FUNCTION_DECL_LABEL_") == 0){
                        
                        existing_function_names.push_back(tok.value.substr(21));
                        
                        bool found_pair=false;
                        const std::string end_label = ">FUNCTION_DECL_END_" + tok.value.substr(21); 
                    
                        for(int j = 0;j<lex.tokens.size();j++){
                            if(lex.tokens[j].type!=BYTECODE_GOTO_LABEL){
                                continue;
                            }

                            if(lex.tokens[j].value==end_label){
                                found_pair=true;
                                tok.goto_check_union.pair_token_jump_pos = j;
                                break;
                            }
                        }

                        if(found_pair==false){
                            display_err("Invalid function goto label of name: " + tok.value +"; Expected an end_label allocated to it within the compiler bytecode");
                            return;
                        }
                    }
                        
                }
            }
        }

        for (size_t i = 0; i < lex.tokens.size(); ++i) {
            TOKEN& tok = lex.tokens[i];

            if (tok.type == BYTECODE_GOTO || tok.type == BYTECODE_GOTO_IF_ZERO) {
                std::string label = tok.value;

                auto it = lex.goto_positions.find(label);
                if (it == lex.goto_positions.end()) {
                    display_err("Label not found in goto map: " + label);
                    return;
                }

                int label_index = it->second;
                tok.jump_pos = label_index; // GOTO instruction points to label token
                tok.scope_level = lex.goto_scope_count[label].scope_level;

                std::cout << "Linked GOTO " << label
                        << " (at token " << i << ") → label token " << label_index
                        << " (scope " << tok.scope_level.value() << ")\n";
            }
        }
        
        // simulate scopes and variable declarations
        current_scope_count = 0;

        for (size_t i = 0; i < lex.tokens.size(); ++i) {
            TOKEN& tok = lex.tokens[i];

            if (tok.type == BYTECODE_MAKE_BLOCK) {
                current_scope_count++;
                std::cout<<"added: "<<current_scope_count<<"\n";
                lex.pre_calc_stack.push_back({0, {}}); 
            } 
            else if( tok.type == BYTECODE_DEL_BLOCK) {
                
                if (!lex.pre_calc_stack.empty()) {
                    for (auto& var_name : lex.pre_calc_stack.back().var_names) {
                        if(lex.declared_variables[var_name].scope_level > 0) {
                            lex.declared_variables.erase(var_name);
                            lex.declared_pre_calcs--;
                        }
                    }
                    lex.pre_calc_stack.pop_back();
                }
                current_scope_count--;
                if(current_scope_count < 0 ){current_scope_count=0;}
                // std::cout<<"substracted: "<<current_scope_count<<"\n";
            }

            else if (tok.type == BYTECODE_STORE ) {
                if (i + 1 < lex.tokens.size()) {
                    std::string var_name = lex.tokens[i + 1].value;

                    if (lex.declared_variables.find(var_name) == lex.declared_variables.end()) {
                        std::cout<<"storing: "<<var_name<<" now at indentation level: "<<current_scope_count<<"\n";;
                        int var_id = lex.declared_pre_calcs + 1;
                        lex.declared_variables[var_name] = {var_id, current_scope_count};
                        lex.pre_calc_stack.back().var_names.push_back(var_name);
                        tok.is_new_val=true;
                        lex.declared_pre_calcs++; 
                    }
                    
                    tok.var_id = lex.declared_variables[var_name].id;
                    tok.variable_scope_level = lex.declared_variables[var_name].scope_level;
                }
            }else if(tok.type == BYTECODE_LOAD) {
                if (i + 1 < lex.tokens.size()) {
                    std::string var_name = lex.tokens[i + 1].value;

                    if (lex.declared_variables.find(var_name) == lex.declared_variables.end()) {
                        tok.var_id = -1; // default error value
                        std::cout<<"loading at with error: "<<current_scope_count<<" variable of name: "<<var_name<<"\n";
                        tok.variable_scope_level = 0;

                    } else {
                        std::cout<<"loading variable of name: "<<var_name<<" at indentation scope: "<<current_scope_count<<"\n";
                        tok.var_id = lex.declared_variables[var_name].id;
                        tok.variable_scope_level = lex.declared_variables[var_name].scope_level;
                    }
                }
            }else if(tok.type==BYTECODE_SET_AT){
                if(i+1<lex.tokens.size()){
                    std::string var_name = lex.tokens[i+1].value;
                    if (lex.declared_variables.find(var_name) == lex.declared_variables.end()) {
                        tok.var_id = -1; // default error value
                        tok.variable_scope_level = lex.declared_variables[var_name].scope_level;
                    } else {
                        tok.var_id = lex.declared_variables[var_name].id;
                        tok.variable_scope_level = lex.declared_variables[var_name].scope_level;
                    }
                }
            }else if(tok.type==BYTECODE_LOAD_AT){
                if(i+1<lex.tokens.size()){
                    std::string var_name = lex.tokens[i+1].value;
                    if (lex.declared_variables.find(var_name) == lex.declared_variables.end()) {
                        tok.var_id = -1; // default error value
                        tok.variable_scope_level = lex.declared_variables[var_name].scope_level;
                    } else {
                        tok.var_id = lex.declared_variables[var_name].id;
                        tok.variable_scope_level = lex.declared_variables[var_name].scope_level;
                    }
                }
            }
            else if (tok.type == BYTECODE_GOTO || tok.type == BYTECODE_GOTO_IF_ZERO) {
                if (tok.scope_level.has_value()) {
                    int target_scope = tok.scope_level.value();
                    
                    if(tok.goto_check_union.is_function_start==false){
                        std::cout<<"going to target scope: "<<target_scope<<"\n";
                        while (current_scope_count > target_scope) {
                            if (!lex.pre_calc_stack.empty()) {
                                for (auto& var_name : lex.pre_calc_stack.back().var_names) {
                                    lex.declared_variables.erase(var_name);
                                    lex.declared_pre_calcs--;
                                }
                                lex.pre_calc_stack.pop_back();
                            }
                            current_scope_count--;
                        }
                    }/*else{
                        while(current_scope_count > 1) {
                            if(!lex.pre_calc_stack.empty()){
                                for(auto& var_name : lex.pre_calc_stack.back().var_names){
                                    lex.declared_variables.erase(var_name);
                                    lex.declared_pre_calcs--;
                                }
                                lex.pre_calc_stack.pop_back();
                            }
                            current_scope_count--;
                        }
                    }*/
                }
            }
        }

        int function_info_usecase_check_index = 0;

        for(size_t i = 0; i < lex.tokens.size();i++){
            TOKEN& tok = lex.tokens[i];
            if(tok.type==BYTECODE_LOAD){
                if(i+1<lex.tokens.size() &&  is_function(lex.tokens[i+1].value) && lex.tokens[i+1].used_function == true){
                    std::cout<<"already set for: "<<lex.tokens[i+1].value<<"\n";
                }
                if(i+1<lex.tokens.size() && is_function(lex.tokens[i+1].value) && lex.tokens[i+1].used_function == false){ // the value could've already been setted from the ast.
                    i++;
                    if(i+1<lex.tokens.size()){
                        i++;
                        bool used=false;
                        TOKEN_T next_type = lex.tokens[i].type;
                        if (next_type == BYTECODE_STORE ||
                            next_type == BYTECODE_TOP ||
                            next_type == BYTECODE_ADD ||
                            next_type == BYTECODE_SUB ||
                            next_type == BYTECODE_MUL ||
                            next_type == BYTECODE_DIV ||
                            next_type == BYTECODE_EQ ||
                            next_type == BYTECODE_NOTEQ || 
                            next_type == BYTECODE_NOT ||
                            next_type == BYTECODE_ADD_NEG || 
                            next_type == BYTECODE_NEG
                            ) {
                            used = true;
                        }

                        if(used==false && lex.unused_load_function_indicies_info[function_info_usecase_check_index]==true){
                            used=true;
                        }

                        if(used==false){
                            std::cout<<"unsued function: "<<lex.tokens[i-1].value<<"\n";
                        }else{
                            std::cout<<"used function: "<<lex.tokens[i-1].value<<"\n";
                        }

                        lex.tokens[i-2].used_function = used; // set it to the actual keyword
                        lex.tokens[i-2].is_function = true;

                        function_info_usecase_check_index++;
                    }else{
                        // realistically impossible case
                        return;
                    }
                }
            }
        }
    }

}

void display_lex(LEXER& lex) {
    std::cout << "[";
    for (size_t i = 0; i < lex.tokens.size(); i++) {
        
        std::cout << lex.tokens[i].value;
        
        if(lex.tokens[i].double_val.has_value()){
            std::cout<<"(float: "<<lex.tokens[i].double_val.value()<<")";
        }else if(lex.tokens[i].int_val.has_value()){
            std::cout<<"(int: "<<lex.tokens[i].int_val.value()<<")";
        }else if(lex.tokens[i].jump_pos.has_value()&&lex.tokens[i].scope_level.has_value()){
            std::cout<<"(jump to: "<<lex.tokens[i].jump_pos.value()<<", at scope:"<<lex.tokens[i].scope_level.value()<<")";
        }else if(lex.tokens[i].var_id.has_value()){
            std::cout<<"(var id: "<<lex.tokens[i].var_id.value()<<", at scope:"<<lex.tokens[i].variable_scope_level.value()<<")";
        }

        if (i + 1 != lex.tokens.size()){std::cout << ", ";} 
    }
    std::cout << "]\n";

    if(lex.in_bytecode==true){
        std::cout << "\nGoto positions:\n";
        for (const auto& pos : lex.goto_positions) {
            std::cout << "  Label: " << pos.first << " -> Token Index: " << pos.second << "\n";
        }
    }
}

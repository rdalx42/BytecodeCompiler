

#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib> // malloc/free
#include "../runtime/err.h"

#pragma GCC optimize("O3","unroll-loops")

#define MAX_STACK_VALS 1024
#define MAX_VALS 1024
#define MAX_OP_STACK 101


enum VAL_TYPE {
    INT_VAL,
    FLOAT_VAL,
    STR_VAL,
    VEC_VAL
};

struct VALUE {
    
    // store all by value //
    
    VAL_TYPE type = INT_VAL;
    union{
        int int_val;
        double float_val;
    };
    std::string* str_val=nullptr;
    std::vector<VALUE>* vec_val=nullptr;

    VALUE()=default;
};

struct vexa_stack {
    VALUE* data;
    size_t capacity;
    size_t top;

    void init(size_t cap) {
        data = (VALUE*)malloc(sizeof(VALUE) * cap);
        capacity = cap;
        top = 0;
    }

    void destroy() {
        free(data);
        data = nullptr;
        top = 0;
        capacity = 0;
    }

    bool is_empty() const { return top == 0; }
    size_t sz() const { return top; }

    VALUE& get_back() { return data[top - 1]; }

    void push(const VALUE& val) { data[top++] = val; }

    VALUE& pop() { return data[--top]; }

    VALUE& peek() { return get_back(); }
};

struct STACK_VALUE {
    union {
        int id;
        int var_id;
    };
};


struct MEMORY {
    int length_of_values = 0; // number of values currently used
    int current_stack_amount = 0;

    STACK_VALUE stack_vals[MAX_STACK_VALS];
    size_t stack_vals_size = 0;

    vexa_stack operation_stack;
    VALUE values[MAX_VALS];                             

    void set(const TOKEN& tok,const VALUE& val) {
        
        
        if(tok.var_id.has_value()==false||tok.var_id.value()==-1){
            display_err("Invalid variable ID for setting value");
            return;
        }

        // put value on stack too
        if(tok.is_new_val.has_value()==true && tok.is_new_val.value()==true){
            stack_vals[stack_vals_size].id = current_stack_amount;
            stack_vals[stack_vals_size].var_id = tok.var_id.value();
            stack_vals_size++;
            length_of_values++;
        }
        values[tok.var_id.value() - 1] = val;
    }

    void delete_val(const int& id) {
        
        values[id] = VALUE{}; // reset value
       
    }

    void del_stack() {
        while (stack_vals_size > 0 && stack_vals[stack_vals_size - 1].id == current_stack_amount) {
            
            delete_val(stack_vals[stack_vals_size - 1].var_id);
            length_of_values--;
            --stack_vals_size;
        }
    }

    VALUE get(const TOKEN& tok) {

        if(tok.var_id.has_value()==false||tok.var_id.value()==-1){
           
            display_err("Invalid variable for getting value of: " + tok.value);
            return VALUE{};
        }

        int found_id = tok.var_id.value();
        return values[found_id - 1];
    }
};
#endif

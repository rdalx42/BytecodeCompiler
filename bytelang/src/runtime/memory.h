#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <array>
#include "../runtime/err.h"

#pragma GCC optimize("Ofast","inline-functions")

#define MAX_OP_STACK 20
#define MAX_STACK_VALS 1024
#define MAX_VALS 1024
#define MAX_STRING_LEN 256      // max string length
#define MAX_VECTOR_SIZE 128     // max elements in a vector

enum VAL_TYPE {
    INT_VAL,
    FLOAT_VAL,
    STR_VAL,
    VEC_VAL
};

struct STRING_COMPONENT{
    std::string str_val="";
};

struct VALUE; 


struct ARR_COMPONENT {
    VALUE* arr[MAX_VECTOR_SIZE];
    size_t size = 0;

    void add_el(VALUE* el){
        if(size >= MAX_VECTOR_SIZE){
            display_err("Array full");
            return;
        }
        arr[size++] = el;
    }

    VALUE* get_el_at(unsigned short index) const {
        if(index >= size) return nullptr;
        return arr[index];
    }

    void clear_arr() {
        
        size = 0;
    }
};


struct VALUE {
    VAL_TYPE type = INT_VAL;
    union {
        int int_val;
        float float_val;
        ARR_COMPONENT* arr_val;
    };
    std::optional<STRING_COMPONENT> str_component;

    void clear_val(){
        type=INT_VAL;
        str_component.reset();
        int_val=0;
        arr_val->clear_arr();
        float_val=0.0;
    }

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

    inline void clear_stack(){
        free(data);
        data={};
        top=0;
    }

    inline bool is_empty() const { return top == 0; }
    inline size_t sz() const { return top; }

    inline VALUE& get_back() { return data[top - 1]; }

    inline void push(const VALUE& val) {if(top>capacity){display_err("Stack overflow on operation stack, too much data given to vm");throw std::runtime_error("Vexa Error");} data[top++] = val; }

    inline VALUE& pop() { return data[--top]; }

    inline VALUE& peek() { return get_back(); }
};

struct STACK_VALUE {
    int id;
    int var_id;
};

struct MEMORY {
    int length_of_values = 0;
    int current_stack_amount = 0;

    STACK_VALUE stack_vals[MAX_STACK_VALS];
    size_t stack_vals_size = 0;

    vexa_stack operation_stack;
    VALUE values[MAX_VALS];

    inline void set(const TOKEN& tok, VALUE& val) {
        if (tok.var_id.value() == -1) {
            display_err("Invalid variable ID for setting value");
            return;
        }

        if (tok.is_new_val.has_value() && tok.is_new_val.value()) {
           
            stack_vals[stack_vals_size].id = current_stack_amount;
            stack_vals[stack_vals_size].var_id = tok.var_id.value();
            stack_vals_size++;
            length_of_values++;
        }

        values[tok.var_id.value() - 1] = val;
    }

    inline void delete_val(const int& id) {
        values[id].clear_val();
        length_of_values--;
    }


    inline void del_stack() {
        while (stack_vals_size > 0 && stack_vals[stack_vals_size - 1].id == current_stack_amount) {
            int var_id = stack_vals[stack_vals_size - 1].var_id;
            values[var_id].clear_val();
            --stack_vals_size;
            --length_of_values;
        }
    }

    inline VALUE get(const TOKEN& tok) {
        if (tok.var_id.value() == -1) {
            display_err("Invalid variable for getting value of: " + tok.value);
            return VALUE{};
        }
        return values[tok.var_id.value() - 1];
    }
};

#endif

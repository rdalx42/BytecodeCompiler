#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <array>
#include <cstring>
#include "../runtime/err.h"

#pragma GCC optimize("Ofast","inline-functions")

#define MAX_OP_STACK 20
#define MAX_STACK_VALS 124
#define MAX_VALS 1024
#define DEFAULT_STRING_LEN 256      // default string length
#define MAX_VECTOR_SIZE 128     // max elements in a vector
#define MAX_CALL_STACK 128


enum VAL_TYPE {
    INT_VAL,
    FLOAT_VAL,
    STR_VAL,
    VEC_VAL
};

struct CALL_STACK{

   // V <-- represents the return position of function
    int frames[MAX_CALL_STACK]; // we store them in frames, FIFO -> the first-out value will always be the right value, hopefully this won't cause too many issues with the recursion logic
    unsigned int top=0;

    void push(const unsigned int& return_pos) {
        std::cout<<"pushed: "<<return_pos<<"\n";
        if (top >= MAX_CALL_STACK) {
            display_err("Function call stack overflow");
        }
        frames[top++] = return_pos;
    }

    void pop() {
        if (top == 0) {
            display_err("Function call stack underflow");
        }
        --top;
    }

    int& top_frame() {
        return frames[top - 1];
    }

    bool empty() const { return top == 0; }
};

struct FAST_STRING_COMPONENT{
    
    char* value;
    unsigned int val_lenght=DEFAULT_STRING_LEN;

    void clear(){
        memset(value,0,strlen(value));
        value=nullptr;
    }
    void output(){
        std::cout<<*value;
    }

    void set_at(const int& index, const char& char_val){
        
        if(index<0||index>val_lenght){
            display_err("Can't modify string value at position greated than size!");
            return;
        }

        value[index]=char_val;
    }

        
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

    void print_arr() const {
        
        std::cout<<"[";
        
        for(int i = 0 ; i < size; i++){
            std::cout<<arr[i];
            if(i+1<size){std::cout<<",";}
        }std::cout<<"]";
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
        FAST_STRING_COMPONENT* str_val;
    };
 
    void clear_val() {
        switch(type) {
            case INT_VAL: int_val = 0; break;
            case FLOAT_VAL: float_val = 0.0; break;
            case VEC_VAL:
                if (arr_val) {
                    arr_val->clear_arr();
                    delete arr_val;
                    arr_val = nullptr;
                }
                break;
            case STR_VAL:
                if (str_val) {
                    if (str_val->value) free(str_val->value); // or delete[]
                    delete str_val;
                    str_val = nullptr;
                }
                break;
        }
        type = INT_VAL;
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

    inline void pop_no_return(){--top;}
    inline VALUE& pop() { return data[--top]; }

    inline VALUE& peek() { return get_back(); }
};

struct STACK_VALUE {
    int id;
    int var_id;
};

struct FAST_OPERATION_TEMPORARY_SLOTS{
    FAST_STRING_COMPONENT fots_str;
};

struct MEMORY {

    FAST_OPERATION_TEMPORARY_SLOTS fast_operation_temporary_slots;
    
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

        // free old string
       /// std::cout<<"stored: "<<tok.var_id.value()<<'\n';
        VALUE& old_val = values[tok.var_id.value() - 1]; 
        if (old_val.type == STR_VAL && old_val.str_val) {
            if (old_val.str_val->value) free(old_val.str_val->value);
            delete old_val.str_val;
            old_val.str_val = nullptr;
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
      //  std::cout<<"erased: "<<id<<"\n";
        values[id].clear_val();
        length_of_values--;
    }

    // UTILS

    void concat_fast_string_safe(FAST_STRING_COMPONENT& dest, const FAST_STRING_COMPONENT& a, const FAST_STRING_COMPONENT& b){
        if(!a.value||!b.value){
            display_err("Attempted to concate 2 invalid string elements");
            return;
        }

        size_t len_a=strlen(a.value);
        size_t len_b=strlen(b.value);

        if(len_a+len_b>dest.val_lenght){
            display_err("String too long in concatenation operation, storage capacity string is of: "+std::to_string(dest.val_lenght));
            return;
        }

        memcpy(dest.value,a.value,len_a); // copy a
        memcpy(dest.value+len_a,b.value,len_b); // append b
        dest.value[len_a+len_b]='\0'; // null terminate
    }


    // UTILS


    inline void del_stack() {
        while (stack_vals_size > 0 && stack_vals[stack_vals_size - 1].id == current_stack_amount) {
            int var_id = stack_vals[stack_vals_size - 1].var_id;
            values[var_id].clear_val();
            --stack_vals_size;
            --length_of_values;
        }
    }

    inline const VALUE& get(const TOKEN& tok) const { // pass by reference
    //    std::cout<<">"<<tok.value<<"\n";
        if (tok.var_id.value() == -1) {
            display_err("Invalid variable");
            static VALUE dummy{};
            return dummy; // works
        }
        return values[tok.var_id.value() - 1];
    }
};

#endif

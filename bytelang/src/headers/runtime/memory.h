#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include "../runtime/err.h"

struct VALUE {
    using value_t = std::variant<int, double, std::string, std::vector<VALUE>>;

    value_t data;

    VALUE() = default;
    VALUE(int i) : data(i) {}
    VALUE(double d) : data(d) {}
    VALUE(const std::string& s) : data(s) {}
    VALUE(const std::vector<VALUE>& v) : data(v) {}
};

struct STACK_VALUE {
    int id = 0;
    std::string name = "";
};

struct MEMORY {
    int current_stack_amount = 0;
    std::vector<STACK_VALUE> stack_vals;
    std::vector<VALUE> operation_stack;

    std::unordered_map<std::string, int> id_for_value; 
    std::vector<VALUE> values;                         

    int get_or_create_id(const std::string& name) {
        auto it = id_for_value.find(name);
        if (it != id_for_value.end()) return it->second;

        int id = (int)values.size();
        id_for_value[name] = id;
        values.emplace_back();

        STACK_VALUE sv { current_stack_amount, name };
        stack_vals.push_back(sv);
        return id;
    }

    void set(const std::string& name, const VALUE& val) {
        int id = get_or_create_id(name);
        values[id] = val;
    }

    void delete_val(const std::string& name) {
        auto it = id_for_value.find(name);
        if (it != id_for_value.end()) {
            int id = it->second;
            values[id] = VALUE(); // clear value
            id_for_value.erase(it);
        } else {
            display_err("Variable of name '" + name + "' not defined.");
        }
    }

    void del_stack() {
        while (!stack_vals.empty() && stack_vals.back().id == current_stack_amount) {
            delete_val(stack_vals.back().name);
            stack_vals.pop_back();
        }
    }

    VALUE get(const std::string& name) const {
        auto it = id_for_value.find(name);
        if (it != id_for_value.end()) {
            return values[it->second];
        }
        display_err("Variable of name '" + name + "' not defined.");
        return VALUE();
    }

    bool exists(const std::string& name) const {
        return id_for_value.find(name) != id_for_value.end();
    }
};

#endif

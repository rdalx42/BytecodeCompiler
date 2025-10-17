#ifndef MEMORY_H
#define MEMORY_H

#pragma GCC optimize("O3","unroll-loops")

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
    int var_id = -1;
};

struct MEMORY {
    int current_stack_amount = 0;
    std::vector<STACK_VALUE> stack_vals;
    std::vector<VALUE> operation_stack;

    std::unordered_map<std::string, int> id_for_value;
    std::vector<VALUE> values;

    MEMORY() {
        values.reserve(256);
        operation_stack.reserve(128);
        stack_vals.reserve(256);
        id_for_value.reserve(256);
    }

    int get_or_create_id(const std::string& name) {
        auto it = id_for_value.find(name);
        if (it != id_for_value.end()) return it->second;

        int id = (int)values.size();
        id_for_value.emplace(name, id);
        values.emplace_back();
        stack_vals.push_back({ current_stack_amount, id });
        return id;
    }

    void set(const std::string& name, const VALUE& val) {
        int id = get_or_create_id(name);
        if (id >= (int)values.size()) values.resize(id + 1);
        values[id] = val;
    }

    void delete_val(const std::string& name) {
        auto it = id_for_value.find(name);
        if (it == id_for_value.end()) {
            display_err("Variable of name '" + name + "' not defined.");
            return;
        }
        int id = it->second;
        if (id < (int)values.size()) values[id] = VALUE();
        id_for_value.erase(it);
    }

    void del_stack() {
        while (!stack_vals.empty() && stack_vals.back().id == current_stack_amount) {
            auto& sv = stack_vals.back();
            if (sv.var_id >= 0 && sv.var_id < (int)values.size()) values[sv.var_id] = VALUE();
            stack_vals.pop_back();
        }
    }

    VALUE get(const std::string& name) const {
        auto it = id_for_value.find(name);
        if (it == id_for_value.end()) display_err("Variable of name '" + name + "' not defined.");
        return values[it->second];
    }

    bool exists(const std::string& name) const {
        return id_for_value.find(name) != id_for_value.end();
    }
};

#endif

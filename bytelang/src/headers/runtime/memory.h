#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <vector>
#include <variant>
#include <memory>
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

struct MEMORY {

    std::vector<VALUE>operation_stack; // for calculations, wont use std::stack since std::vector is faster 
    std::unordered_map<std::string, VALUE> variables;

    void set(const std::string& name, const VALUE& val) {
        variables[name] = val;
    }

    VALUE get(const std::string& name) {
        if (variables.find(name) != variables.end()) {
            return variables[name];
        }
        std::string err = "Variable of name '" + name + "' not defined.";
        display_err(err);
        return VALUE();
    }

    bool exists(const std::string& name) const {
        return variables.find(name) != variables.end();
    }
};

#endif

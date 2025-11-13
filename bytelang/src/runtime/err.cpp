#include "../headers/runtime/err.h"
#include <iostream>
#include <cstdlib> 

void display_err(const std::string& message) {
    const std::string RED   = "\033[31m";  
    const std::string RESET = "\033[0m";   

    std::cerr << RED << "[Error]: " << message << RESET << std::endl;

    std::exit(EXIT_FAILURE);
}

void display_non_fatal_err(const std::string& message){
    const std::string RED   = "\033[31m";  
    const std::string RESET = "\033[0m";   

    std::cerr << RED << "[Error]: " << message << RESET << std::endl;
}

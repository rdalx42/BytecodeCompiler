
#include "../headers/runtime/err.h"
#include <iostream>

void display_err(std::string message){
    ERROR E;
    E.message = message;
    std::cout<<"[Error]: "<<E.message<<std::endl;
}

#ifndef SHELL_H
#define SHELL_H

#include <string>

struct SHELL{
    std::string filename;
};

void doshell(SHELL& sh);
void prompt(SHELL& sh);

#endif 

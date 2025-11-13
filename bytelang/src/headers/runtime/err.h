
#ifndef ERR_H
#define ERR_H

#include <algorithm>
#include <string>

struct ERROR{

    std::string message = "";
};

void display_err(const std::string& message);
void display_non_fatal_err(const std::string& message);

#endif 


#ifndef ERR_H
#define ERR_H

#include <algorithm>
#include <string>

struct ERROR{

    std::string message = "";
};

void display_err(std::string message);

#endif 
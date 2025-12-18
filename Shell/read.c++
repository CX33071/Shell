#include <string>
#include <iostream>
#include <algorithm>
std::string read(){
    std::string input;
    std::getline(std::cin, input);
    input.erase(0, input.find_first_not_of(" "));
    input.erase(input.find_last_not_of(" ") + 1);
    return input;
}
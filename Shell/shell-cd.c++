#include <filesystem>
#include <iostream>
#include <locale>
#include <string>
constexpr const char* RED = "\033[1;31m";
constexpr const char* GREEN = "\033[1;32m";
constexpr const char* BLUE = "\033[1;34m";
constexpr const char* RESET = "\033[0m";
class CD {
   private:
    std::string formal_dir;
    std::string current_dir;
    std::string get_dir() const {
        try {
            return std::filesystem::current_path().string();
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << BLUE << e.what()
                      << RESET << std::endl;
            return ".";
        }
    }
   public:
    CD() {
        std::locale::global(std::locale(""));
        std::cout.imbue(std::locale());
        current_dir = get_dir();
        formal_dir = current_dir;
    }
    bool change_dir(const std::string& target) {
        std::string next = target;
        std::string old_formal = formal_dir; 
        if (target == "-") {
            next = formal_dir;
        }
        formal_dir = current_dir;
        try {
            std::filesystem::current_path(next);
            current_dir = get_dir();
            if (target == "-") {
                std::cout << GREEN << current_dir << RESET << std::endl;
            }
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << RED << e.what() << RESET
                      << std::endl;
            formal_dir = old_formal;  
            return false;
        }
    }
    const std::string& get_current_dir() const { return current_dir; }
    const std::string& get_prev_dir() const { return formal_dir; }
};
int main(int argc, char* argv[]) {
    CD cd;
    std::string target = ".";
    if (argc > 1) {
        target = argv[1];
    }
    bool success = cd.change_dir(target);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

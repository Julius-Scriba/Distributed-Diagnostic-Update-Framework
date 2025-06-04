#include "Loader.h"
#include <iostream>

int main() {
    Loader loader("./plugins");
    loader.load();
    for (const auto& mod : loader.modules()) {
        std::cout << "Loaded module: " << mod->name() << std::endl;
        mod->init();
    }
    return 0;
}

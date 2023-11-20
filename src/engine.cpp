#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "../include/client.hpp"

int main() {
    auto engineData = initEngine();
    runEngine(engineData);
    return 0;
}

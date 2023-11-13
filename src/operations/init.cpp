#include <string>
#include <fstream>

void initDiskStorage(std::string& directory) {
    if (!std::filesystem::exists(directory)) std::filesystem::create_directory(directory);
}




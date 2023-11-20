#include "../../include/commands.hpp"

void NukeDataCommand::execute() {
    for (const auto& entry : std::filesystem::directory_iterator(rootDir)) 
        std::filesystem::remove_all(entry.path());
}

#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstdlib>

const int MAX_GENERATION = 3;

std::string getExecutableName() {
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    path[len] = '\0';
    std::string fullPath(path);
    return fullPath.substr(fullPath.find_last_of('/') + 1);
}

// Determines if the binary is "selfrep" (the root binary)
bool isRootBinary(const std::string& name) {
    return name == "selfrep";
}

// Generates names like: copy1, copy2, or copy11, copy12, ...
std::string generateChildName(const std::string& parent, int i) {
    std::ostringstream name;
    if (parent == "selfrep") {
        name << "copy" << i;
    } else {
        name << parent << i;
    }
    return name.str();
}

void replicateThreeChildren(const std::string& parentName) {
    for (int i = 1; i <= MAX_GENERATION; ++i) {
        std::string childName = generateChildName(parentName, i);

        // Copy self binary
        std::ifstream src("/proc/self/exe", std::ios::binary);
        std::ofstream dst(childName, std::ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();

        // Make executable
        std::string chmodCmd = "chmod +x " + childName;
        system(chmodCmd.c_str());
    }
}

int main() {
    std::string currentBinary = getExecutableName();

    // On execution, replicate only if children are not already created
    // (light protection: skip if child already exists)
    bool shouldReplicate = false;

    // Check if all 3 children exist
    for (int i = 1; i <= MAX_GENERATION; ++i) {
        std::string child = generateChildName(currentBinary, i);
        std::ifstream test(child);
        if (!test.good()) {
            shouldReplicate = true;
            break;
        }
    }

    // If they don’t exist yet, create them
    if (shouldReplicate) {
        replicateThreeChildren(currentBinary);
    }

    return 0;
}

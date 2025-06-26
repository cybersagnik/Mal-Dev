#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unistd.h>
#include <cstdio>
#include <limits.h>

void secretPayload() {
    std::cout << "Payload executing..." << std::endl;
    std::cout << "Important logic here." << std::endl;
}

void selfDelete() {
    char path[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        if (access(path, F_OK) == 0) {
            if (unlink(path) == 0) {
                std::cout << "✅ Binary successfully self-deleted.\n";
            } else {
                perror("❌ unlink failed");
            }
        }
    } else {
        perror("❌ readlink failed");
    }
}

std::string extractFunction(const std::string& filename, const std::string& funcName) {
    std::ifstream file(filename);
    std::string line, extracted;
    bool inside = false;
    int braceCount = 0;
    std::regex funcStart("^\\s*void\\s+" + funcName + "\\s*\\(.*\\)\\s*\\{");

    while (std::getline(file, line)) {
        if (!inside && std::regex_search(line, funcStart)) {
            inside = true;
        }

        if (inside) {
            extracted += line + "\n";
            braceCount += std::count(line.begin(), line.end(), '{');
            braceCount -= std::count(line.begin(), line.end(), '}');
            if (braceCount == 0) break;
        }
    }

    return extracted;
}

void generateMinimalCopy(const std::string& outFile, const std::string& functionCode) {
    std::ofstream output(outFile);
    output << "#include <iostream>\n";
    output << functionCode << "\n";
    output << "int main() {\n";
    output << "    secretPayload();\n";
    output << "    return 0;\n";
    output << "}\n";
}

int main() {
    std::string src = "selfrep.cpp";
    std::string out = "copy.cpp";
    std::string func = "secretPayload";

    std::string realFuncCode = extractFunction(src, func);
    generateMinimalCopy(out, realFuncCode);

    system(("g++ " + out + " -o copy").c_str());

    std::remove(src.c_str());
    std::remove(out.c_str());

    selfDelete(); // Deletes *selfrep* binary only

    return 0;
}

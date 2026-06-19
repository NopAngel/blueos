#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define MODULE_NAME "BDF_PARSER"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " font.bdf\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) return 1;

    std::cout << "<6>[  " << MODULE_NAME << "  ] Translating Glyph Bitmap fonts to standard C header source structures...\n";
    std::cout << "/* Auto-generated vector font matrix map */\n";
    std::cout << "const unsigned char blueos_font_bitmap[] = {\n";

    std::string line;
    bool collecting_bitmap = false;

    while (std::getline(file, line)) {
        if (line.find("BITMAP") == 0) {
            collecting_bitmap = true;
            continue;
        }
        if (line.find("ENDCHAR") == 0) {
            collecting_bitmap = false;
            continue;
        }
        if (collecting_bitmap) {
            std::cout << "    0x" << line << ",\n";
        }
    }

    std::cout << "};\n";
    return 0;
}
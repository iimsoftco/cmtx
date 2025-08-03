#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: cmtx input.mtx output_prefix\n";
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_prefix = argv[2];

    std::ifstream infile(input_path, std::ios::binary);
    if (!infile) {
        std::cerr << "Failed to open MTX file\n";
        return 1;
    }

    // Read entire file into memory
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(infile)),
                                     std::istreambuf_iterator<char>());
    size_t pos = 0;
    int jpeg_count = 0;

    while (pos + 1 < data.size()) {
        // Look for JPEG SOI marker 0xFFD8
        if (data[pos] == 0xFF && data[pos + 1] == 0xD8) {
            size_t start = pos;
            pos += 2;

            // Search for JPEG EOI marker 0xFFD9
            while (pos + 1 < data.size()) {
                if (data[pos] == 0xFF && data[pos + 1] == 0xD9) {
                    size_t end = pos + 2; // Include the EOI bytes
                    // Extract this JPEG chunk
                    std::string out_filename = std::string(output_prefix) + std::to_string(jpeg_count) + ".jpg";
                    std::ofstream outfile(out_filename, std::ios::binary);
                    if (!outfile) {
                        std::cerr << "Failed to open output file " << out_filename << "\n";
                        return 1;
                    }
                    outfile.write(reinterpret_cast<char*>(&data[start]), end - start);
                    std::cout << "Extracted JPEG #" << jpeg_count << " to " << out_filename << "\n";
                    ++jpeg_count;
                    pos = end;
                    break;
                }
                ++pos;
            }
        } else {
            ++pos;
        }
    }

    if (jpeg_count == 0) {
        std::cerr << "No JPEG images found in MTX file\n";
        return 1;
    }

    return 0;
}

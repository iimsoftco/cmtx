#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// === Shared ===
bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}

// === JPEG → MTX ===
bool jpeg_to_mtx(const std::string& input_jpeg, const std::string& output_mtx) {
    std::ifstream infile(input_jpeg, std::ios::binary);
    if (!infile) {
        std::cerr << "Failed to open input JPEG: " << input_jpeg << "\n";
        return false;
    }

    std::vector<char> jpeg_data((std::istreambuf_iterator<char>(infile)),
                                 std::istreambuf_iterator<char>());

    if (jpeg_data.size() < 2 || jpeg_data[0] != (char)0xFF || jpeg_data[1] != (char)0xD8) {
        std::cerr << "Error: Input is not a valid JPEG (missing SOI)\n";
        return false;
    }

    std::ofstream outfile(output_mtx, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to open output MTX file: " << output_mtx << "\n";
        return false;
    }

    outfile.write(jpeg_data.data(), jpeg_data.size());
    std::cout << "Packed JPEG into MTX: " << output_mtx << "\n";
    return true;
}

// === MTX → JPEG ===
bool mtx_to_jpeg(const std::string& input_mtx, const std::string& output_jpeg) {
    std::ifstream infile(input_mtx, std::ios::binary);
    if (!infile) {
        std::cerr << "Failed to open MTX file: " << input_mtx << "\n";
        return false;
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(infile)),
                                     std::istreambuf_iterator<char>());

    // Search for SOI and EOI markers
    size_t start = std::string::npos;
    size_t end = std::string::npos;

    for (size_t i = 0; i + 1 < data.size(); ++i) {
        if (data[i] == 0xFF && data[i + 1] == 0xD8 && start == std::string::npos) {
            start = i;
        }
        if (data[i] == 0xFF && data[i + 1] == 0xD9 && start != std::string::npos) {
            end = i + 2;
            break;
        }
    }

    if (start == std::string::npos || end == std::string::npos || end <= start) {
        std::cerr << "Could not find a valid JPEG in MTX file\n";
        return false;
    }

    std::ofstream outfile(output_jpeg, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to open output JPEG file: " << output_jpeg << "\n";
        return false;
    }

    outfile.write(reinterpret_cast<char*>(&data[start]), end - start);
    std::cout << "Extracted JPEG to: " << output_jpeg << "\n";
    return true;
}

// === Main ===
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "[cmtx canary] Usage:\n";
        std::cerr << "  " << argv[0] << " extract input.mtx output.jpg\n";
        std::cerr << "  " << argv[0] << " pack input.jpg output.mtx\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    if (mode == "extract") {
        return mtx_to_jpeg(input, output) ? 0 : 1;
    } else if (mode == "pack") {
        return jpeg_to_mtx(input, output) ? 0 : 1;
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }
}

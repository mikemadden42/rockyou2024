#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

constexpr std::size_t CHUNK_SIZE = 1024 * 1024;  // 1 MB

int search_in_chunk(const std::string_view buffer, const std::string& keyword) {
    int count = 0;
    std::size_t pos = 0;
    while ((pos = buffer.find(keyword, pos)) != std::string::npos) {
        ++count;
        pos += keyword.length();
    }
    return count;
}

void search_in_file(const std::string& filename, const std::string& keyword) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    // Allocate buffer with extra space for overlapping
    std::vector<char> buffer(CHUNK_SIZE);

    auto start_time = std::chrono::high_resolution_clock::now();

    int count = 0;
    while (file.read(buffer.data(), CHUNK_SIZE)) {
        auto bytes_read = file.gcount();
        count += search_in_chunk(std::string_view(buffer.data(), bytes_read),
                                 keyword);

        // Move the file pointer to adjust for overlapping
        file.seekg(-(std::streamoff)(keyword.size() - 1), std::ios::cur);

        // Print current count on the same line
        std::cout << "\rCurrent occurrences of keyword \"" << keyword
                  << "\": " << count << std::flush;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_time_used = end_time - start_time;

    std::cout << std::endl;  // Move to the next line after completion
    std::cout << "Search complete. Time taken: " << cpu_time_used.count()
              << " seconds" << std::endl;
}

int main() {
    try {
        std::string keyword;
        std::string filename;

        // Get the keyword from the user
        std::cout << "Enter the keyword to search: ";
        std::getline(std::cin, keyword);

        // Get the filename from the user
        std::cout << "Enter the filename to search in: ";
        std::getline(std::cin, filename);

        if (!fs::exists(filename)) {
            throw std::runtime_error("File does not exist: " + filename);
        }

        search_in_file(filename, keyword);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

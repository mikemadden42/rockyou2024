#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

constexpr std::size_t CHUNK_SIZE = 1024 * 1024;  // 1 MB

std::uint64_t search_in_chunk(const std::string_view buffer,
                              const std::string& keyword) {
    std::uint64_t count = 0;
    std::size_t pos = 0;
    while ((pos = buffer.find(keyword, pos)) != std::string::npos) {
        ++count;
        pos += keyword.length();
    }
    return count;
}

void search_in_file(const std::string& filename, const std::string& keyword) {
    std::unique_ptr<std::FILE, int (*)(std::FILE*)> file(
        std::fopen(filename.c_str(), "rb"), &std::fclose);
    if (!file) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    // Read directly into our buffer; stdio buffering only slows down 1 MB reads
    std::setvbuf(file.get(), nullptr, _IONBF, 0);

    // Allocate buffer with extra space for overlapping
    const std::size_t overlap = keyword.size() - 1;
    std::vector<char> buffer(CHUNK_SIZE + overlap);

    auto print_count = [&keyword](std::uint64_t count) {
        std::cout << "\rOccurrences of keyword \"" << keyword
                  << "\": " << count << std::flush;
    };

    auto start_time = std::chrono::high_resolution_clock::now();

    std::uint64_t count = 0;
    std::size_t carried = 0;
    while (std::size_t bytes_read = std::fread(buffer.data() + carried, 1,
                                               CHUNK_SIZE, file.get())) {
        const std::size_t bytes_available = carried + bytes_read;
        count += search_in_chunk(
            std::string_view(buffer.data(), bytes_available), keyword);

        // Carry the tail forward so matches spanning two chunks are found
        carried = std::min(overlap, bytes_available);
        std::memmove(buffer.data(), buffer.data() + bytes_available - carried,
                     carried);

        // Print current count on the same line
        print_count(count);
    }

    if (std::ferror(file.get())) {
        std::cout << std::endl;
        throw std::runtime_error("Error reading file: " + filename);
    }

    print_count(count);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    std::cout << std::endl;  // Move to the next line after completion
    std::cout << "Search complete. Time taken: " << elapsed_time.count()
              << " seconds" << std::endl;
}

int main() {
    try {
        std::string keyword;
        std::string filename;

        // Get the keyword from the user
        std::cout << "Enter the keyword to search: ";
        std::getline(std::cin, keyword);

        if (keyword.empty()) {
            throw std::runtime_error("Keyword must not be empty");
        }

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

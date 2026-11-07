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

// Is this one line, without its newline, equal to the keyword?
bool is_exact_line(const std::string_view line, const std::string& keyword) {
    if (line.size() == keyword.size() + 1 && line.back() == '\r') {
        return line.substr(0, keyword.size()) == keyword;
    }
    return line == keyword;
}

// The buffer must end with a newline, so every hit has a byte after it
std::uint64_t count_exact_lines(const std::string_view buffer,
                                const std::string& keyword,
                                bool at_line_start) {
    std::uint64_t count = 0;
    std::size_t pos = 0;
    while ((pos = buffer.find(keyword, pos)) != std::string::npos) {
        const bool starts_line = pos == 0 ? at_line_start : buffer[pos - 1] == '\n';
        const std::size_t end = pos + keyword.size();
        const bool ends_line =
            buffer[end] == '\n' ||
            (buffer[end] == '\r' && buffer[end + 1] == '\n');
        if (starts_line && ends_line) {
            ++count;
        }
        pos = end;
    }
    return count;
}

void search_in_file(const std::string& filename, const std::string& keyword,
                    bool exact) {
    std::unique_ptr<std::FILE, int (*)(std::FILE*)> file(
        std::fopen(filename.c_str(), "rb"), &std::fclose);
    if (!file) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    // Read directly into our buffer; stdio buffering only slows down 1 MB reads
    std::setvbuf(file.get(), nullptr, _IONBF, 0);

    // Allocate buffer with extra space for overlapping
    // In exact mode this holds an unfinished line: the keyword plus a '\r'
    const std::size_t overlap = exact ? keyword.size() + 1 : keyword.size() - 1;
    std::vector<char> buffer(CHUNK_SIZE + overlap);

    const std::string label = exact ? "Lines matching keyword \""
                                    : "Occurrences of keyword \"";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::uint64_t count = 0;
    std::size_t carried = 0;
    bool at_line_start = true;
    while (std::size_t bytes_read = std::fread(buffer.data() + carried, 1,
                                               CHUNK_SIZE, file.get())) {
        const std::size_t bytes_available = carried + bytes_read;
        const std::string_view view(buffer.data(), bytes_available);

        // Carry the tail forward so matches spanning two chunks are found
        if (exact) {
            const std::size_t last_newline = view.rfind('\n');
            const std::size_t complete =
                last_newline == std::string_view::npos ? 0 : last_newline + 1;

            count += count_exact_lines(view.substr(0, complete), keyword,
                                       at_line_start);
            at_line_start = at_line_start || complete > 0;
            carried = bytes_available - complete;

            // An unfinished line too long to match can't be a whole-line hit
            if (carried > overlap || !at_line_start) {
                carried = 0;
                at_line_start = false;
            }
        } else {
            count += search_in_chunk(view, keyword);
            carried = std::min(overlap, bytes_available);
        }
        std::memmove(buffer.data(), buffer.data() + bytes_available - carried,
                     carried);

        // Print current count on the same line
        std::cerr << '\r' << label << keyword << "\": " << count << std::flush;
    }

    if (std::ferror(file.get())) {
        std::cerr << std::endl;
        throw std::runtime_error("Error reading file: " + filename);
    }

    // The last line may have no newline
    if (exact && at_line_start &&
        is_exact_line(std::string_view(buffer.data(), carried), keyword)) {
        ++count;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    // Return to the start of the line so the result replaces the progress count
    std::cerr << '\r' << std::flush;
    std::cout << label << keyword << "\": " << count << std::endl;
    std::cerr << "Search complete. Time taken: " << elapsed_time.count()
              << " seconds" << std::endl;
}

int main(int argc, char* argv[]) {
    const std::string usage =
        "Usage: " + std::string(argv[0]) + " [--exact] [filename]";

    bool exact = false;
    std::string filename;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--help" || argument == "-h") {
            std::cout << usage << std::endl;
            return 0;
        }
        if (argument == "--exact") {
            exact = true;
        } else if (argument[0] == '-' || !filename.empty()) {
            std::cerr << usage << std::endl;
            return 1;
        } else {
            filename = argument;
        }
    }

    try {
        std::string keyword;

        // Get the keyword from the user
        std::cerr << "Enter the keyword to search: ";
        std::getline(std::cin, keyword);

        if (keyword.empty()) {
            throw std::runtime_error("Keyword must not be empty");
        }

        // Get the filename from the command line or the user
        if (filename.empty()) {
            std::cerr << "Enter the filename to search in: ";
            std::getline(std::cin, filename);
        }

        if (!fs::exists(filename)) {
            throw std::runtime_error("File does not exist: " + filename);
        }

        search_in_file(filename, keyword, exact);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

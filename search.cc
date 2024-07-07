#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#define CHUNK_SIZE (1024 * 1024)  // 1 MB
#define MAX_KEYWORD_LENGTH 256
#define MAX_FILENAME_LENGTH 1024

int search_in_chunk(const char *buffer, const char *keyword,
                    size_t keyword_length) {
    int count = 0;
    const char *pos = buffer;
    while ((pos = strstr(pos, keyword)) != nullptr) {
        count++;
        pos += keyword_length;
    }
    return count;
}

int main() {
    clock_t start_time, end_time;
    double cpu_time_used;

    char keyword[MAX_KEYWORD_LENGTH];
    char filename[MAX_FILENAME_LENGTH];

    // Get the keyword from the user
    std::cout << "Enter the keyword to search: ";
    std::cin.getline(keyword, MAX_KEYWORD_LENGTH);
    size_t keyword_length = strlen(keyword);
    if (keyword[keyword_length - 1] == '\n') {
        keyword[keyword_length - 1] = '\0';
        keyword_length--;
    }

    // Get the filename from the user
    std::cout << "Enter the filename to search in: ";
    std::cin.getline(filename, MAX_FILENAME_LENGTH);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    char *buffer = new char[CHUNK_SIZE + keyword_length - 1];
    if (buffer == nullptr) {
        std::cerr << "Error allocating buffer" << std::endl;
        file.close();
        return EXIT_FAILURE;
    }

    // Start the timer
    start_time = clock();

    size_t chunk_start = 0;
    size_t bytes_read;
    int count = 0;
    while (file.read(buffer, CHUNK_SIZE)) {
        bytes_read = file.gcount();
        buffer[bytes_read] = '\0';  // Null-terminate the buffer
        count += search_in_chunk(buffer, keyword, keyword_length);
        chunk_start += bytes_read;

        // Move the file pointer to adjust for overlapping
        file.seekg(-(std::streamoff)(keyword_length - 1), std::ios::cur);
        chunk_start -= keyword_length - 1;

        // Print current count on the same line
        std::cout << "\rCurrent occurrences of keyword \"" << keyword
                  << "\": " << count << std::flush;
    }

    // Stop the timer
    end_time = clock();

    std::cout << std::endl;  // Move to the next line after completion
    delete[] buffer;
    file.close();

    // Calculate the elapsed time
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    std::cout << "Search complete. Time taken: " << cpu_time_used << " seconds"
              << std::endl;

    return EXIT_SUCCESS;
}

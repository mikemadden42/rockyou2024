#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;

constexpr std::size_t CHUNK_SIZE = 1024 * 1024;  // 1 MB, matches search.cc

struct Result {
    bool succeeded;
    std::string output;
};

// Run the search binary, answering its prompts from a file
class SearchRunner {
   public:
    SearchRunner(fs::path binary, fs::path work_dir)
        : binary_(std::move(binary)), work_dir_(std::move(work_dir)) {}

    fs::path write_file(const std::string& contents) const {
        const fs::path path = work_dir_ / "input.txt";
        std::ofstream file(path, std::ios::binary);
        file << contents;
        return path;
    }

    Result run(const std::string& keyword, const fs::path& filename) const {
        return execute(keyword + '\n' + filename.string() + '\n', "", "2>&1");
    }

    // Pass arguments on the command line and capture stdout on its own
    Result run_with_arguments(const std::string& keyword,
                              const std::string& arguments) const {
        const fs::path errors = work_dir_ / "errors.txt";
        return execute(keyword + '\n', arguments,
                       "2> \"" + errors.string() + '"');
    }

    const fs::path& work_dir() const { return work_dir_; }

   private:
    Result execute(const std::string& answers_text,
                   const std::string& arguments,
                   const std::string& stderr_redirect) const {
        const fs::path answers = work_dir_ / "answers.txt";
        const fs::path output = work_dir_ / "output.txt";

        std::ofstream(answers, std::ios::binary) << answers_text;

        const std::string command = '"' + binary_.string() + "\" " + arguments +
                                    " < \"" + answers.string() + "\" > \"" +
                                    output.string() + "\" " + stderr_redirect;
        const bool succeeded = std::system(command.c_str()) == 0;

        std::ifstream file(output, std::ios::binary);
        return {succeeded, std::string(std::istreambuf_iterator<char>(file),
                                       std::istreambuf_iterator<char>())};
    }

    fs::path binary_;
    fs::path work_dir_;
};

// The count is rewritten in place, so the last one printed is the final total
std::optional<std::uint64_t> final_count(const std::string& output) {
    const auto pos = output.rfind("\": ");
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    return std::stoull(output.substr(pos + 3));
}

void expect_count(const SearchRunner& runner, const std::string& keyword,
                  const std::string& contents, std::uint64_t expected) {
    const Result result = runner.run(keyword, runner.write_file(contents));
    const auto count = final_count(result.output);

    if (!result.succeeded || !count || *count != expected ||
        result.output.find("Search complete") == std::string::npos) {
        std::ostringstream message;
        message << "expected count " << expected << ", got "
                << (count ? std::to_string(*count) : "none")
                << (result.succeeded ? "" : " (search failed)");
        throw std::runtime_error(message.str());
    }
}

void expect_error(const SearchRunner& runner, const std::string& keyword,
                  const fs::path& filename, const std::string& expected) {
    const Result result = runner.run(keyword, filename);

    if (result.succeeded ||
        result.output.find(expected) == std::string::npos) {
        throw std::runtime_error("expected error \"" + expected + "\", got: " +
                                 result.output.substr(0, 200));
    }
}

const std::map<std::string, std::function<void(const SearchRunner&)>> TESTS = {
    {"small_file",
     [](const SearchRunner& runner) {
         expect_count(runner, "needle", "needle\nabc needle\nneedle\n", 3);
     }},
    {"empty_file",
     [](const SearchRunner& runner) {
         expect_count(runner, "needle", "", 0);
     }},
    {"final_partial_chunk",
     [](const SearchRunner& runner) {
         expect_count(runner, "needle",
                      "needle\n" + std::string(CHUNK_SIZE, 'x') + "\nneedle\n",
                      2);
     }},
    {"across_chunks",
     [](const SearchRunner& runner) {
         const std::string padding(CHUNK_SIZE - 3, 'x');
         expect_count(runner, "needle", padding + "needle" + padding, 1);
     }},
    {"end_of_file",
     [](const SearchRunner& runner) {
         expect_count(runner, "needle",
                      std::string(2 * CHUNK_SIZE - 6, 'x') + "needle", 1);
     }},
    {"no_double_count",
     [](const SearchRunner& runner) {
         expect_count(runner, "aa", std::string(3 * CHUNK_SIZE, 'a'),
                      3 * CHUNK_SIZE / 2);
     }},
    {"keyword_longer_than_chunk",
     [](const SearchRunner& runner) {
         const std::string keyword(CHUNK_SIZE + 1, 'k');
         expect_count(runner, keyword,
                      std::string(CHUNK_SIZE - 9, 'x') + keyword +
                          std::string(500, 'x'),
                      1);
     }},
    {"empty_keyword",
     [](const SearchRunner& runner) {
         expect_error(runner, "",
                      runner.write_file(std::string(2 * CHUNK_SIZE, 'x')),
                      "Keyword must not be empty");
     }},
    {"missing_file",
     [](const SearchRunner& runner) {
         expect_error(runner, "needle", runner.work_dir() / "missing.txt",
                      "File does not exist");
     }},
    {"directory",
     [](const SearchRunner& runner) {
         expect_error(runner, "needle", runner.work_dir(), "Error reading file");
     }},
    {"filename_argument",
     [](const SearchRunner& runner) {
         const fs::path path = runner.write_file("needle\nabc needle\nneedle\n");
         const Result result =
             runner.run_with_arguments("needle", '"' + path.string() + '"');
         const std::string expected = "Occurrences of keyword \"needle\": 3\n";

         if (!result.succeeded || result.output != expected) {
             throw std::runtime_error("expected stdout \"" + expected +
                                      "\", got: " +
                                      result.output.substr(0, 200));
         }
     }},
    {"too_many_arguments",
     [](const SearchRunner& runner) {
         const Result result = runner.run_with_arguments("needle", "one two");

         std::ifstream errors(runner.work_dir() / "errors.txt");
         const std::string message((std::istreambuf_iterator<char>(errors)),
                                   std::istreambuf_iterator<char>());

         if (result.succeeded || message.find("Usage:") == std::string::npos) {
             throw std::runtime_error("expected a usage error, got: " +
                                      message.substr(0, 200));
         }
     }},
};

int main(int argc, char* argv[]) {
    if (argc != 3 || TESTS.count(argv[2]) == 0) {
        std::cerr << "Usage: search_test <search binary> <test name>"
                  << std::endl;
        return 2;
    }

    const std::string test_name = argv[2];
    const fs::path work_dir = fs::current_path() / ("search_test_" + test_name);
    int exit_code = 0;

    try {
        fs::remove_all(work_dir);
        fs::create_directories(work_dir);
        TESTS.at(test_name)(SearchRunner(fs::absolute(argv[1]), work_dir));
    } catch (const std::exception& e) {
        std::cerr << test_name << ": " << e.what() << std::endl;
        exit_code = 1;
    }

    std::error_code ignored;
    fs::remove_all(work_dir, ignored);
    return exit_code;
}

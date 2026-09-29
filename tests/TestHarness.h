#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace test {

struct TestCase {
    std::string suiteName;
    std::string testName;
    std::function<void()> func;
};

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner s_runner;
        return s_runner;
    }

    void addTest(const std::string& suite, const std::string& name, std::function<void()> func) {
        m_tests.push_back({suite, name, std::move(func)});
    }

    int runAll() {
        int passed = 0;
        int failed = 0;
        std::cout << "\n==================================================\n";
        std::cout << "  ClipHub Test Suite (Windows 7 E5500 Target)  \n";
        std::cout << "==================================================\n";

        for (const auto& t : m_tests) {
            std::cout << "[ RUN      ] " << t.suiteName << "." << t.testName << " ... ";
            auto start = std::chrono::high_resolution_clock::now();
            try {
                t.func();
                auto end = std::chrono::high_resolution_clock::now();
                auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                std::cout << "\033[32mPASS\033[0m (" << us << " us)\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "\033[31mFAIL\033[0m: " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cout << "\033[31mFAIL\033[0m: Unknown exception thrown\n";
                failed++;
            }
        }

        std::cout << "==================================================\n";
        std::cout << "Total: " << (passed + failed) 
                  << " | \033[32mPassed: " << passed << "\033[0m"
                  << " | \033[" << (failed ? "31" : "32") << "mFailed: " << failed << "\033[0m\n";
        std::cout << "==================================================\n\n";

        return (failed == 0) ? 0 : 1;
    }

private:
    std::vector<TestCase> m_tests;
};

} // namespace test

#define TEST(suite, name) \
    void test_##suite##_##name(); \
    struct Register_##suite##_##name { \
        Register_##suite##_##name() { \
            test::TestRunner::instance().addTest(#suite, #name, test_##suite##_##name); \
        } \
    } g_reg_##suite##_##name; \
    void test_##suite##_##name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            throw std::runtime_error("Assertion failed: " #cond " at " __FILE__ ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            throw std::runtime_error("Assertion failed: " #a " == " #b " at " __FILE__ ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            throw std::runtime_error("Assertion failed: " #a " != " #b " at " __FILE__ ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

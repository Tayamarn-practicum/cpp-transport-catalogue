#pragma once

#include <cmath>
#include <iostream>
#include <string>

// -------- Utility functions for testing ----------

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cout << std::boolalpha;
        std::cout << file << static_cast<std::string>("(") << line << static_cast<std::string>("): ") << func << static_cast<std::string>(": ");
        std::cout << static_cast<std::string>("ASSERT_EQUAL(") << t_str << static_cast<std::string>(", ") << u_str << static_cast<std::string>(") failed: ");
        std::cout << t << static_cast<std::string>(" != ") << u << static_cast<std::string>(".");
        if (!hint.empty()) {
            std::cout << static_cast<std::string>(" Hint: ") << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T, typename U>
void AssertApproxEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    const double EPSILON = 1e-5;
    if (std::fabs(t - u) > EPSILON) {
        std::cout << std::fabs(t - u) << "     " << EPSILON << std::endl;
        std::cout << std::boolalpha;
        std::cout << file << static_cast<std::string>("(") << line << static_cast<std::string>("): ") << func << static_cast<std::string>(": ");
        std::cout << static_cast<std::string>("ASSERT_APPOX_EQUAL(") << t_str << static_cast<std::string>(", ") << u_str << static_cast<std::string>(") failed: ");
        std::cout << t << static_cast<std::string>(" != ") << u << static_cast<std::string>(".");
        if (!hint.empty()) {
            std::cout << static_cast<std::string>(" Hint: ") << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT_APPOX_EQUAL(a, b) AssertApproxEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_APPOX_EQUAL_HINT(a, b, hint) AssertApproxEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

inline void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
                const std::string& hint) {
    if (!value) {
        std::cout << file << static_cast<std::string>("(") << line << static_cast<std::string>("): ") << func << static_cast<std::string>(": ");
        std::cout << static_cast<std::string>("ASSERT(") << expr_str << static_cast<std::string>(") failed.");
        if (!hint.empty()) {
            std::cout << static_cast<std::string>(" Hint: ") << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(const T& t, const std::string& t_str) {
    t();
    std::cerr << t_str << static_cast<std::string>(" OK") << std::endl;
}

#define RUN_TEST(func)  RunTestImpl((func), #func)

// -------- Tests as is begin here ----------

namespace tests {
    void TCAddStop();

    void TCAddBus();

    void InputAddStop();

    void InputAddDist();

    void InputAddBusOneWay();

    void InputAddBusTwoWay();

    // This is the main testing function
    void RunTests();
}

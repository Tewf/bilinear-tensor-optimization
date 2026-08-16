#pragma once

#include <iostream>
#include <string>

/// A test is a program that prints what it checked and exits non-zero if any of
/// it was wrong. CTest supplies the reporting, so nothing else is needed and no
/// test framework is worth a dependency here.
namespace check {

inline int failure_count = 0;

inline void equal(const std::string& what, long long actual, long long expected) {
    if (actual == expected) {
        std::cout << "  ok    " << what << " = " << actual << "\n";
    } else {
        std::cout << "  FAIL  " << what << " = " << actual << ", expected " << expected << "\n";
        ++failure_count;
    }
}

inline int report(const std::string& suite) {
    if (failure_count == 0) {
        std::cout << suite << ": all checks passed\n";
        return 0;
    }
    std::cout << suite << ": " << failure_count << " check(s) failed\n";
    return 1;
}

}  // namespace check

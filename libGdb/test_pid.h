#pragma once
#include <cstdlib>
#include <unistd.h>
#include <string>

inline int test_get_oya_pid() {
    const char* s = std::getenv("MEL_TEST_OYA_PID");
    if (s && *s) {
        char* end = nullptr;
        long v = std::strtol(s, &end, 10);
        if (end && *end == '\0' && v > 0) return static_cast<int>(v);
    }
    return static_cast<int>(getpid());  // 既定は自分の PID
}

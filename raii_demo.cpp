// filename: raii_demo.cpp
// g++ -std=c++17 -O2 -o raii_demo raii_demo.cpp

#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <utility>   // std::exchange

class FileHandle {
public:
    explicit FileHandle(const char* path, const char* mode)
        : file_(std::fopen(path, mode))
    {
        if (!file_) {
            throw std::runtime_error(
                std::string("파일 열기 실패: ") + path);
        }
        std::cout << "[FileHandle] 파일 열림: " << path << "\n";
    }

    // 소멸자: 항상 호출됨 (정상 종료 or 예외 모두)
    ~FileHandle() {
        if (file_) {
            std::fclose(file_);
            std::cout << "[FileHandle] 파일 닫힘\n";
        }
    }

    // 복사 금지 — 파일 핸들은 고유 자원
    FileHandle(const FileHandle&)            = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // 이동 허용
    FileHandle(FileHandle&& other) noexcept
        : file_(std::exchange(other.file_, nullptr)) {}

    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (file_) std::fclose(file_);
            file_ = std::exchange(other.file_, nullptr);
        }
        return *this;
    }

    FILE* get() const noexcept { return file_; }
    explicit operator bool() const noexcept { return file_ != nullptr; }

    // 편의 메서드
    int write(const char* text) {
        return std::fputs(text, file_);
    }

private:
    FILE* file_;
};

// ScopeGuard — 어떤 코드든 스코프 종료 시 실행
class ScopeGuard {
public:
    template<typename F>
    explicit ScopeGuard(F&& fn)
        : fn_(std::forward<F>(fn)), active_(true) {}

    ~ScopeGuard() {
        if (active_) fn_();
    }

    // 조건부로 해제 가능
    void dismiss() noexcept { active_ = false; }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> fn_;
    bool active_;
};

void demonstrate_raii() {
    std::cout << "\n--- FileHandle 예외 안전성 테스트 ---\n";

    try {
        FileHandle f("/tmp/raii_test.txt", "w");
        f.write("RAII 테스트\n");

        // 예외 발생 시뮬레이션
        throw std::runtime_error("작업 중 에러 발생!");

        // 이 줄은 실행되지 않음
        std::cout << "이 메시지는 출력되지 않음\n";
    } catch (const std::exception& e) {
        std::cout << "예외 처리: " << e.what() << "\n";
        // FileHandle 소멸자는 예외 전에 이미 호출됨
    }

    std::cout << "\n--- ScopeGuard 테스트 ---\n";
    {
        ScopeGuard guard([] {
            std::cout << "[ScopeGuard] 정리 코드 실행\n";
        });

        std::cout << "작업 수행 중...\n";
        // 스코프 종료 시 guard 소멸자가 람다를 호출
    }
}

int main() {
    demonstrate_raii();
    return 0;
}

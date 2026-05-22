// filename: string_view_parser.cpp
// g++ -std=c++17 -O2 -Wall -o string_view_parser string_view_parser.cpp
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>

// CSV 한 줄을 string_view 기반으로 파싱 (복사 없음)
std::vector<std::string_view> split_sv(std::string_view str, char delim) {
    std::vector<std::string_view> parts;
    size_t start = 0;
    while (start < str.size()) {
        size_t end = str.find(delim, start);
        if (end == std::string_view::npos) end = str.size();
        parts.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    return parts;
}

// trim: 좌우 공백 제거 (복사 없음)
std::string_view trim(std::string_view sv) {
    while (!sv.empty() && std::isspace(sv.front())) sv.remove_prefix(1);
    while (!sv.empty() && std::isspace(sv.back()))  sv.remove_suffix(1);
    return sv;
}

// string_view에서 정수 변환 (C++17 std::from_chars)
std::optional<int> parse_int(std::string_view sv) {
    int value;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec == std::errc{}) return value;
    return std::nullopt;
}

// vector<char> 버퍼 활용 예제
std::vector<char> build_response(std::string_view status, std::string_view body) {
    std::vector<char> buf;
    buf.reserve(256 + body.size());

    auto append = [&](std::string_view sv) {
        buf.insert(buf.end(), sv.begin(), sv.end());
    };

    append("HTTP/1.1 ");
    append(status);
    append("\r\nContent-Length: ");

    char len_buf[20];
    auto [ptr, ec] = std::to_chars(len_buf, len_buf + 20, body.size());
    append({len_buf, ptr});
    append("\r\n\r\n");
    append(body);

    return buf;
}

int main() {
    // CSV 파싱 (복사 없음)
    const std::string csv_line = "name, age, score, city";
    auto parts = split_sv(csv_line, ',');
    std::cout << "CSV 파싱 결과:\n";
    for (size_t i = 0; i < parts.size(); ++i) {
        std::cout << "  [" << i << "] = '"
                  << trim(parts[i]) << "'\n";
    }

    // 정수 파싱
    std::cout << "\n정수 파싱:\n";
    std::string data = "42, -17, 100, xyz, 999";
    for (auto tok : split_sv(data, ',')) {
        auto trimmed = trim(tok);
        auto val = parse_int(trimmed);
        if (val) std::cout << "  '" << trimmed << "' = " << *val << "\n";
        else     std::cout << "  '" << trimmed << "' = (파싱 실패)\n";
    }

    // HTTP 응답 빌드 (vector<char> 버퍼)
    auto response = build_response("200 OK", "Hello, World!");
    std::cout << "\nHTTP 응답 버퍼 크기: " << response.size() << " bytes\n";
    std::cout << "내용:\n" << std::string_view(response.data(), response.size()) << "\n";

    // at() vs operator[] 안전성
    std::cout << "\n=== 범위 검사 ===\n";
    std::vector<int> v = {1, 2, 3};
    std::cout << "v[2] = " << v[2] << "\n";
    try {
        v.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "v.at(10) 예외: " << e.what() << "\n";
    }

    return 0;
}

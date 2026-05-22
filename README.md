# 깊이 파는 C++ 자료구조

C++17/C++20 STL 내부 구현, 메모리 모델, 캐시 최적화, Lock-Free 동시성, 커스텀 할당자를 깊이 다루는 교재의 예제 코드입니다.

## 브랜치 구조

각 챕터는 독립 브랜치로 관리됩니다.

```
part00/chapter-01  — 첫 번째 챕터
part00/chapter-02  — 두 번째 챕터 (누적)
...
```

## 빌드 방법

```bash
g++ -std=c++17 -O2 -Wall -Wextra -o output source.cpp
```

또는 CMake 사용:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## 교재

[text.ibetter.kr](https://text.ibetter.kr)에서 전체 교재를 읽을 수 있습니다.

// filename: ue_patterns_reference.cpp
// g++ -std=c++17 -O2 -o ue_patterns ue_patterns_reference.cpp
// Unreal Engine TArray 핵심 패턴 (개념 참조용)

#include <vector>
#include <cstddef>
#include <cstring>

// TArray는 Slack 기반 성장 전략을 씀
// AddUninitialized: 생성자 호출 없이 메모리만 확보
// Reserve: 슬랙 예약
// Shrink: 슬랙 제거

// 이 교재의 PART 04에서 다룬 vector::reserve와 동일 패턴
// 단, UE는 커스텀 힙 할당자(FDefaultAllocator)를 기본 사용

template<typename T>
class TArrayLike {
    T* data_  = nullptr;
    int num_  = 0;   // 실제 원소 수
    int max_  = 0;   // 할당된 용량 (슬랙 포함)
public:
    void reserve_slack(int new_max) {
        if (new_max > max_) {
            T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_max));
            if (data_) {
                std::memcpy(new_data, data_, sizeof(T) * num_);
                ::operator delete(data_);
            }
            data_ = new_data;
            max_  = new_max;
        }
    }
    void shrink() { reserve_slack(num_); }
    int num()  const { return num_; }
    int slack() const { return max_ - num_; }
    ~TArrayLike() { ::operator delete(data_); }
};

int main() {
    TArrayLike<int> arr;
    arr.reserve_slack(100);  // 슬랙 100 예약
    return 0;
}

// filename: numa_alloc.cpp
// g++ -std=c++17 -O2 -Wall -o numa_alloc numa_alloc.cpp -lnuma
// (Linux + libnuma 필요; macOS에서는 numactl 없음)

#ifdef __linux__
#include <numa.h>
#include <numaif.h>
#endif

#include <iostream>
#include <cstdlib>
#include <memory_resource>
#include <vector>
#include <chrono>
#include <cstring>

// ------------------------------------------------------------
// NUMA 정보 조회
// ------------------------------------------------------------
void print_numa_info() {
#ifdef __linux__
    if (numa_available() < 0) {
        std::cout << "NUMA 미지원 시스템\n";
        return;
    }
    int num_nodes = numa_num_configured_nodes();
    int num_cpus  = numa_num_configured_cpus();
    std::cout << "NUMA 노드 수: " << num_nodes << '\n';
    std::cout << "CPU 수: "       << num_cpus  << '\n';

    for (int node = 0; node < num_nodes; ++node) {
        long long size_mb = numa_node_size64(node, nullptr) / (1024 * 1024);
        std::cout << "  Node " << node << ": " << size_mb << " MB\n";
    }
#else
    std::cout << "(macOS/Windows: NUMA 직접 조회 불가, numactl 사용)\n";
#endif
}

// ------------------------------------------------------------
// NUMA-aware memory_resource
// ------------------------------------------------------------
class NumaMemoryResource : public std::pmr::memory_resource {
public:
    explicit NumaMemoryResource(int numa_node)
        : node_(numa_node) {}

protected:
    void* do_allocate(std::size_t bytes, std::size_t /*alignment*/) override {
#ifdef __linux__
        if (numa_available() >= 0) {
            void* ptr = numa_alloc_onnode(bytes, node_);
            if (ptr) return ptr;
        }
#endif
        // fallback: 일반 malloc
        return std::malloc(bytes);
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t) override {
#ifdef __linux__
        if (numa_available() >= 0) {
            numa_free(ptr, bytes);
            return;
        }
#endif
        std::free(ptr);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        const auto* other_numa = dynamic_cast<const NumaMemoryResource*>(&other);
        return other_numa && other_numa->node_ == node_;
    }

private:
    int node_;
};

// ------------------------------------------------------------
// 메모리 대역폭 측정 (간단한 memcpy 기반)
// ------------------------------------------------------------
long long measure_bandwidth_ns(char* src, char* dst, std::size_t size, int reps) {
    // warm-up
    std::memcpy(dst, src, size);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < reps; ++i) {
        std::memcpy(dst, src, size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / reps;
}

int main() {
    std::cout << "=== NUMA 시스템 정보 ===\n";
    print_numa_info();

    // --- NumaMemoryResource를 pmr 컨테이너에 연결 ---
    std::cout << "\n=== NUMA-aware pmr::vector ===\n";
    {
        NumaMemoryResource local_mr{0};  // NUMA 노드 0
        std::pmr::vector<int> v{&local_mr};

        for (int i = 0; i < 1000; ++i)
            v.push_back(i);

        long long sum = 0;
        for (int x : v) sum += x;
        std::cout << "합 (노드 0 메모리): " << sum << '\n';
    }

#ifdef __linux__
    // --- NUMA 로컬 vs 원격 메모리 접근 속도 비교 ---
    if (numa_available() >= 0 && numa_num_configured_nodes() >= 2) {
        std::cout << "\n=== 로컬 vs 원격 메모리 접근 비교 ===\n";
        constexpr std::size_t SIZE = 64 * 1024 * 1024;  // 64MB
        constexpr int REPS = 10;

        // 현재 스레드를 CPU 0 (노드 0)에 고정
        numa_run_on_node(0);

        char* local_src  = static_cast<char*>(numa_alloc_onnode(SIZE, 0));
        char* local_dst  = static_cast<char*>(numa_alloc_onnode(SIZE, 0));
        char* remote_src = static_cast<char*>(numa_alloc_onnode(SIZE, 1));
        char* remote_dst = static_cast<char*>(numa_alloc_onnode(SIZE, 1));

        if (local_src && local_dst && remote_src && remote_dst) {
            std::memset(local_src,  0xAA, SIZE);
            std::memset(remote_src, 0xBB, SIZE);

            long long t_local  = measure_bandwidth_ns(local_src,  local_dst,  SIZE, REPS);
            long long t_remote = measure_bandwidth_ns(remote_src, remote_dst, SIZE, REPS);

            double bw_local  = (double)SIZE / t_local  * 1000.0;  // GB/s
            double bw_remote = (double)SIZE / t_remote * 1000.0;

            std::cout << "로컬  memcpy (" << SIZE/1024/1024 << " MB): "
                      << t_local  / 1000 << " us, " << bw_local  << " GB/s\n";
            std::cout << "원격  memcpy (" << SIZE/1024/1024 << " MB): "
                      << t_remote / 1000 << " us, " << bw_remote << " GB/s\n";
            std::cout << "원격/로컬 비: " << (double)t_remote / t_local << "x 느림\n";
        }

        if (local_src)  numa_free(local_src,  SIZE);
        if (local_dst)  numa_free(local_dst,  SIZE);
        if (remote_src) numa_free(remote_src, SIZE);
        if (remote_dst) numa_free(remote_dst, SIZE);
    } else {
        std::cout << "\n(단일 소켓 시스템: 원격 메모리 비교 생략)\n";
    }
#else
    std::cout << "\n(macOS: NUMA 직접 측정 불가)\n";
    std::cout << "numactl 시뮬레이션:\n";
    std::cout << "  numactl --membind=0 --cpunodebind=0 ./program  # 노드 0에 고정\n";
    std::cout << "  numactl --membind=1 --cpunodebind=0 ./program  # 원격 메모리 강제\n";
#endif

    // --- hwloc 기반 토폴로지 감지 (의사 코드) ---
    std::cout << "\n=== hwloc 기반 NUMA-aware 설계 패턴 ===\n";
    std::cout << R"(
// hwloc 라이브러리: 하드웨어 토폴로지 자동 감지
// sudo apt-get install libhwloc-dev
//
// hwloc_topology_t topology;
// hwloc_topology_init(&topology);
// hwloc_topology_load(&topology);
//
// int num_nodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NUMANODE);
// for (int i = 0; i < num_nodes; ++i) {
//     hwloc_obj_t node = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NUMANODE, i);
//     // 노드에 가장 가까운 CPU 목록 → 스레드 친화성 설정
// }
//
// 스레드 핀 + NUMA-aware 할당:
//   1) hwloc로 스레드를 특정 NUMA 노드의 CPU에 고정
//   2) NumaMemoryResource(node_id)로 해당 노드 메모리 할당
//   3) 스레드가 처리할 데이터가 항상 로컬 메모리에 존재
)";

    return 0;
}

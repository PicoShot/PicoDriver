#include "utils/driver.h"
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <iomanip>
#include <iostream>
#include <memory>
#undef max

namespace cache
{
    uintptr_t moduleBase;
}

struct LargeReadBlock {
    uint8_t data[4096];
};

class BenchmarkMetrics {
public:
    BenchmarkMetrics() : totalReads(0), averageReadsPerSecond(0), peakReadsPerSecond(0) {}

    // Delete copy constructor and assignment operator
    BenchmarkMetrics(const BenchmarkMetrics&) = delete;
    BenchmarkMetrics& operator=(const BenchmarkMetrics&) = delete;

    std::atomic<uint64_t> totalReads;
    double averageReadsPerSecond;
    double peakReadsPerSecond;
    std::vector<double> readsPerSecondHistory;
    std::mutex historyMutex;

    void addHistoryPoint(double rps) {
        std::lock_guard<std::mutex> lock(historyMutex);
        readsPerSecondHistory.push_back(rps);
        peakReadsPerSecond = std::max(peakReadsPerSecond, rps);
    }

    std::vector<double> getLastHistory(size_t count) {
        std::lock_guard<std::mutex> lock(historyMutex);
        if (readsPerSecondHistory.size() <= count) {
            return readsPerSecondHistory;
        }
        return std::vector<double>(
            readsPerSecondHistory.end() - count,
            readsPerSecondHistory.end()
        );
    }
};

void printMetrics(BenchmarkMetrics& metrics, size_t readSize) {
    std::cout << "\033[2J\033[1;1H";  // Clear screen and move cursor to top
    std::cout << "=== Multi-threaded Driver Benchmark Metrics ===\n";
    std::cout << "Total Reads: " << metrics.totalReads.load() << "\n";
    std::cout << "Average Reads/sec: " << std::fixed << std::setprecision(2)
        << metrics.averageReadsPerSecond << "\n";
    std::cout << "Peak Reads/sec: " << metrics.peakReadsPerSecond << "\n";
    std::cout << "Memory Read Size: " << readSize << " bytes\n";

    double totalDataRead = (metrics.totalReads.load() * readSize) / (1024.0 * 1024.0);
    std::cout << "Total Data Read: " << totalDataRead << " MB\n";
    std::cout << "Data Read Rate: " << (totalDataRead / metrics.averageReadsPerSecond) << " MB/s\n";

    std::cout << "Last 5 seconds (reads/sec): ";
    auto lastHistory = metrics.getLastHistory(5);
    for (double rps : lastHistory) {
        std::cout << std::fixed << std::setprecision(2) << rps << " ";
    }
    std::cout << "\n";
}

void workerThread(BenchmarkMetrics& metrics,
    std::atomic<bool>& running,
    size_t threadId,
    size_t readOffset) {
    while (running) {
        driver::Read<LargeReadBlock>(cache::moduleBase + readOffset + (threadId * sizeof(LargeReadBlock)));
        metrics.totalReads++;
    }
}

void runMultiThreadedBenchmark(BenchmarkMetrics& metrics, int durationSeconds, int numThreads) {
    std::atomic<bool> running = true;
    std::vector<std::thread> threads;

    const auto startTime = std::chrono::high_resolution_clock::now();
    auto lastSecond = startTime;
    uint64_t lastTotalReads = 0;

    std::cout << "Starting benchmark with " << numThreads << " threads for "
        << durationSeconds << " seconds...\n";

    // Start worker threads
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread,
            std::ref(metrics),
            std::ref(running),
            i,
            i * 1024);
    }

    // Monitor and update metrics
    while (true) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto totalDuration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        if (totalDuration >= durationSeconds) {
            running = false;
            break;
        }

        const auto secondElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSecond).count();
        if (secondElapsed >= 1) {
            uint64_t currentReads = metrics.totalReads.load();
            uint64_t readsThisSecond = currentReads - lastTotalReads;
            double rps = static_cast<double>(readsThisSecond) / secondElapsed;

            metrics.addHistoryPoint(rps);
            metrics.averageReadsPerSecond = static_cast<double>(currentReads) / totalDuration;
            lastTotalReads = currentReads;
            lastSecond = now;

            printMetrics(metrics, sizeof(LargeReadBlock));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
}

int main()
{
    std::cout << "Multi-threaded Driver Benchmark Tool\n";
    std::cout << "Initializing...\n";

    if (!driver::AttachToProcess(L"notepad.exe")) {
        std::cerr << "Failed to attach to notepad.exe\n";
        return 1;
    }
    std::cout << "Successfully attached to notepad.exe\n";

    cache::moduleBase = driver::GetModuleBaseByName(L"ntdll.dll");
    if (!cache::moduleBase) {
        std::cerr << "Failed to get ntdll.dll base address\n";
        return 1;
    }
    std::cout << "ntdll.dll base address: 0x" << std::hex << cache::moduleBase << std::dec << "\n";

    // Get number of CPU cores for optimal thread count
    const int numThreads = std::thread::hardware_concurrency() * 2;
    std::cout << "Running benchmark with " << numThreads << " threads\n";

    const int benchmarkDuration = 60;
    BenchmarkMetrics metrics;
    runMultiThreadedBenchmark(metrics, benchmarkDuration, numThreads);

    // Print final results
    std::cout << "\nFinal Benchmark Results:\n";
    std::cout << "========================\n";
    std::cout << "Total Duration: " << benchmarkDuration << " seconds\n";
    std::cout << "Number of Threads: " << numThreads << "\n";
    std::cout << "Total Reads Performed: " << metrics.totalReads.load() << "\n";
    std::cout << "Average Reads/sec: " << std::fixed << std::setprecision(2)
        << metrics.averageReadsPerSecond << "\n";
    std::cout << "Peak Reads/sec: " << metrics.peakReadsPerSecond << "\n";
    std::cout << "Memory Read Size: " << sizeof(LargeReadBlock) << " bytes\n";
    double totalDataRead = (metrics.totalReads.load() * sizeof(LargeReadBlock)) / (1024.0 * 1024.0);
    std::cout << "Total Data Read: " << totalDataRead << " MB\n";
    std::cout << "Average Data Read Rate: " << (totalDataRead / benchmarkDuration) << " MB/s\n";

    system("pause");
    return 0;
}
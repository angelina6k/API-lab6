// Частина 1. Race condition: три реалізації
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdlib>

constexpr int ITERATIONS = 5 * 100000; // варіант 5: N = 5
int counterMutexVersion = 0;
std::mutex counterMutex;
std::atomic<int> atomicCounter{ 0 };
int unsafeCounter = 0;

void incrementUnsafe() {
    for (int i = 0; i < ITERATIONS; ++i) ++unsafeCounter;
}
void incrementMutex() {
    for (int i = 0; i < ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(counterMutex);
        ++counterMutexVersion;
    }
}
void incrementAtomic() {
    for (int i = 0; i < ITERATIONS; ++i) ++atomicCounter;
}

int main() {
    system("chcp 65001 > nul");
    setlocale(LC_ALL, ".UTF8");

    const int expected = 2 * ITERATIONS;

    std::thread a(incrementUnsafe), b(incrementUnsafe);
    a.join(); b.join();
    std::cout << "[Без синхронізації] Очікується: " << expected
        << ", отримано: " << unsafeCounter << '\n';

    std::thread c(incrementMutex), d(incrementMutex);
    c.join(); d.join();
    std::cout << "[std::mutex] Очікується: " << expected
        << ", отримано: " << counterMutexVersion << '\n';

    std::thread e(incrementAtomic), f(incrementAtomic);
    e.join(); f.join();
    std::cout << "[std::atomic] Очікується: " << expected
        << ", отримано: " << atomicCounter.load() << '\n';
}

// Частина 4. Варіант 5: підрахунок елементів масиву,
// значення яких належить заданому діапазону.
// Послідовний та паралельний варіанти std::async/std::future. C++20.
#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>
#include <cstdlib>


using Clock = std::chrono::steady_clock;
using ull = unsigned long long;

ull countRangeSequential(const std::vector<int>& data, int low, int high) {
    ull count = 0;
    for (int value : data)
        if (value >= low && value <= high) ++count;
    return count;
}

ull countRangeBlock(const std::vector<int>& data,
    std::size_t begin, std::size_t end,
    int low, int high) {
    ull count = 0;
    for (std::size_t i = begin; i < end; ++i)
        if (data[i] >= low && data[i] <= high) ++count;
    return count;
}

ull countRangeParallel(const std::vector<int>& data,
    int low, int high, unsigned taskCount) {
    taskCount = std::max(1u, taskCount);
    const std::size_t blockSize = (data.size() + taskCount - 1) / taskCount;
    std::vector<std::future<ull>> futures;

    for (unsigned task = 0; task < taskCount; ++task) {
        const std::size_t begin = task * blockSize;
        const std::size_t end = std::min(data.size(), begin + blockSize);
        if (begin >= end) break;

        futures.push_back(std::async(std::launch::async,
            [&data, begin, end, low, high] {
                return countRangeBlock(data, begin, end, low, high);
            }));
    }

    ull total = 0;
    for (auto& future : futures) total += future.get();
    return total;
}

int main() {
    system("chcp 65001 > nul");
    setlocale(LC_ALL, ".UTF8");

    constexpr int LOW = 200;
    constexpr int HIGH = 700;
    const unsigned tasks = 4;
    const std::size_t sizes[] = { 1'000'000, 5'000'000, 10'000'000 };

    std::mt19937 gen(2026);
    std::uniform_int_distribution<int> dist(0, 1000);

    std::cout << "Діапазон підрахунку: [" << LOW << "; " << HIGH << "]\n";
    std::cout << "Кількість асинхронних задач: " << tasks << "\n\n";
    std::cout << std::left << std::setw(14) << "Розмір"
        << std::setw(18) << "Послідовно, мс"
        << std::setw(18) << "Паралельно, мс"
        << std::setw(14) << "Прискорення" << '\n';

    for (std::size_t n : sizes) {
        std::vector<int> data(n);
        for (int& x : data) x = dist(gen);

        auto startSeq = Clock::now();
        const ull seqResult = countRangeSequential(data, LOW, HIGH);
        auto endSeq = Clock::now();

        auto startPar = Clock::now();
        const ull parResult = countRangeParallel(data, LOW, HIGH, tasks);
        auto endPar = Clock::now();

        const double seqMs =
            std::chrono::duration<double, std::milli>(endSeq - startSeq).count();
        const double parMs =
            std::chrono::duration<double, std::milli>(endPar - startPar).count();

        std::cout << std::left << std::setw(14) << n
            << std::setw(18) << std::fixed << std::setprecision(3) << seqMs
            << std::setw(18) << parMs
            << std::setw(14) << (parMs > 0 ? seqMs / parMs : 0.0)
            << '\n';

        std::cout << "  Результати: послідовний = " << seqResult
            << ", паралельний = " << parResult
            << (seqResult == parResult ? " [OK]\n" : " [ПОМИЛКА]\n");
    }
}


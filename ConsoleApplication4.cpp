// Частина 3. Варіант 5: потокове читання сенсора температури
// Виробник генерує температурні показники; споживач обчислює
// ковзне середнє та виявляє різкі стрибки. C++20.
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <deque>
#include <random>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <cstdlib>

struct Reading {
    int id;
    double temperature;
};

std::queue<Reading> readings;
std::mutex queueMutex, outputMutex;
std::condition_variable condition;
bool finished = false;
constexpr int READING_COUNT = 30;
constexpr std::size_t WINDOW_SIZE = 5;
constexpr double JUMP_LIMIT = 8.0; // поріг різкого стрибка, °C

void producer() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> normalTemp(18.0, 27.0);

    for (int i = 1; i <= READING_COUNT; ++i) {
        double value = normalTemp(gen);
        // Два контрольовані стрибки для демонстрації виявлення аномалій.
        if (i == 12) value = 39.5;
        if (i == 23) value = 10.0;

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            readings.push({ i, value });
        }
        condition.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        finished = true;
    }
    condition.notify_all();
}

void consumer() {
    std::deque<double> window;
    double previous = 0.0;
    bool hasPrevious = false;

    while (true) {
        Reading current{};
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [] {
                return !readings.empty() || finished;
                });

            if (readings.empty() && finished) break;

            current = readings.front();
            readings.pop();
        }

        window.push_back(current.temperature);
        if (window.size() > WINDOW_SIZE) window.pop_front();

        double sum = 0.0;
        for (double t : window) sum += t;
        const double average = sum / window.size();

        const bool anomaly = hasPrevious &&
            std::abs(current.temperature - previous) >= JUMP_LIMIT;

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << "Показник #" << std::setw(2) << current.id
                << ": " << std::fixed << std::setprecision(2)
                << current.temperature << " °C"
                << " | ковзне середнє (" << window.size()
                << "): " << average << " °C";
            if (anomaly) std::cout << " | АНOMАЛІЯ: різкий стрибок";
            std::cout << '\n';
        }

        previous = current.temperature;
        hasPrevious = true;
    }

    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Споживач завершив роботу.\n";
}

int main() {
    system("chcp 65001 > nul");
    setlocale(LC_ALL, ".UTF8");

    std::thread producerThread(producer);
    std::thread consumerThread(consumer);

    producerThread.join();
    consumerThread.join();

    std::cout << "Усі показники сенсора оброблено.\n";
}

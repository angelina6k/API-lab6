// Частина 2. Демонстрація deadlock. 
// Очікувано зависає; зупинити вручну. Стандарт C++20.
#include <iostream>
#include <thread>
#include <mutex>
#include <barrier>

std::mutex mutexA, mutexB;
std::barrier syncPoint(2);

void task1() {
    std::cout << "Thread 1: locking mutexA\n";
    std::lock_guard<std::mutex> a(mutexA);
    std::cout << "Thread 1: mutexA locked\n";
    syncPoint.arrive_and_wait();
    std::cout << "Thread 1: waiting for mutexB\n";
    std::lock_guard<std::mutex> b(mutexB);
}
void task2() {
    std::cout << "Thread 2: locking mutexB\n";
    std::lock_guard<std::mutex> b(mutexB);
    std::cout << "Thread 2: mutexB locked\n";
    syncPoint.arrive_and_wait();
    std::cout << "Thread 2: waiting for mutexA\n";
    std::lock_guard<std::mutex> a(mutexA);
}
int main() {
    std::thread t1(task1), t2(task2);
    t1.join(); t2.join();
}
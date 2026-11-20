// Частина 2Б. Два способи усунення deadlock. C++20
#include <iostream>
#include <thread>
#include <mutex>
#include <barrier>
#include <cstdlib>

std::mutex mutexA, mutexB;
std::barrier syncPoint(2);

// Спосіб 1: одночасне захоплення двох м'ютексів
void taskScoped1() {
    syncPoint.arrive_and_wait();
    std::scoped_lock lock(mutexA, mutexB);
    std::cout << "Thread 1: both mutexes locked (scoped_lock)\n";
}
void taskScoped2() {
    syncPoint.arrive_and_wait();
    std::scoped_lock lock(mutexB, mutexA);
    std::cout << "Thread 2: both mutexes locked (scoped_lock)\n";
}

// Спосіб 2: усі потоки захоплюють м'ютекси в однаковому порядку A -> B
void taskOrder1() {
    syncPoint.arrive_and_wait();
    std::lock_guard<std::mutex> a(mutexA);
    std::lock_guard<std::mutex> b(mutexB);
    std::cout << "Thread 1: both mutexes locked (A -> B)\n";
}
void taskOrder2() {
    syncPoint.arrive_and_wait();
    std::lock_guard<std::mutex> a(mutexA);
    std::lock_guard<std::mutex> b(mutexB);
    std::cout << "Thread 2: both mutexes locked (A -> B)\n";
}

int main() {
    system("chcp 65001 > nul");
    setlocale(LC_ALL, ".UTF8");

    // Запустіть лише ОДИН блок за раз.
    std::thread t1(taskScoped1), t2(taskScoped2);
    t1.join(); t2.join();

    // Для другого способу закоментуйте попередні 3 рядки
    // та розкоментуйте:
    //std::thread p1(taskOrder1), p2(taskOrder2);
    //p1.join(); p2.join();
}
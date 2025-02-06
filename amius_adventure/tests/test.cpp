#include <tuple>
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include "channel.hpp"

void channels() {
    std::cout << "testing channels... ";
    Sender<int>* tx;
    Receiver<int>* rx;
    std::tie(tx, rx) = make_channel<int>();

    tx->send(5);
    tx->send(2);

    // queue and recieve
    RecvStatus status;
    std::optional<int> recvInt;
    std::tie(status, recvInt) = rx->recv();
    assert(status == RECV_OK);
    assert(*recvInt == 2); // in this queue the first entry handled is the newest entry added, like a stack
    std::tie(status, recvInt) = rx->recv();
    assert(status == RECV_OK);
    assert(*recvInt == 5);

    // empty
    std::tie(status, recvInt) = rx->recv();
    assert(status == RECV_EMPTY);

    // timeout
    std::thread thread([rx](){
        rx->recvTimeout(std::chrono::seconds(2));
    });

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    thread.join();
    assert(std::chrono::steady_clock::now() - start >= std::chrono::seconds(2));

    // hung up
    delete tx;
    std::tie(status, recvInt) = rx->recv();
    assert(status == RECV_HUNG_UP);

    // finish
    delete rx;

    std::cout << "success!" << std::endl;
}

int main() {
    channels();
}
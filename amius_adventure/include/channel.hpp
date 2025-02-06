#pragma once
#ifndef AMIUS_ADVENTURE_CHANNEL
#include <tuple>
#include <vector>
#include <stdint.h>
#include <optional>
#include <thread>
#include <chrono>

#define MAX_CHANNEL_QUEUE_SIZE 16

template<typename T>
class Channel {
private:
    std::vector<T> queue;
public:
    Channel() {
        queue = std::vector<T>();
    }
    /// @brief check for any data on the channel then return
    /// @return T if there is data, null if there is no data on the channel
    std::optional<T> recv() {
        if (queue.size() <= 0 ) {
            return std::optional<T>{};
        }
        T data = queue.back();
        queue.pop_back();
        return data;
    }
    /// @brief puts data in the queue
    /// @param data data to send
    /// @return true if data was added to the queue, false if cue is full
    bool send(T data) {
        if (queue.size() >= MAX_CHANNEL_QUEUE_SIZE) {
            return false;
        }
        queue.push_back(data);
        return true;
    }
};

enum SendStatus {
    SEND_OK,
    SEND_FULL,
    SEND_HUNG_UP
};

template<typename T>
class Sender {
private:
    std::optional<Channel<T>>* channel;
public:
    Sender() : channel(new std::optional<Channel<T>>{}) {}
    Sender(std::optional<Channel<T>>* channel) : channel(channel) {}
    /// @brief Attempts to send data on the channel
    /// @param data data you wish to send
    /// @return send status
    SendStatus send(T data) {
        if (!this->channel->has_value()) {
            return SEND_HUNG_UP;
        }
        if (!(*this->channel)->send(data)) {
            return SEND_FULL;
        }
        return SEND_OK;
    }
    ~Sender() {
        if (this->channel->has_value()) {
            *this->channel = std::optional<Channel<T>>{};
        }
        else {
            delete this->channel;
        }
    }
};

enum RecvStatus {
    RECV_OK,
    RECV_EMPTY,
    RECV_HUNG_UP
};

template<typename T>
class Receiver {
private:
    std::optional<Channel<T>>* channel;
public:
    Receiver() : channel(new std::optional<Channel<T>>{}) {}
    Receiver(std::optional<Channel<T>>* channel) : channel(channel) {}
    std::tuple<RecvStatus, std::optional<T>> recv() {
        if (!this->channel->has_value()) {
            return std::make_tuple(RECV_HUNG_UP, std::optional<T>{});
        }
        std::optional<T> data = (*this->channel)->recv();
        return std::make_tuple(!data.has_value() ? RECV_EMPTY : RECV_OK, data);
    }
    /// @brief runs recv in a loop until data appears on the channel
    /// @return T
    std::tuple<RecvStatus, std::optional<T>> recvBlock() {
        RecvStatus status = RECV_EMPTY;
        std::optional<T> data = std::optional<T>{};
        while (status == RECV_EMPTY) {
            std::tie(status, data) = this->recv();
            if (status == RECV_HUNG_UP) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100)); // we dont want to waste battery by checking as often as possible
        }
        return std::make_tuple(status, data);
    }
    std::tuple<RecvStatus, std::optional<T>> recvTimeout(std::chrono::duration<uint32_t> duration) {
        std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
        RecvStatus status = RECV_EMPTY;
        std::optional<T> data = std::optional<T>{};
        while (status == RECV_EMPTY) {
            std::tie(status, data) = this->recv();
            if (status == RECV_HUNG_UP) {
                break;
            }
            if (std::chrono::steady_clock::now() - timestamp >= duration) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100)); // we dont want to waste battery by checking as often as possible
        }
        return std::make_tuple(status, data);
    }
    ~Receiver() {
        if (this->channel->has_value()) {
            *this->channel = std::optional<Channel<T>>{};
        }
        else {
            delete this->channel;
        }
    }
};


template<typename T>
std::tuple<Sender<T>*, Receiver<T>*> make_channel() {
    std::optional<Channel<T>>* channel = new std::optional<Channel<T>>{Channel<T>()};
    Sender<T>* tx = new Sender<T>(channel);
    Receiver<T>* rx = new Receiver<T>(channel);
    auto ret = std::make_tuple(tx, rx);
    return ret; //crashes here
}

#define AMIUS_ADVENTURE_CHANNEL
#endif
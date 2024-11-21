#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/select.h>

const int UDP_PORT = 12345;
const int TCP_PORT = 54321;
const int BROADCAST_ADDR = INADDR_BROADCAST;
const int MAX_WORKERS = 2;
const int TIMEOUT = 5; // seconds

std::mutex mtx;
std::condition_variable cv;
std::map<std::string, bool> workers;
std::vector<std::pair<std::string, double>> results;

void broadcast_discovery() {
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("socket");
        return;
    }

    int broadcastEnable = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    struct sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = htonl(BROADCAST_ADDR);
    broadcastAddr.sin_port = htons(UDP_PORT);

    const char* message = "DISCOVER_WORKERS";
    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    char buffer[1024];
    struct sockaddr_in workerAddr;
    socklen_t addrLen = sizeof(workerAddr);

    while (true) {
        int bytesReceived = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&workerAddr, &addrLen);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string workerIP = inet_ntoa(workerAddr.sin_addr);
            std::unique_lock<std::mutex> lock(mtx);
            workers[workerIP] = true;
            if (workers.size() >= MAX_WORKERS) break;
        }
    }

    close(udp_socket);
}

void assign_tasks() {
    // Assuming the function to be integrated is f(x) = x^2
    auto f = [](double x) { return x * x; };

    double a = 0.0, b = 1.0; // Integration limits
    int n = workers.size(); // Number of segments
    double h = (b - a) / n;

    fd_set readfds;
    struct timeval timeout;
    std::map<int, std::pair<std::string, double>> taskMap;

    int max_fd = 0;
    int i = 0;
    for (const auto& worker : workers) {
        double start = a + i * h;
        double end = start + h;

        int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_socket < 0) {
            perror("socket");
            continue;
        }

        int optval = 1;
        setsockopt(tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

        struct sockaddr_in workerAddr;
        memset(&workerAddr, 0, sizeof(workerAddr));
        workerAddr.sin_family = AF_INET;
        workerAddr.sin_addr.s_addr = inet_addr(worker.first.c_str());
        workerAddr.sin_port = htons(TCP_PORT);

        if (connect(tcp_socket, (struct sockaddr*)&workerAddr, sizeof(workerAddr)) < 0) {
            perror("connect");
            close(tcp_socket);
            workers[worker.first] = false;
            continue;
        }

        std::string task = std::to_string(start) + " " + std::to_string(end);
        send(tcp_socket, task.c_str(), task.size(), 0);

        FD_SET(tcp_socket, &readfds);
        taskMap[tcp_socket] = {worker.first, 0.0};
        if (tcp_socket > max_fd) max_fd = tcp_socket;

        ++i;
    }

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    while (!taskMap.empty()) {
        fd_set tmp_fds = readfds;
        int activity = select(max_fd + 1, &tmp_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("select");
            break;
        }

        if (activity == 0) {
            std::cerr << "Timeout occurred, some workers may be down." << std::endl;
            break;
        }

        for (auto it = taskMap.begin(); it != taskMap.end();) {
            int sock = it->first;
            if (FD_ISSET(sock, &tmp_fds)) {
                char buffer[1024];
                int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    double result = std::stod(buffer);
                    std::unique_lock<std::mutex> lock(mtx);
                    results.push_back({it->second.first, result});
                }
                close(sock);
                FD_CLR(sock, &readfds);
                it = taskMap.erase(it);
            } else {
                ++it;
            }
        }
    }

    cv.notify_one();
}

int main() {
    std::thread discoveryThread(broadcast_discovery);
    discoveryThread.join();

    std::thread taskThread(assign_tasks);
    taskThread.join();

    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return results.size() == workers.size(); });

    double totalResult = 0.0;
    for (const auto& result : results) {
        totalResult += result.second;
    }

    std::cout << "Total integral result: " << totalResult << std::endl;

    return 0;
}
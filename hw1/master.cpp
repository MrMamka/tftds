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
#include <unordered_map>

#include "helper.h"

const int BROADCAST_TIMEOUT = 5; // seconds

std::mutex mtx;
std::condition_variable cv;
std::map<std::string, bool> workers;
std::vector<std::pair<std::string, double>> results;

struct Master {
    Master(std::string addr): addr(addr) {}

    void StartMaster() {
        char server_message[256] = "You have reached the server!";

        int server_socket;
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("socket");
            return;
        }

        // TODO:
        //        int optval = 1;
        //setsockopt(tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(TCP_PORT);
        server_address.sin_addr.s_addr = inet_addr(addr.c_str());

        if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
            perror("bind");
            close(server_socket);
            return;
        }

        if (listen(server_socket, MAX_WORKERS) < 0) {
            perror("listen");
            close(server_socket);
            return;
        }

        this->DoBroadcast();

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = BROADCAST_TIMEOUT;
        timeout.tv_usec = 0;

        while(true) {
            int activity = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
            if (activity < 0) {
                perror("select");
                close(server_socket);
                return;
            }

            if (activity == 0) {
                std::cout << "Timeout occurred." << std::endl;
                break;
            }

            if (FD_ISSET(server_socket, &read_fds)) {
                int client_socket = accept(server_socket, NULL, NULL);
                if (client_socket < 0) {
                    perror("accept");
                    close(server_socket);
                    return;
                }

                std::cout << "Accepted connection" << std::endl;
                
                workers.push_back(WorkerInfo{socket: client_socket, is_available: true});
                // send(client_socket, server_message, sizeof(server_message), 0);
                // close(client_socket); // remove!!!
            }
        }

        ScheduleTasks();

        close(server_socket);
    }

    void ScheduleTasks() {
        PrepareTasks();

        for (size_t i = 0; i < workers.size(); ++i) {
            char request[256];
            memcpy(request, &tasks.at(i).start, sizeof(double));
            memcpy(request + sizeof(double), &tasks.at(i).end, sizeof(double));
            memcpy(request + 2 * sizeof(double), &i, sizeof(int));

            send(workers.at(i).socket, request, sizeof(request), 0);
        }

        // gave tasks
    }

    void PrepareTasks() {
        double a = 0.0, b = 10.0;
        int n = workers.size();
        double h = (b - a) / n;

        for (size_t i = 0; i < workers.size(); ++i) {
            double start = a + i * h;
            double end = start + h;

            TaskInfo task{start: start, end: end};
            tasks[i] = task;
        }
    }

    void DoBroadcast() {
        // std::cout << "Broadcast started" << std::endl;

        int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket < 0) {
            perror("socket");
            return;
        }

        int broadcastEnable = 1;
        setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

        struct sockaddr_in udpAddr;
        memset(&udpAddr, 0, sizeof(udpAddr));
        udpAddr.sin_family = AF_INET;
        udpAddr.sin_addr.s_addr = inet_addr(addr.c_str());
        udpAddr.sin_port = htons(UDP_PORT);

        if (bind(udp_socket, (struct sockaddr*)&udpAddr, sizeof(udpAddr)) < 0) {
            perror("bind");
            close(udp_socket);
            return;
        }

        struct sockaddr_in broadcastAddr;
        memset(&broadcastAddr, 0, sizeof(broadcastAddr));
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        broadcastAddr.sin_port = htons(UDP_PORT);

        const char* message = "DISCOVER_WORKERS";
        sendto(udp_socket, message, strlen(message), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

        close(udp_socket);
    }

    struct WorkerInfo {
        int socket;
        bool is_available;
    };

    struct TaskInfo {
        double start;
        double end;
    };

    std::string addr;
    std::vector<WorkerInfo> workers;
    std::unordered_map<size_t, TaskInfo> tasks;
};

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

    timeout.tv_sec = BROADCAST_TIMEOUT;
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
    Master master("127.0.0.0");
    master.StartMaster();
    
    return 0;
}

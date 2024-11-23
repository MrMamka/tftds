#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include "helper.h"

double integrate(double start, double end) {
    // Simple trapezoidal rule for integration
    auto f = [](double x) { return x * x; };
    int n = 1000;
    double h = (end - start) / n;
    double sum = 0.5 * (f(start) + f(end));
    for (int i = 1; i < n; ++i) {
        sum += f(start + i * h);
    }
    return sum * h;
}

struct Worker {
    void ConnectToMaster() {
        master_socket = socket(AF_INET, SOCK_STREAM, 0);
        
        if (connect(master_socket, (struct sockaddr *) &master_addr, sizeof(master_addr)) == -1) {
            perror("connection");
        }

        char server_response[256];
	    recv(master_socket, &server_response, sizeof(server_response), 0);

        double start;
        double end;
        size_t task_id; 
        memcpy(&start, server_response, sizeof(double));
        memcpy(&end, server_response + sizeof(double), sizeof(double));
        memcpy(&task_id, server_response + 2 * sizeof(double), sizeof(int));

        printf("The server sent the data: %f %f %zu\n", start, end, task_id);

	    close(master_socket);
    }

    void FindMasterAtBroadcast() {
        int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket < 0) {
            perror("socket");
        }

        int broadcastEnable = 1;
        if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST | SO_REUSEPORT | SO_REUSEADDR, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
            throw std::runtime_error("Failed to set broadcast option");
            close(udp_socket);
        }

        struct sockaddr_in broadcastAddr;
        memset(&broadcastAddr, 0, sizeof(broadcastAddr));
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        broadcastAddr.sin_port = htons(UDP_PORT);

        if (bind(udp_socket, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) < 0) {
            perror("bind");
            close(udp_socket);
        }

        char buffer[1024];
        struct sockaddr_in masterAddr;
        socklen_t addrLen = sizeof(masterAddr);

        int bytesReceived = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&masterAddr, &addrLen);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            if (strcmp(buffer, "DISCOVER_WORKERS") == 0) {
                masterAddr.sin_port = htons(TCP_PORT);
                master_addr = masterAddr;
                std::cout << "Got master addr: " << inet_ntoa(master_addr.sin_addr) << std::endl;
            }
        }

        close(udp_socket);
    }

    sockaddr_in master_addr;
    int master_socket;
};

int main() {
    Worker worker;
    worker.FindMasterAtBroadcast();
    worker.ConnectToMaster();

    return 0;
}

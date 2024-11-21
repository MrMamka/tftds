#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const int UDP_PORT = 12345;
const int TCP_PORT = 54321;

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

int main() {
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in udpAddr;
    memset(&udpAddr, 0, sizeof(udpAddr));
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpAddr.sin_port = htons(UDP_PORT);

    if (bind(udp_socket, (struct sockaddr*)&udpAddr, sizeof(udpAddr)) < 0) {
        perror("bind");
        close(udp_socket);
        return 1;
    }

    char buffer[1024];
    struct sockaddr_in masterAddr;
    socklen_t addrLen = sizeof(masterAddr);

    int bytesReceived = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&masterAddr, &addrLen);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        if (strcmp(buffer, "DISCOVER_WORKERS") == 0) {
            const char* response = "WORKER_AVAILABLE";
            sendto(udp_socket, response, strlen(response), 0, (struct sockaddr*)&masterAddr, addrLen);
        }
    }

    close(udp_socket);

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("socket");
        return 1;
    }

    int optval = 1;
    setsockopt(tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

    struct sockaddr_in tcpAddr;
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpAddr.sin_port = htons(TCP_PORT);

    if (bind(tcp_socket, (struct sockaddr*)&tcpAddr, sizeof(tcpAddr)) < 0) {
        perror("bind");
        close(tcp_socket);
        return 1;
    }

    listen(tcp_socket, 1);

    struct sockaddr_in clientAddr; // rename to master?
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(tcp_socket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        perror("accept");
        close(tcp_socket);
        return 1;
    }

    char taskBuffer[1024];
    int taskBytesReceived = recv(clientSocket, taskBuffer, sizeof(taskBuffer), 0);
    if (taskBytesReceived > 0) {
        taskBuffer[taskBytesReceived] = '\0';
        double start, end;
        sscanf(taskBuffer, "%lf %lf", &start, &end);
        double result = integrate(start, end);
        std::string resultStr = std::to_string(result);
        send(clientSocket, resultStr.c_str(), resultStr.size(), 0);
    }

    close(clientSocket);
    close(tcp_socket);

    return 0;
}
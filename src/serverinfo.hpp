#include "serverinfo.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// Отправка данных на конкретный мастер-сервер
void ServerInfo::SendServerInfoToMaster(const std::string &masterAddress) {
    char infoString[512];
    snprintf(infoString, sizeof(infoString),
             "\\infoRequest\\port\\%d\\gamedir\\%s\\map\\%s\\maxplayers\\%d\\players\\%d\\protocol\\%d\\",
             port, gameDir.c_str(), currentMap.c_str(), maxPlayers, numPlayers, PROTOCOL_VERSION);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    struct sockaddr_in masterAddr;
    memset(&masterAddr, 0, sizeof(masterAddr));
    masterAddr.sin_family = AF_INET;
    masterAddr.sin_port = htons(27010); // Порт мастер-сервера Steam
    inet_pton(AF_INET, masterAddress.c_str(), &masterAddr.sin_addr);

    sendto(sockfd, infoString, strlen(infoString), 0, (struct sockaddr *)&masterAddr, sizeof(masterAddr));
    close(sockfd);
}

// Регистрация на всех мастер-серверах
void ServerInfo::RegisterWithMasters() {
    const std::string masterServers[] = {
        "hl1master.steampowered.com",
        "127.0.0.1" // Локальный мастер-сервер (для тестирования)
    };

    for (const auto &master : masterServers) {
        SendServerInfoToMaster(master);
    }
}

// Обновление информации о сервере
void ServerInfo::UpdateServerInfo(const std::string &newMap, int newNumPlayers) {
    currentMap = newMap;
    numPlayers = newNumPlayers;
}

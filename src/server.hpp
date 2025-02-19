#ifndef __TINY_CSGO_SERVER_HPP__
#define __TINY_CSGO_SERVER_HPP__

#include "asio.hpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <optional>
#include "serverinfo.hpp" // Для информации о сервере

using asio::ip::udp;

// Определение протокола
#define PROTOCOL_VERSION 48

class GoldSourceServer {
public:
    GoldSourceServer(asio::io_context& io_context, uint16_t port, const std::string& redirect_ip, uint16_t redirect_port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)),
          redirect_ip_(redirect_ip),
          redirect_port_(redirect_port),
          serverInfo(port, "cstrike", "de_dust2", 32) {
        listen_for_packets();
        register_with_masters(); // Регистрация на мастер-серверах при старте
    }

private:
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    std::array<char, 4096> recv_buffer_;
    std::string redirect_ip_;
    uint16_t redirect_port_;
    ServerInfo serverInfo; // Информация о сервере
    std::vector<std::string> masterServers = {"hl1master.steampowered.com", "127.0.0.1"}; // Мастер-серверы

    // Регистрация на мастер-серверах
    void register_with_masters();

    // Асинхронный прием пакетов
    void listen_for_packets();

    // Обработка входящих пакетов
    void process_packet(std::size_t length);

    // Отправка A2S_INFO
    void send_a2s_info();

    // Отправка A2S_CHALLENGE
    void send_a2s_challenge();

    // Отправка редиректа
    void send_redirect();
};

#endif // __TINY_CSGO_SERVER_HPP__

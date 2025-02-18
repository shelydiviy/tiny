#ifndef __TINY_CSGO_SERVER_HPP__
#define __TINY_CSGO_SERVER_HPP__

#ifdef _WIN32
#pragma once
#endif

#include "asio.hpp"
#include "Steam3Server.h"
#include "GCClient.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <optional>

using asio::ip::udp;

class GoldSourceServer {
public:
    GoldSourceServer(asio::io_context& io_context, uint16_t port, const std::string& redirect_ip, uint16_t redirect_port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)), redirect_ip_(redirect_ip), redirect_port_(redirect_port) {
        listen_for_packets();
    }

private:
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    std::array<char, 4096> recv_buffer_;
    std::string redirect_ip_;
    uint16_t redirect_port_;

    void listen_for_packets() {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), sender_endpoint_,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    process_packet(bytes_recvd);
                }
                listen_for_packets();
            });
    }

    void process_packet(std::size_t length) {
        if (length < 5 || recv_buffer_[0] != '\xFF' || recv_buffer_[1] != '\xFF' || recv_buffer_[2] != '\xFF' || recv_buffer_[3] != '\xFF') {
            return; // Not a valid GoldSource query packet
        }

        std::string request(recv_buffer_.data() + 4, length - 4);

        if (request.starts_with("TSource Engine Query")) {
            send_a2s_info();
        } else if (request.starts_with("\x55")) {
            send_a2s_challenge();
        } else if (request.starts_with("\x57")) {
            send_redirect();
        }
    }

    void send_a2s_info() {
        std::vector<char> response = {
            '\xFF', '\xFF', '\xFF', '\xFF', '\x49',
            47, // Protocol version
        };

        // Server details
        const std::string server_name = "Redirect Server";
        const std::string map_name = "de_dust2";
        const std::string game_dir = "cstrike";
        const std::string game_desc = "Counter-Strike 1.6";
        uint16_t player_count = 0;
        uint16_t max_players = 32;
        uint16_t bot_count = 0;
        char server_type = 'd'; // Dedicated
        char environment = 'l'; // Linux
        char visibility = 0;    // Public
        char vac = 1;           // VAC secured

        // Add details to the response
        response.insert(response.end(), server_name.begin(), server_name.end());
        response.push_back('\0');
        response.insert(response.end(), map_name.begin(), map_name.end());
        response.push_back('\0');
        response.insert(response.end(), game_dir.begin(), game_dir.end());
        response.push_back('\0');
        response.insert(response.end(), game_desc.begin(), game_desc.end());
        response.push_back('\0');

        // Player and server info
        response.push_back(static_cast<char>(player_count));
        response.push_back(static_cast<char>(max_players));
        response.push_back(static_cast<char>(bot_count));
        response.push_back(server_type);
        response.push_back(environment);
        response.push_back(visibility);
        response.push_back(vac);

        // Additional data
        const std::string version = "47";
        response.insert(response.end(), version.begin(), version.end());
        response.push_back('\0');

        socket_.send_to(asio::buffer(response), sender_endpoint_);
    }

    void send_a2s_challenge() {
        std::vector<char> response = {'\xFF', '\xFF', '\xFF', '\xFF', '\x41'};

        // Add a random challenge number
        int32_t challenge = rand();
        response.push_back((challenge >> 24) & 0xFF);
        response.push_back((challenge >> 16) & 0xFF);
        response.push_back((challenge >> 8) & 0xFF);
        response.push_back(challenge & 0xFF);

        socket_.send_to(asio::buffer(response), sender_endpoint_);
    }

    void send_redirect() {
        std::string redirect_message = "\xFF\xFF\xFF\xFF\x30"; // Redirect header
        redirect_message += redirect_ip_ + ":" + std::to_string(redirect_port_);
        socket_.send_to(asio::buffer(redirect_message), sender_endpoint_);
    }
};

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            std::cerr << "Usage: <port> <redirect_ip> <redirect_port>\n";
            return 1;
        }

        uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));
        std::string redirect_ip = argv[2];
        uint16_t redirect_port = static_cast<uint16_t>(std::stoi(argv[3]));

        asio::io_context io_context;
        GoldSourceServer server(io_context, port, redirect_ip, redirect_port);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <iostream>
#include <atomic>
#include <sstream>
#include <vector>
#include <functional>
#include <rtc/rtc.hpp> 

class NetworkManager {
public:
    NetworkManager() : partnerX(0), partnerY(0), hasPartnerPos(false) {}

    bool Init(const std::string& serverUrl) {
        mWebSocket = std::make_shared<rtc::WebSocket>();

        mWebSocket->onOpen([serverUrl]() {
            std::cout << "[Network] Ligado com sucesso ao servidor Node.js em: " << serverUrl << std::endl;
        });

        mWebSocket->onError([](const std::string& error) {
            std::cout << "[Network] Erro na ligacao: " << error << std::endl;
        });

        mWebSocket->onMessage([this](auto data) {
            
            if (std::holds_alternative<std::string>(data)) {
                std::string msg = std::get<std::string>(data);
                std::stringstream ss(msg);
                std::string item;
                int x = 0, y = 0;
                
                if (std::getline(ss, item, ',')) x = std::stoi(item);
                if (std::getline(ss, item, ',')) y = std::stoi(item);

                partnerX = x; partnerY = y; hasPartnerPos = true;
            }
            else if (std::holds_alternative<std::vector<std::byte>>(data)) {
                auto bin = std::get<std::vector<std::byte>>(data);
                if (onAudioReceived) {
                    const uint8_t* raw = reinterpret_cast<const uint8_t*>(bin.data());
                    std::vector<uint8_t> audioData(raw, raw + bin.size());
                    
                    onAudioReceived(audioData);
                }
            }
        });

        std::cout << "[Network] A tentar ligar..." << std::endl;
        mWebSocket->open(serverUrl);
        return true;
    }

    void SendPosition(int x, int y) {
        if (mWebSocket && mWebSocket->isOpen()) {
            std::string msg = std::to_string(x) + "," + std::to_string(y);
            mWebSocket->send(msg);
        }
    }

    void SendAudio(const std::vector<uint8_t>& audioData) {
        if (mWebSocket && mWebSocket->isOpen() && !audioData.empty()) {
            mWebSocket->send(reinterpret_cast<const std::byte*>(audioData.data()), audioData.size());
        }
    }

    bool GetPartnerPosition(int& outX, int& outY) {
        if (hasPartnerPos) {
            outX = partnerX; outY = partnerY; return true;
        }
        return false;
    }

    void SetAudioReceivedCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
        onAudioReceived = callback;
    }

private:
    std::shared_ptr<rtc::WebSocket> mWebSocket;
    std::atomic<int> partnerX, partnerY;
    std::atomic<bool> hasPartnerPos;
    
    std::function<void(const std::vector<uint8_t>&)> onAudioReceived;
};

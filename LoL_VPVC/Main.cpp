#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cmath>

#include <rtc/rtc.hpp>

#include "NetworkManager.h"
#include "minimap_reader.h"
#include "AudioManager.h"

std::atomic<bool> isRunning(true);

double CalculateDistance(int x1, int y1, int x2, int y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "       LoL Proximity Voice Chat         " << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::string serverUrl;
    std::cout << "[Config] Cole o link do servidor Ngrok aqui (ou aperte ENTER para jogar local): ";
    std::getline(std::cin, serverUrl);

    if (serverUrl.empty()) {
        serverUrl = "ws://127.0.0.1:8080";
        std::cout << "[Config] Nenhum link inserido. Usando servidor local: " << serverUrl << std::endl;
    } else {
        if (serverUrl.find("https://") == 0) {
            serverUrl.replace(0, 8, "ws://");
        } else if (serverUrl.find("http://") == 0) {
            serverUrl.replace(0, 7, "ws://");
        } else if (serverUrl.find("ws://") != 0 && serverUrl.find("wss://") != 0) {
            serverUrl = "ws://" + serverUrl;
        }
        std::cout << "[Config] Iniciando conexao com: " << serverUrl << std::endl;
    }

    rtc::InitLogger(rtc::LogLevel::Error); 
    
    NetworkManager net;
    if (!net.Init(serverUrl)) {
        std::cout << "[Erro] Falha ao conectar no servidor! Verifique o link e tente novamente." << std::endl;
        std::cin.get();
        return 1;
    }

    AudioManager audio;
    if (!audio.Init()) {
        std::cout << "[Erro] Nao foi possivel iniciar o audio." << std::endl;
        std::cin.get();
        return 1;
    }

    audio.SetNetworkSendCallback([&net](const std::vector<uint8_t>& audioData) {
        net.SendAudio(audioData);
    });

    net.SetAudioReceivedCallback([&audio](const std::vector<uint8_t>& audioData) {
        audio.ReceiveAudio(audioData);
    });

    MinimapReader minimap;
    minimap.Init(1600, 800, 320, 320); 

    std::cout << "\n[Sistema] O radar esta ativo e furtivo!" << std::endl;
    std::cout << "[Sistema] Deixe esta janela aberta no fundo e bom jogo." << std::endl;

    while (isRunning) {
        minimap.Capture();
        MapPos myPos = minimap.FindCentroid(0);

        if (myPos.valid()) {
            net.SendPosition(myPos.x, myPos.y);

            int partnerX = 0, partnerY = 0;
            if (net.GetPartnerPosition(partnerX, partnerY)) {
                double distance = CalculateDistance(myPos.x, myPos.y, partnerX, partnerY);
                audio.SetVolumeFromDistance(distance);
            } else {
                audio.SetVolumeFromDistance(9999.0);
            }
        } else {
            audio.SetVolumeFromDistance(9999.0);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cmath>
#include <vector>
#include <sstream>
#include <cctype>
#include <windows.h>
#include <rtc/rtc.hpp>

#include "NetworkManager.h"
#include "minimap_reader.h"
#include "AudioManager.h"

std::atomic<bool> isRunning(true);
std::atomic<double> currentDistance(9999.0);
std::atomic<bool> isMuted(false);
std::atomic<bool> isDeafened(false);
std::atomic<float> userVolume(100.0f);
std::atomic<char> muteKeybind(0);
std::string globalServerUrl = "";

double CalculateDistance(int x1, int y1, int x2, int y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "       LoL Proximity Voice Chat         " << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::cout << "Link do servidor Ngrok ou IP Radmin: ";
    std::getline(std::cin, globalServerUrl);

    if (globalServerUrl.empty()) {
        globalServerUrl = "ws://127.0.0.1:8080";
    } else {
        if (globalServerUrl.find("https://") == 0) {
            globalServerUrl.replace(0, 8, "ws://");
        } else if (globalServerUrl.find("http://") == 0) {
            globalServerUrl.replace(0, 7, "ws://");
        } else if (globalServerUrl.find("ws://") != 0 && globalServerUrl.find("wss://") != 0) {
            globalServerUrl = "ws://" + globalServerUrl;
        }
    }

    rtc::InitLogger(rtc::LogLevel::Error); 
    
    NetworkManager net;
    if (!net.Init(globalServerUrl)) {
        std::cout << "Falha ao conectar no servidor!" << std::endl;
        std::cin.get();
        return 1;
    }

    AudioManager audio;
    if (!audio.Init()) {
        std::cout << "Nao foi possivel iniciar o audio." << std::endl;
        std::cin.get();
        return 1;
    }

    audio.SetNetworkSendCallback([&net](const std::vector<uint8_t>& audioData) {
        if (!isMuted.load() && !isDeafened.load()) {
            net.SendAudio(audioData);
        }
    });

    net.SetAudioReceivedCallback([&audio](const std::vector<uint8_t>& audioData) {
        if (isDeafened.load()) return;

        double dist = currentDistance.load();
        double maxStartFade = 25.0;
        double maxEndFade = 45.0;
        double fade = 1.0;

        if (dist > maxEndFade) {
            fade = 0.0;
        } else if (dist > maxStartFade) {
            fade = 1.0 - ((dist - maxStartFade) / (maxEndFade - maxStartFade));
        }

        float volMult = (userVolume.load() / 100.0f) * 2.0f;
        float finalMult = static_cast<float>(fade) * volMult;

        std::vector<uint8_t> modData = audioData;
        float* fData = reinterpret_cast<float*>(modData.data());
        size_t fCount = modData.size() / sizeof(float);
        
        for(size_t i = 0; i < fCount; i++) {
            fData[i] *= finalMult;
        }

        audio.ReceiveAudio(modData);
    });

    std::thread cmdThread([]() {
        std::string line;
        while (isRunning) {
            std::getline(std::cin, line);
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            
            if (cmd == "volume") {
                float v;
                if (iss >> v) {
                    if (v < 0.0f) v = 0.0f;
                    if (v > 100.0f) v = 100.0f;
                    userVolume.store(v);
                    std::cout << "Volume definido para " << v << std::endl;
                }
            } else if (cmd == "mute") {
                isMuted.store(!isMuted.load());
                if (isMuted.load()) std::cout << "Microfone silenciado com sucesso!" << std::endl;
                else std::cout << "Microfone ativado!" << std::endl;
            } else if (cmd == "muteall") {
                isDeafened.store(!isDeafened.load());
                if (isDeafened.load()) std::cout << "Audio e microfone silenciados com sucesso!" << std::endl;
                else std::cout << "Audio e microfone ativados!" << std::endl;
            } else if (cmd == "keybind") {
                std::string action;
                iss >> action;
                if (action == "mute") {
                    std::string key;
                    iss >> key;
                    if (key.length() > 0) {
                        muteKeybind.store(std::toupper(key[0]));
                        std::cout << "Keybind de mute definida para a tecla: " << static_cast<char>(muteKeybind.load()) << std::endl;
                    }
                }
            } else if (cmd == "ping") {
                std::string ip = globalServerUrl;
                if (ip.find("ws://") == 0) ip = ip.substr(5);
                if (ip.find("wss://") == 0) ip = ip.substr(6);
                size_t colonPos = ip.find(':');
                if (colonPos != std::string::npos) ip = ip.substr(0, colonPos);
                
                std::cout << "\nTestando latencia com o servidor (" << ip << ")...\n";
                std::string sysCmd = "ping -n 1 " + ip;
                std::system(sysCmd.c_str());
            } else if (cmd == "help") {
                std::cout << "\n--- Comandos Disponiveis ---" << std::endl;
                std::cout << "volume [0-100]       - Define o volume geral do programa." << std::endl;
                std::cout << "mute                 - Silencia ou ativa o seu microfone." << std::endl;
                std::cout << "muteall              - Silencia totalmente o microfone e o audio recebido." << std::endl;
                std::cout << "keybind mute [tecla] - Define um atalho no teclado para mutar/desmutar." << std::endl;
                std::cout << "ping                 - Mostra a latencia da rede em ms." << std::endl;
                std::cout << "help                 - Mostra esta lista de comandos.\n" << std::endl;
            }
        }
    });
    cmdThread.detach();

    std::thread keyThread([]() {
        bool wasPressed = false;
        while (isRunning) {
            char k = muteKeybind.load();
            if (k != 0) {
                short state = GetAsyncKeyState(k);
                bool isPressed = (state & 0x8000) != 0;
                
                if (isPressed && !wasPressed) {
                    isMuted.store(!isMuted.load());
                    if (isMuted.load()) std::cout << "\nMicrofone silenciado com sucesso!\n";
                    else std::cout << "\nMicrofone ativado!\n";
                }
                wasPressed = isPressed;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    keyThread.detach();

    MinimapReader minimap;
    minimap.Init(1600, 800, 320, 320); 

    std::cout << "\nPara uma lista de todos os comandos, insira help" << std::endl;
    std::cout << "O radar esta ativo e furtivo." << std::endl;

    while (isRunning) {
        minimap.Capture();
        MapPos myPos = minimap.FindCentroid(0);

        if (myPos.valid()) {
            net.SendPosition(myPos.x, myPos.y);

            int partnerX = 0, partnerY = 0;
            if (net.GetPartnerPosition(partnerX, partnerY)) {
                currentDistance.store(CalculateDistance(myPos.x, myPos.y, partnerX, partnerY));
                audio.SetVolumeFromDistance(0.0);
            } else {
                currentDistance.store(9999.0);
                audio.SetVolumeFromDistance(0.0);
            }
        } else {
            currentDistance.store(9999.0);
            audio.SetVolumeFromDistance(0.0);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}

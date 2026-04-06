#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include "miniaudio.h"

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool Init();
    void SetVolumeFromDistance(double distanceInPixels);
    void ReceiveAudio(const std::vector<uint8_t>& audioData);
    void SetNetworkSendCallback(std::function<void(const std::vector<uint8_t>&)> callback);

private:
    ma_device device;
    bool isInitialized = false;
    
    std::atomic<float> currentVolume; 
    
    std::vector<uint8_t> playbackBuffer;
    std::mutex audioMutex;

    std::function<void(const std::vector<uint8_t>&)> onAudioCaptured;

    static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

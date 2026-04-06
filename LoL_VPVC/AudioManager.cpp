#include "AudioManager.h"
#include <iostream>
#include <algorithm>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

AudioManager::AudioManager() : currentVolume(1.0f) {}

AudioManager::~AudioManager() {
    if (isInitialized) {
        ma_device_uninit(&device);
    }
}

void AudioManager::SetVolumeFromDistance(double distanceInPixels) {

    if (distanceInPixels <= 50.0) {
        currentVolume = 1.0f;
    } 
    else if (distanceInPixels >= 250.0) {
        currentVolume = 0.0f;
    } 
    else {
        currentVolume = 1.0f - static_cast<float>((distanceInPixels - 50.0) / 200.0);
    }
}

void AudioManager::SetNetworkSendCallback(std::function<void(const std::vector<uint8_t>&)> callback) {
    onAudioCaptured = callback;
}

void AudioManager::ReceiveAudio(const std::vector<uint8_t>& audioData) {
    std::lock_guard<std::mutex> lock(audioMutex);
    playbackBuffer.insert(playbackBuffer.end(), audioData.begin(), audioData.end());
}

void AudioManager::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AudioManager* audio = (AudioManager*)pDevice->pUserData;
    
    if (pInput != NULL && audio->onAudioCaptured) {
        size_t byteCount = frameCount * sizeof(float);
        const uint8_t* byteInput = static_cast<const uint8_t*>(pInput);
        
        std::vector<uint8_t> capturedBytes(byteInput, byteInput + byteCount);
        audio->onAudioCaptured(capturedBytes);
    }

    if (pOutput != NULL) {
        float* pOutputF32 = (float*)pOutput;
        size_t bytesNeeded = frameCount * sizeof(float);

        std::lock_guard<std::mutex> lock(audio->audioMutex);

        if (audio->playbackBuffer.size() >= bytesNeeded) {
            memcpy(pOutput, audio->playbackBuffer.data(), bytesNeeded);
            audio->playbackBuffer.erase(audio->playbackBuffer.begin(), audio->playbackBuffer.begin() + bytesNeeded);
            
            float vol = audio->currentVolume.load();
            for (ma_uint32 i = 0; i < frameCount; i++) {
                pOutputF32[i] *= vol;
            }
        } else {
            memset(pOutput, 0, bytesNeeded);
        }
    }
}

bool AudioManager::Init() {
    ma_device_config config = ma_device_config_init(ma_device_type_duplex);
    config.capture.pDeviceID = NULL;
    config.capture.format = ma_format_f32;
    config.capture.channels = 1;
    config.playback.pDeviceID = NULL;
    config.playback.format = ma_format_f32;
    config.playback.channels = 1; 
    config.sampleRate = 48000;
    config.dataCallback = data_callback;
    config.pUserData = this;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        std::cout << "[Erro] Falha ao inicializar a placa de som." << std::endl;
        return false;
    }

    ma_device_start(&device);
    std::cout << "[Audio] Microfone e Alto-falante ligados com sucesso!" << std::endl;
    isInitialized = true;
    return true;
}

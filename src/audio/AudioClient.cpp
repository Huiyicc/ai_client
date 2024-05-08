//
// Created by 19254 on 24-5-6.
//
#include <iomanip>
#include <fstream>
#include <thread>
#include "AudioClient.h"
#include "portaudio.h"
#include "stdexcept"
#include "extend/Logger.h"

#include "WavWriter.h"
#include "extend/Defer.h"

namespace AC {

// =========================
AudioDevices::AudioDevices(int index, const PaDeviceInfo *info) {
  ID = index;
  Name = info->name;
  HostApi = info->hostApi;
  InputChannelNum = info->maxInputChannels;
  OutputChannelNum = info->maxOutputChannels;
  DeviceInfo = info;
}

// =========================

AudioStream::AudioStream(void *stream,
                         void *userData,
                         PaStreamParameters p,
                         uint32_t sampleRate,
                         uint32_t framesPerBuffer,
                         const std::function<void(void *, const void *, unsigned long)> &callback) {
  m_stream = static_cast<PaStream *>(stream);
  m_userData = userData;
  m_callback = callback;
  m_params = std::make_shared<PaStreamParameters>();
  *m_params = p;
  m_sampleRate = sampleRate;
  m_framesPerBuffer = framesPerBuffer;
};

void AudioStream::Start() {
  auto call = [](const void *inputBuffer,
                 void *outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo *timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void *sysData) -> int {
    auto datas = static_cast<void **>(sysData);
    auto _this = static_cast<AudioStream *>(datas[0]);
    auto _user = datas[1];

    auto rprt = (const uint16_t *)inputBuffer;


    _this->m_pcmData.insert(_this->m_pcmData.end(), rprt, rprt + framesPerBuffer*_this->m_params->channelCount);

    if (_this->m_pcmData.size() >= 512000) {
      _this->SaveData("recorded_audio.wav");
      auto l = std::thread([_this](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        _this->Stop();
      });
      l.detach();
    }
    return 0;
  };
  m_datas[0] = this;
  m_datas[1] = m_userData;
  auto err = Pa_OpenStream(
    &m_stream,
    m_params.get(),
    nullptr,
    m_sampleRate,
    m_framesPerBuffer,
    paClipOff,
    call,
    &m_datas
  );
  if (err != paNoError) {
    std::string out = "open audio stream failed,";
    out.append(Pa_GetErrorText(err));
    PrintError(out);
    throw std::runtime_error(out);
  }
  err = Pa_StartStream(m_stream);
  if (err != paNoError) {
    Pa_AbortStream(m_stream);
    Pa_CloseStream(m_stream);
    std::string out = "start audio stream failed,";
    out.append(Pa_GetErrorText(err));
    PrintError(out);
    throw std::runtime_error(out);
  }
  m_isRunning = true;
  while (true) {
    if (!m_isRunning) {
      break;
    }
    Pa_Sleep(1000);
  }
}

void AudioStream::SaveData(const std::string &path, bool clear) {
  WavWriter ww;
  ww.initialize(path.c_str(),
                 m_sampleRate,
                 m_params->channelCount
                 , true, 2);
  ww.startWriting();
  ww.writeData((uint8_t*)m_pcmData.data(), m_pcmData.size()*sizeof(uint16_t));
  ww.finishWriting();
};

void AudioStream::Stop() {
  if (m_isRunning) {
    DEFER({m_isRunning = false;});
    auto err = Pa_StopStream(m_stream);
    if (err != paNoError) {
      std::string out = "stop audio stream failed,";
      out.append(Pa_GetErrorText(err));
      PrintError(out);
      throw std::runtime_error(out);
    }
    err = Pa_CloseStream(m_stream);
    if (err != paNoError) {
      std::string out = "close audio stream failed,";
      out.append(Pa_GetErrorText(err));
      PrintError(out);
      throw std::runtime_error(out);
    }

  }
}

// =========================

AudioClient *AudioClient::m_instance = nullptr;

AudioClient *AudioClient::GetInstance() {
  if (m_instance == nullptr) {
    m_instance = new AudioClient();
  }
  return m_instance;
}

AudioClient::~AudioClient() {
  // 关闭PortAudio
  Pa_Terminate();
}

AudioClient::AudioClient() {
  auto err = Pa_Initialize();
  if (err != paNoError) {
    std::string out = "init audio failed,";
    out.append(Pa_GetErrorText(err));
    PrintError(out);
    throw std::runtime_error(out);
  }
}

std::vector<AudioDevices>
AudioClient::GetAllDevices() {
  std::vector<AudioDevices> devices;
  // 获取设备数量
  auto deviceCount = Pa_GetDeviceCount();
  // 遍历设备列表
  for (int i = 0; i < deviceCount; ++i) {
    auto deviceInfo = Pa_GetDeviceInfo(i);
    if (deviceInfo) {
      devices.emplace_back(i, deviceInfo);
    }
  }
  return std::move(devices);
}

AudioDevices AudioClient::GetDefaultInputDevice() {
  auto defaultMicDevice = Pa_GetDefaultInputDevice();
  if (defaultMicDevice == paNoDevice) {
    PrintError("No default input device");
    throw std::runtime_error("No default input device");
  }
  auto deviceInfo = Pa_GetDeviceInfo(defaultMicDevice);
  return {defaultMicDevice, deviceInfo};
}

AudioStream AudioClient::OpenStream(const AudioDevices &inputDevice,
                                    uint32_t sampleRate,
                                    uint32_t framesPerBuffer,
                                    int channelCount,
                                    void *userData,
                                    const std::function<void(void *, const void *, unsigned long)> &callback) {
  PaStreamParameters inputParameters;
  inputParameters.device = inputDevice.ID;
  inputParameters.channelCount = channelCount;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency = inputDevice.DefaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = nullptr;
  return {nullptr, userData, inputParameters, sampleRate, framesPerBuffer, callback};
}

} // AC
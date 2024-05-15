//
// Created by 19254 on 24-5-6.
//
#include <fstream>
#include <thread>
#include "AudioClient.h"
#include "portaudio.h"
#include "stdexcept"
#include "extend/Logger.h"
#include "Algorithm/CircularBuffer.h"
#include "WavWriter.h"
#include "extend/Defer.h"
#include "extend/Timer.h"

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

AudioStream::AudioStream(int flitterSize,
                         void *stream,
                         void *userData,
                         PaStreamParameters p,
                         uint32_t sampleRate,
                         uint32_t framesPerBuffer,
                         const RecordCallback &callback) :
  m_energyBuffer(flitterSize), m_isRunning(false) {
  m_stream = static_cast<PaStream *>(stream);
  m_userData = userData;
  m_muteCallback = callback;
  m_params = std::make_shared<PaStreamParameters>();
  *m_params = p;
  m_sampleRate = sampleRate;
  m_framesPerBuffer = framesPerBuffer;
};

int32_t AudioStream::GetScanIndex() const {
  return m_scanIndex;
};

void AudioStream::SetScanIndex(int32_t i) {
  m_scanIndex = i;
};

int OnStreamCallBack(const void *inputBuffer,
                     void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void *sysData) {
  /**
   * @param datas 总数据
   * @param f 阈值
   * @param ft 容错分数,0-1,相当于datas.size()的百分比
   */
  auto flitterFunc = [](const std::vector<float> &datas, float f, float ft) -> bool {
    // 容错分数
    int c = datas.size() * ft;
    c = std::max(c, 0);
    // 判断是否超过阈值
    bool r = false;
    for (auto &i: datas) {
      if (i > f) {
        c--;
        if (c <= 0) {
          r = true;
          break;
        }
        continue;
      }
    }
    return r;
  };
  auto datas = static_cast<void **>(sysData);
  auto _this = static_cast<AudioStream *>(datas[0]);
  auto _user = datas[1];

  auto rprt = (const short *) inputBuffer;

  auto minPcm = std::numeric_limits<const short>::min();
  auto maxPcm = std::numeric_limits<const short>::max();
  auto middle = (maxPcm + minPcm) / 2;

  auto fNum = framesPerBuffer * _this->m_params->channelCount;
  float total = 0;
  float v = 0;
  for (size_t i = 0; i < fNum; ++i) {
    auto tt = rprt[i];
    if (rprt[i] > middle) {
      v = static_cast<float>(rprt[i]) / static_cast<float>(maxPcm);
    } else {
      v = (static_cast<float>(fabs(rprt[i])) / static_cast<float>(fabs(minPcm)));
    }
    total += v;
  }
  v = total / static_cast<float>(fNum);
  //PrintDebug("energy: {}", v);
  _this->m_energyBuffer.push(v);
  if (_this->m_energyBuffer.isFull()) {
    if (_this->m_records) {
      // 正在录音,滤波不通过取消录音
      if (!flitterFunc(_this->m_energyBuffer.getOrdered(),
                       _this->m_flitterSize,
                       _this->m_flitterFiltration)) {
        auto now = Timer::GetCurrentTimeMillis();
        if (now - _this->m_lastTime > _this->m_flitterTime) {
          bool r = true;
          if (_this->m_muteCallback) {
            r = _this->m_muteCallback(_this, _user);
          }
          _this->m_lastTime = now;
          // 防止爆闪
          _this->m_records = false;
          _this->m_insert = false;
        }
      }
    } else {
      // 没开始录音,超过阈值
      if (v > _this->m_flitterSize) {
        // 滤波,防止瞬时噪音
        if (flitterFunc(_this->m_energyBuffer.getOrdered(),
                        _this->m_flitterSize,
                        0.5)) {
          bool r = true;
          if (_this->m_StartCallback) {
            r = _this->m_StartCallback(_this, _user);
          }
          if (r) {
            _this->m_records = true;
            _this->m_insert = true;
          }
        }
      }else {
        // 因为过滤了短暂的爆音,但是这种过滤会导致丢掉说话开始时的一秒左右数据,所以要一个预存
        // 保留前reNum次采样的数据
        uint32_t reNum = 3;
        // 最大预存尺寸
        uint32_t maxIndex = reNum * fNum;
        if (_this->m_scanIndex >= maxIndex) {
          std::vector<short> tempV(maxIndex);
          tempV.clear();
          tempV.insert(tempV.end(), _this->m_pcmData.begin() + fNum, _this->m_pcmData.end());
          _this->m_scanIndex = tempV.size();
          _this->m_pcmData.clear();
          _this->m_pcmData.insert(_this->m_pcmData.end(), tempV.begin(), tempV.end());
          _this->m_insert = true;
        } else {
          _this->m_insert = true;
        }
      }
    }
  }
  if (_this->m_insert || _this->m_records) {
    _this->m_pcmData.insert(_this->m_pcmData.end(), rprt, rprt + framesPerBuffer * _this->m_params->channelCount);
    if (_this->m_pcmData.size() >= 10000) {
      _this->m_scanIndex += static_cast<int32_t>(framesPerBuffer * _this->m_params->channelCount);
      if (_this->m_pcmData.size() >= _this->m_maxSize) {
        _this->m_scanIndex = 0;
        _this->m_pcmData.clear();
      }
    }
  }

  return 0;
}

void AudioStream::Start(float flitterSize, float flitterFiltration, uint32_t flitterTime, uint64_t maxSize) {
  m_lastTime = Timer::GetCurrentTimeMillis();
  m_flitterSize = flitterSize;
  m_flitterFiltration = flitterFiltration;
  m_flitterTime = flitterTime;
  m_maxSize = maxSize;
  m_datas[0] = this;
  m_datas[1] = m_userData;
  auto err = Pa_OpenStream(
    &m_stream,
    m_params.get(),
    nullptr,
    m_sampleRate,
    m_framesPerBuffer,
    paClipOff,
    OnStreamCallBack,
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
}

void AudioStream::SaveData(const std::string &path, bool clear) {
  WavWriter ww;
  ww.initialize(path.c_str(),
                m_sampleRate,
                m_params->channelCount, true, 2);
  ww.startWriting();
  //ww.writeData((uint8_t *) m_pcmData.data(), m_pcmData.size() * sizeof(uint16_t));
  ww.writeDataFromInt16s(m_pcmData.data(), m_pcmData.size());
  ww.finishWriting();
  if (clear) {
    m_pcmData.clear();
    m_scanIndex = 0;  }
};

const std::vector<short> &AudioStream::GetPcmData() {
  return m_pcmData;
};

void AudioStream::Stop() {
  if (m_isRunning) {
    DEFER({ m_isRunning = false; });
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

void AudioStream::SetMuteCallback(const RecordCallback &callback) {
  m_muteCallback = callback;
};

void AudioStream::SetStartCallback(const RecordCallback &callback) {
  m_StartCallback = callback;
};

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
                                    void *userData,
                                    int flitterSize,
                                    uint32_t sampleRate,
                                    uint32_t framesPerBuffer,
                                    int channelCount,
                                    const RecordCallback &callback) {
  PaStreamParameters inputParameters;
  inputParameters.device = inputDevice.ID;
  inputParameters.channelCount = channelCount;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency = inputDevice.DefaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = nullptr;
  return {flitterSize, nullptr, userData, inputParameters, sampleRate, framesPerBuffer, callback};
}

} // AC
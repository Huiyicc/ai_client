//
// Created by 19254 on 24-5-6.
//

#ifndef AUDIO_CLIENT_AUDIOCLIENT_H
#define AUDIO_CLIENT_AUDIOCLIENT_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>
#include "Algorithm/CircularBuffer.h"

struct PaDeviceInfo;
struct PaStreamCallbackTimeInfo;
struct PaStreamParameters;
typedef unsigned long PaStreamCallbackFlags;


namespace AC {
class AudioStream;

typedef std::function<void(AudioStream* t,void* userData)> RecordCallback;

/**
 * 结构体AudioDevices用于描述音频设备的信息。
 *
 * @member ID 设备唯一标识符，默认值为0。
 * @member Name 设备名称，为空字符串默认。
 * @member HostApi 设备所属的宿主API编号，默认值为0。
 * @member InputChannelNum 设备的输入通道数量，默认值为0。
 * @member OutputChannelNum 设备的输出通道数量，默认值为0。
 * @member DefaultLowInputLatency 默认的低输入延迟，默认值为0。
 * @member DefaultLowOutputLatency 默认的低输出延迟，默认值为0。
 * @member DefaultHighInputLatency 默认的高输入延迟，默认值为0。
 * @member DefaultHighOutputLatency 默认的高输出延迟，默认值为0。
 * @member DeviceInfo 指向PaDeviceInfo结构的指针，包含了音频设备的详细信息。
 */
struct AudioDevices {
  int ID = 0;
  std::string Name;
  uint32_t HostApi = 0;
  uint32_t InputChannelNum = 0;
  uint32_t OutputChannelNum = 0;
  double DefaultLowInputLatency{};
  double DefaultLowOutputLatency{};
  double DefaultHighInputLatency{};
  double DefaultHighOutputLatency{};
  const PaDeviceInfo *DeviceInfo = nullptr;

  AudioDevices() = default;

  /**
   * 带参数的构造函数，用于根据设备索引和设备信息指针初始化音频设备对象。
   *
   * @param index 设备索引
   * @param deviceInfo 指向PaDeviceInfo结构的指针，包含了音频设备的详细信息
   */
  AudioDevices(int index, const PaDeviceInfo *);

  explicit operator std::string() const {
    return Name;
  }

  explicit operator const PaDeviceInfo *() const {
    return DeviceInfo;
  }
};

class AudioClient {
public:

  /**
   * 获取AudioClient的单例实例。
   *
   * 本函数为静态成员函数，用于获取AudioClient类的单例实例。在整个应用程序中，
   * 只能存在一个AudioClient实例，通过此函数来确保全局唯一性。
   *
   * @return 返回AudioClient类的单例实例指针。如果实例还未创建，则先创建新实例再返回。
   */
  static AudioClient *GetInstance();

  /**
   * 获取所有音频设备的列表。
   *
   * 这个函数不接受任何参数，它会查询系统中所有的音频设备，并返回一个包含这些设备的向量。
   *
   * @return std::vector<AudioDevices> 返回一个包含所有音频设备的向量。
   */
  static std::vector<AudioDevices>
  GetAllDevices();

  /**
   * 获取默认输入设备。
   *
   * 本函数用于查询系统中的默认输入音频设备。在音频应用中，常常需要知道哪个设备被系统设置为默认输入设备，以便于直接使用该设备进行音频输入操作。
   *
   * @return AudioDevices 返回一个包含默认输入设备信息的结构体。
   */
  static AudioDevices GetDefaultInputDevice();

  AudioStream OpenStream(const AudioDevices &inputDevice,
                         int flitterSize=30,
                         uint32_t sampleRate = 44100,
                         uint32_t framesPerBuffer = 512,
                         int channelCount = 1,
                         void *userData = nullptr,
                         const RecordCallback &callback = nullptr);

private:
  AudioClient();

  ~AudioClient();

  static AudioClient *m_instance;

};

class AudioStream {
public:

  /**
 * @brief 启动音频录制功能，根据设置的参数动态管理录制状态。
 *
 * 当音量超过`flitterSize`（最大值为1）时开始录制，低于该阈值时停止录制。
 * `flitterFiltration`表示滤波器的灵敏度，值越大意味着对音量变化的响应越灵敏。
 * 如果在`flitterTime`毫秒内未检测到声音，会自动停止录制。
 *
 * @param flitterSize 音量阈值，超过此值时开始录制，范围为[0, 1]，默认为0.02。
 * @param flitterFiltration 滤波灵敏度，数值越大越灵敏，默认为0.05。
 * @param flitterTime 没有声音后停止录制的等待时间，单位为毫秒，默认为2500。
 */
  void Start(float flitterSize=0.02,
             float flitterFiltration=0.05,
             uint32_t flitterTime=2500
    );

  void Stop();

  void SetCallback(const RecordCallback &callback);

  void SaveData(const std::string &path, bool clear = true);

  [[nodiscard]] int32_t GetScanIndex() const;

  void SetScanIndex(int32_t i);

  [[nodiscard]] const std::vector<short>& GetPcmData();

private:
  bool m_insert = false;
  uint64_t m_lastTime = 0;
  uint32_t m_flitterTime = 2500;
  bool m_records = false;
  void *m_stream = nullptr;
  void *m_userData = nullptr;
  uint32_t m_sampleRate = 0; // 采样率
  uint32_t m_framesPerBuffer = 0; // 每帧采样数
  float m_flitterSize=0.02;
  float m_flitterFiltration=0.05;
  bool m_isRunning = false;
  std::shared_ptr<PaStreamParameters> m_params;
  RecordCallback m_callback;

  AudioStream(int flitterSize,
              void *stream,
              void *userData,
              PaStreamParameters p,
              uint32_t sampleRate,
              uint32_t framesPerBuffer,
              const RecordCallback &callback);

  friend class AudioClient;

  void *m_datas[2] = {nullptr};
  std::vector<short> m_pcmData;
  int32_t m_scanIndex = 0;
  Algorithm::CircularBuffer<float> m_energyBuffer;

  friend
  int OnStreamCallBack(const void *inputBuffer,
                       void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *sysData);
};

} // AC

#endif //AUDIO_CLIENT_AUDIOCLIENT_H

# 代码来源

[kleydon/WavUtils](https://github.com/kleydon/WavUtils)

# AC::WavUtils
C++中用于读写Wav文件的类。
- 支持逐帧（增量）或一次性读写
- 支持8/16/24/32位整型样本，或32/64位浮点型样本
- 在读写过程中转换至/来自16位整型格式
- 内存中转换至/来自16位整型格式
- 采用小端字节序；支持iOS、Android NDK、Windows和OSX（Post-PowerPC）

# 用法
## Read:
```cpp
#include "WavReader.hpp"
auto wr = AC::WavReader();
wr->initialize(inputWavFilePath);
wr->prepareToRead();  // 元数据在此之后可用
wr->readData(sampleData, wr->getSampleDataSize());
wr->finishReading();
```

## Write:
```cpp
#include "WavWriter.hpp"
...
auto wr = AC::WavReader();
ww->initialize(outputWavFilePath,
               sampleRate,
               numChannels,  // 1 || 2 ,通道数
               samplesAreInts, // false为float采样, true为int采样
               byteDepth);  // Int采样为1(8)/2(16)/3(24)/4(32); float采样为4(32)/8(64),实际上这是位宽度.
ww->startWriting();
ww->writeData(sampleData, sampleDataSize);
ww->finishReading();
```

## 增量读写:
```cpp
// read
wr->readData(bufferA, bufferASize);
wr->readData(bufferB, bufferBSize);

// write
ww->writeData(bufferA, bufferASize);
ww->writeData(bufferB, bufferBSize);
```

## 读写期间转换至/来自Int16样本:
```cpp
// read
wr->readDataToInt16s(int16Samples, numInt16Samples);

// write
ww->writeDataFromInt16s(int16Samples, numInt16Samples);
```

## 内存中转换至/来自Int16样本:
```cpp
// 从Int16样本数组中读取样本
wr->readInt16SampleFromArray(sampleData,  // Wav格式的源样本数据数组
                             sampleDataSize,
                             sampleIndex,  // 要读取的样本索引
                             int16SampleCh1,  // 样本的通道1值，作为int16
                             int16SampleCh2);  // 若存在，样本的通道2值，作为int16
                             
// 将样本写入Int16样本数组
ww->writeInt16SampleToArray(int16SampleCh1,  // 样本的通道1值，作为int16
                            int16SampleCh2,  // 若存在，样本的通道2值，作为int16
                            sampleIndex,  // 要写入的样本索引
                            sampleData,  // Wav格式的目标样本数据数组
                            sampleDataSize);
```

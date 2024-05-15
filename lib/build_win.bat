@echo off
for /f "tokens=2 delims==" %%a in ('wmic cpu get NumberOfCores /value') do set CPU_CORES=%%a
mkdir build
cd  build
mkdir portaudio
cd portaudio
cmake -G Ninja -DCMAKE_INSTALL_PREFIX=%CD%/../../ -DPA_USE_DS=0 -DPA_USE_WDMKS=0 -DPA_USE_WDMKS_DEVICE_INFO=0 -DPA_USE_WMME=0  -DCMAKE_BUILD_TYPE=Release %CD%/../../portaudio
echo Enable %CPU_CORES%-thread compilation
make -j%CPU_CORES%&& make install

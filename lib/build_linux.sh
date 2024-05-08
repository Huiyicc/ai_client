#!/bin/bash
core_count=$(grep -c ^processor /proc/cpuinfo)

mkdir build
cd  build
mkdir portaudio
cd portaudio
$(pwd)/../../portaudio/configure --prefix=$(pwd)/../../
# cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../../  -DCMAKE_BUILD_TYPE=Release $(pwd)/../../portaudio
echo Enable $core_count -thread compilation
make -j$core_count && make install

#!/usr/bin/bash

sudo apt update && sudo apt upgrade -y
sudo apt install -y python3 python3-pip gcc-14 g++-14 ninja-build gdb
pip3 install conan==1.65 cmake clang-format --break-system-packages

conan config init
conan profile update settings.compiler.libcxx=libstdc++11 default
conan profile update settings.compiler.version=14 default
conan profile update settings.build_type=Debug default


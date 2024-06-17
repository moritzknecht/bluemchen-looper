brew install gcc-arm-embedded
git clone https://github.com/moritzknecht/bluemchen-looper
cd bluemchen-looper
git submodule update --init --recursive
./build-libs.sh
make
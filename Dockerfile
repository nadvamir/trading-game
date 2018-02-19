from ubuntu:xenial

# install build deps
run apt-get update && apt-get upgrade -y && \
   apt-get install -y build-essential software-properties-common cmake libboost-all-dev && \
   add-apt-repository ppa:ubuntu-toolchain-r/test && apt-get update && \
   apt-get install -y gcc-7 g++-7 &&  \
   update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 60 && \
   update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60

# n.b. any changes to the files and directories mentioned in the below copies
#      will trigger a rebuild of the container from that point onward

# source dirs
copy bandit/            /source/bandit/
copy crow/              /source/crow/
copy examples/          /source/examples/
copy exchange/          /source/exchange/
copy templates/         /source/templates/
copy test/              /source/test/
copy fxbattle/          /source/fxbattle/

# source files
copy CMakeLists.txt Makefile /source/

# build
run mkdir /build; cd /build; cmake /source; cmake --build .

# run
cmd cd /build; ./FXBattle.exe /config/config.json /config/traded_pairs.json /config/traders.json

from rikorose/gcc-cmake:gcc-7

run apt-get update && apt-get upgrade -y && \
   apt-get install -y libboost-all-dev

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
copy statsd-c-client/   /source/statsd-c-client/

# source files
copy CMakeLists.txt Makefile /source/

# build
run mkdir /build; cd /build; cmake /source; cmake --build .

# run
cmd cd /build; ./FXBattle.exe /config/config.json /config/traded_pairs.json /config/traders.json

CC=g++
INCLUDES=-I. -Iexchange

TEST_SOURCES  = test/exchange_main.test.cpp
TEST_SOURCES += test/exchange_currency.test.cpp
TEST_SOURCES += test/exchange_market.test.cpp
TEST_SOURCES += test/exchange_arbitragedestroyer.test.cpp
TEST_SOURCES += test/exchange_fxconversions.test.cpp
TEST_SOURCES += test/exchange_fxconverter.test.cpp

all: test

.PHONY: test exchange

exchange: test

test: build
	cd build && make
	./build/Test.exe --reporter=spec

build:
	mkdir build
	cd build && cmake ..

clean:
	rm -rf build

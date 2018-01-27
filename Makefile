CC=g++
INCLUDES=-I. -Iexchange

TEST_SOURCES  = test/exchange_currency.test.cpp
TEST_SOURCES += test/exchange_main.test.cpp

all: test

.PHONY: test exchange

exchange: test

test: build
	$(CC) $(INCLUDES) $(TEST_SOURCES) -o build/exchange.test.exe
	build/exchange.test.exe

build:
	mkdir build

clean:
	rm -rf build

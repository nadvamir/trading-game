CC=g++

all: test

.PHONY: test exchange

exchange: test

test: build
	$(CC) -I. -Iexchange test/exchange_currency.test.cpp -o build/exchange.test.exe
	build/exchange.test.exe

build:
	mkdir build

clean:
	rm -rf build

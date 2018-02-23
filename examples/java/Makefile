all: build run

.PHONY: build

build:
	gradle build shadow

run:
	java -jar build/libs/java-1.0-all.jar http://127.0.0.1:8080 api_key1


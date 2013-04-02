all: 
	mkdir -p build
	cd build; cmake .. ; make

debug:
	mkdir -p build
	cd build; cmake -DDEBUG=1 .. ; make

clean:
	rm -r build
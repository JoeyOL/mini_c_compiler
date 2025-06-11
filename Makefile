scanner: src/main.cpp src/scanner/scanner.cpp src/scanner/scanner.h
	g++ -o scanner src/main.cpp src/scanner/scanner.cpp  -I./src
clean:
	rm -f scanner
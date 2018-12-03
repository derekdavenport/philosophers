all: philosophers 

philosophers : main.cpp
	g++ -o philosophers main.cpp -Wno-deprecated

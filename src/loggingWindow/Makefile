
all: main.cc mainWindow.cc testTCP.cc
	g++ -std=c++11 -o logWindow main.cc mainWindow.cc -I. `pkg-config gtkmm-3.0 --cflags --libs`
	g++ -std=c++11 -o testTCP testTCP.cc

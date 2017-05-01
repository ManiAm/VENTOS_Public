
all: mainWin testMainWin



# link command for mainWin
mainWin: main.o mainWindow.o 
	g++ -o mainWindow main.o mainWindow.o -I. -lpthread -lboost_system -lboost_filesystem `pkg-config gtkmm-3.0 --libs`

# compile
main.o : main.cc mainWindow.h
	g++ -std=c++11 -O2 -c -o main.o main.cc -I. `pkg-config gtkmm-3.0 --cflags`

mainWindow.o : mainWindow.cc mainWindow.h debugStream.h
	g++ -std=c++11 -O2 -c -o mainWindow.o mainWindow.cc -I. `pkg-config gtkmm-3.0 --cflags`



# link command for mainwin
testMainWin: testTCP.o
	g++ -o testTCP testTCP.o
	
# compile
testTCP.o : testTCP.cc sockets.h
	g++ -std=c++11 -c -o testTCP.o testTCP.cc
	

msgheaders:
smheaders:


clean:
	-rm -rf main.o
	-rm -rf mainWindow.o
	-rm -rf mainWindow
	-rm -rf testTCP.o
	-rm -rf testTCP


all: eebl_rx eebl_tx testEthernet
	
eebl_rx: eebl_rx.o
	gcc -std=c99 -o eebl_rx eebl_rx.o ../libs/lib_rsi_wave_api.a ../libs/libdsrc.a -lpthread -I ../headers -I ../headers/DSRC_J2735/ -I ../headers/rsi_wave_api/

eebl_tx: eebl_tx.o
	gcc -std=c99 -o eebl_tx eebl_tx.o ../libs/lib_rsi_wave_api.a ../libs/libdsrc.a -lpthread -I ../headers -I ../headers/DSRC_J2735/ -I ../headers/rsi_wave_api/
	
testEthernet: testEthernet.o
	gcc -std=c99 -o testEthernet testEthernet.o -lpthread
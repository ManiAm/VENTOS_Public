
all: eebl_rx eebl_tx testEthernet
	
eebl_rx: eebl_rx.c
	gcc -std=c99 eebl_rx.c -o eebl_rx ../libs/lib_rsi_wave_api.a ../libs/libdsrc.a -lpthread -I ../headers -I ../headers/DSRC_J2735/ -I ../headers/rsi_wave_api/

eebl_tx: eebl_tx.c
	gcc -std=c99 eebl_tx.c -o eebl_tx ../libs/lib_rsi_wave_api.a ../libs/libdsrc.a -lpthread -I ../headers -I ../headers/DSRC_J2735/ -I ../headers/rsi_wave_api/
	
testEthernet: testEthernet.c
	gcc -std=c99 testEthernet.c -o testEthernet -lpthread
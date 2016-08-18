# default C dialect for gcc is -std=gnu90
# https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html
# http://stackoverflow.com/questions/14737104/what-is-the-default-c-mode-for-the-current-gcc-especially-on-ubuntu

LIBS = ../libs/lib_rsi_wave_api.a ../libs/libdsrc.a ../libs/libventos.a
HEADERS = -I ../headers -I ../headers/SAE_J2735 -I ../headers/SAE_J2735_messages -I ../headers/rsi_wave_api

# SAE J2735 'Basic Safety Message (BSM)' over WSM
rsi_bsm_rx: rsi_bsm_rx.c
	gcc -std=c99 -Wall rsi_bsm_rx.c -o rsi_bsm_rx $(LIBS) $(HEADERS) -lpthread 
rsi_bsm_tx: rsi_bsm_tx.c
	gcc -std=c99 -Wall rsi_bsm_tx.c -o rsi_bsm_tx $(LIBS) $(HEADERS) -lpthread

# SAE J2735 MAP data over WSM
rsi_map_rx: rsi_map_rx.c
	gcc -std=c99 -Wall rsi_map_rx.c -o rsi_map_rx $(LIBS) $(HEADERS) -lpthread
rsi_map_tx: rsi_map_tx.c
	gcc -std=c99 -Wall rsi_map_tx.c -o rsi_map_tx $(LIBS) $(HEADERS) -lpthread

# SAE J2735 'Road Side Alert (RSA)' over WSM
rsi_rsa_rx: rsi_rsa_rx.c
	gcc -std=c99 -Wall rsi_rsa_rx.c -o rsi_rsa_rx $(LIBS) $(HEADERS) -lpthread
rsi_rsa_tx: rsi_rsa_tx.c
	gcc -std=c99 -Wall rsi_rsa_tx.c -o rsi_rsa_tx $(LIBS) $(HEADERS) -lpthread

# SAE J2735 'Signal Phase And Timing (SPAT)' over WSM
rsi_spat_rx: rsi_spat_rx.c
	gcc -std=c99 -Wall rsi_spat_rx.c -o rsi_spat_rx $(LIBS) $(HEADERS) -lpthread
rsi_spat_tx: rsi_spat_tx.c
	gcc -std=c99 -Wall rsi_spat_tx.c -o rsi_spat_tx $(LIBS) $(HEADERS) -lpthread
	
# SAE J2735 'Traveler Information Message (TIM)' over WSM
rsi_tim_rx: rsi_tim_rx.c
	gcc -std=c99 -Wall rsi_tim_rx.c -o rsi_tim_rx $(LIBS) $(HEADERS) -lpthread
rsi_tim_tx: rsi_tim_tx.c
	gcc -std=c99 -Wall rsi_tim_tx.c -o rsi_tim_tx $(LIBS) $(HEADERS) -lpthread

# ----------------------------------------------------------------------------------

# 'Emergency Electronic Brake Light (EEBL)' using BSM
eebl_rx: eebl_rx.c
	gcc -std=c99 -Wall eebl_rx.c -o eebl_rx $(LIBS) $(HEADERS) -lpthread
eebl_tx: eebl_tx.c
	gcc -std=c99 -Wall eebl_tx.c -o eebl_tx $(LIBS) $(HEADERS) -lpthread

# EEBL for VENTOS
emit_eebl: emit_eebl.c
	gcc -std=c99 -Wall emit_eebl.c -o emit_eebl $(LIBS) $(HEADERS) -lpthread
	
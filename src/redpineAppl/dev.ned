
package VENTOS.src.redpineAppl;

simple dev
{
    parameters:
        @class(VENTOS::dev);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        
        string host = default("localhost"); 
        int port = default(22);
        string username = default("ubuntu");
        string password = default("Redpine"); 
        
        // Driver Mode:
        // 1  END-TO-END mode, 
        // 2  RF Evaluation Mode
        int DRIVER_MODE = default(1);
        
        // COEX Mode:                  
        // 1  WLAN STATION /WIFI-Direct/WLAN PER
        // 2  WLAN ACCESS POINT(including muliple APs on different vaps)
        // 3  WLAN ACCESS POINT + STATION MODE(on multiple vaps)
        // 4  BT CLASSIC MODE/BT CLASSIC PER MODE
        // 5  WLAN STATION + BT CLASSIC MODE
        // 6  WLAN ACCESS POINT + BT CLASSIC MODE
        // 8  BT LE MODE /BT LE PER MODE
        // 9  WLAN STATION + BT LE MODE
        // 12 BT CLASSIC + BT LE MODE 							
        // 14 WLAN ACCESS POINT + BT CLASSIC MODE+ BT LE MODE
        // 16 ZIGBEE MODE/ ZIGBEE PER MODE
        // 17 WLAN STATION + ZIGBEE
        int COEX_MODE = default(1);
        
        // To enable TA-level SDIO aggregation set 1 else set 0 to disable it.
        int TA_AGGR = default(4);
        
        // Disable Firmware load set 1 to skip FW loading through Driver else set to 0.
        int SKIP_FW_LOAD = default(0);
        
        // FW Download Mode:	
        // 1  Full Flash mode with Secondary Boot Loader
        // 2  Full RAM mode with Secondary Boot Loader
        // 3  Flash + RAM mode with Secondary Boot Loader
        // 4  Firmware loading WITHOUT Secondary Boot Loader
        // Recommended to use the default mode=1
        int FW_LOAD_MODE = default(1);
        
        // ps_handshake Mode:
        // 1  No hand shake Mode
        // 2  Packet hand shake Mode
        // 3  GPIO Hand shake Mode
        // Default is Packet handshake mode=2
        int HANDSHAKE_MODE = default(2);
        
        // SDIO Clock speed
        int SDIO_CLOCK_SPEED = default(50000);
        
        // Antenna diversity enable
        int RSI_ANTENNA_DIVERSITY = default(0);
        
        // Antenna Selection
        // 2 Internal Antenna Selection
        // 3 External Antenna Selection
        int ANT_SEL_VAL = default(2);         
        
        // RF_POWER_MODE Selection
        // 0x00 For Both TX and RX High Power 
        // 0x11 For Both TX and RX Medium Power
        // 0x22 For Both TX and RX LOW Power
        
        // 0x10 For High Power TX and Medium RX Power
        // 0x20 For High Power TX and LOW RX Power
        
        // 0x01 For Medium TX and RX High Power
        // 0x21 For Medium Power TX and LOW RX Power
        
        // 0x02 For Low Power TX and RX High Power 
        // 0x12 For LOW Power TX and Medium RX Power
        int WLAN_RF_PWR_MODE = default(0x00);
        int BT_RF_PWR_MODE = default(0x00);
        int ZIGB_RF_PWR_MODE = default(0x00);
        
        // COUNTRY Selection
        // 0   World Domain
        // 840 US Domain Maps to US Region
        // 276 Germany Maps to EU Region
        // 392 Japan Maps to Japan Region
        int SET_COUNTRY_CODE = default(840); 
}


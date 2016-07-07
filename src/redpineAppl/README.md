Redpine Board
=============

In order to prepare a redpine's board for communication with VENTOS, you need to follow these steps:

**Step 1:** The default static IPv4 address assigned to each board is 192.168.60.42. In order to connect more than two boards to the same LAN you need to assing a unique IP address to each board. Change the IP address of a board by editing file:

    /etc/network/interfaces
    
Then restart networking using

    sudo service networking restart

If that gives you trouble reboot the board.

**Step 2:** Drivers should be located in /home/release and you need to change its owner from root to ubuntu.

    sudo chown ubuntu /home/release
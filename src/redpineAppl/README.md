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

**Step 3:** Make sure to install these packages:

    sudo apt-get install libgmp3-dev

Tip: In order to connect the board to the Internet, you need to add a http proxy for apt-get. Open the following file

    /etc/apt/apt.conf

and append these:

    Acquire::http::Proxy "http://192.168.60.30:3128/";
    Acquire::https::Proxy "http://192.168.60.30:3128/";

Make sure that an http proxy server like `squid` is running on your local machine. squid server can be configured using the conf file at

    /etc/squid3/squid.conf

Go to `TAG: acl section` and add the following lines:

    acl board1 src 192.168.60.42
    acl board2 src 192.168.60.43

Go to `TAG: http_access` and add the following lines:

    http_access allow board1
    http_access allow board2

Go to `TAG: http_port` and change http_port value to 3128

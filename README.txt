Pcap Touch Server 
by Kara Martens

Released under the GNU GPL v3
https://www.gnu.org/licenses/gpl.html

INSTALL
-------
To install the application run 
	make && make install
The applictaion is now installed at /usr/bin/pcapserver

USAGE
-----
pcapserver [-f pcap filter] [-i interface]
  pcap filter  A filter using the libpcap format,
               filters with spaces should be in quotes.
  interface    The name of the network interface to capture on.

UNINSTALL
---------
make uninstall 
or
rm /usr/bin/pcapserver
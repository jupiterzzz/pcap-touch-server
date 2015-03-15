## Pcap Touch Server ##

The Pcap Touch Server is a companion to the Pcap Touch iOS application. It will capture network traffic and stream it to the iOS device so it can be viewed in real time.

Get the [iOS application](http://pcaptouch.net)

Requirements:
  * Pcap Touch iOS application
  * Linux computer with root privalages

Dependancies:
  * libpcap-dev

### Install ###
To install the application run
> `make && make install`
The applictaion is now installed at /usr/bin/pcapserver

### Usage ###
```
pcapserver [-f pcap filter] [-i interface]
  pcap filter  A filter using the libpcap format,
               filters with spaces should be in quotes.
  interface    The name of the network interface to capture on.
```
### Uninstall ###
`make uninstall`
or
`rm /usr/bin/pcapserver`
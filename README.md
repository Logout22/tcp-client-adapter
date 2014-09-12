tcpbridge
=========

A small tool to connect two clients without an intermediate server,
e.g. a passive VNC server to an ordinary VNC client instead of a
listening viewer.

------------     ---------------------------------     ------------
||        ||     ||        -------------        ||     ||        ||
|| Client || --> || Server |   <--->   | Server || <-- || Client ||
||        || <-- || socket | TCPBRIDGE | socket || --> ||        ||
------------     ||        |           |        ||     ------------
                 ||        -------------        ||
                 ---------------------------------

For instance, I like to use KRDC, which is a feature-rich
VNC client with the ability to scale the server's resolution.
However, it provides no mode for a listening VNC viewer, so I made
this little tool to fill the gap.

It is written in C and so far designed for UNIX systems only.

TODO add reference to installation notes

Feel free to contact me: You can find my e-mail address in the
commit history (e.g. execute git log).


XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
export LC_ALL=C
xinit ./xinitrc -- "$XEPHYR" :1 -ac -screen 800x600 -host-cursor
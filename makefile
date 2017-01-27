
all:
	gcc -D_DEFAULT_SOURCE -std=c99 fuck_systemd.c -c -o fuck_systemd.o
	gcc -shared -fpic -D_DEFAULT_SOURCE -std=c99 fuck_systemd.c -o fuck_systemd.so

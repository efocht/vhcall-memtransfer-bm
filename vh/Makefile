.PHONY: all
SRC=vhcalltestlib.c
OBJ=$(SRC:.c=.o)
CFLAGS=-fpic -I$(WORKSPACE)/opt/nec/ve/veos/include
all: libvhcalltestlib.so

libvhcalltestlib.so: $(OBJ)
	$(CC) -shared -o $@ $(OBJ)
vhcalltestlib.o: vhcalltestlib.c
	$(CC) -c $(CFLAGS) vhcalltestlib.c -o $@

.PHONY: clean
clean:
	rm -f *.so *.o

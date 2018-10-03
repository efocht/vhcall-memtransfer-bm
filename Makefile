CC=/opt/nec/ve/bin/ncc

.PHONY: all
all : vh ve2vh vh2ve

ve2vh : ve2vh.o
	$(CC) -o ve2vh -fopenmp $< -lsysve

ve2vh.o : ve2vh.c
	$(CC) -o ve2vh.o -O2 -fopenmp -c $<

vh2ve : vh2ve.o
	$(CC) -o vh2ve -fopenmp $< -lsysve

vh2ve.o : vh2ve.c
	$(CC) -o vh2ve.o -O2 -fopenmp -c $<

.PHONY: clean vh
vh:
	make -C vh

clean :
	rm -f *.o ve2vh vh2ve

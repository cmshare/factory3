LIBDLL =  hybServKit.a 
LIBSRC =  hybServKit.c cmbase.c
LIBINC =  cmbase.h
LIBOBJ =  hybServKit.o cmbase.o

.PHONY:clean

$(LIBDLL): $(LIBOBJ)
	rm -rf $@ 
	ar -r $@ $^
%.o: %.c $(LIBINC)
	$(CC) -c $< -o $@ -g -Wall

clean: 
	rm -f   $(LIBOBJ)  $(LIBDLL)

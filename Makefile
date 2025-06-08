CC = cc
CFLAGS = -Isrc
LDFLAGS =
LIBS =

EXEC =

OBJS = src/append.o src/bitio.o src/crcio.o src/dhuf.o src/extract.o src/header.o src/huf.o src/indicator.o src/larc.o src/lhadd.o src/lharc.o src/lhext.o src/lhlist.o src/maketbl.o src/maketree.o src/patmatch.o src/shuf.o src/slide.o src/util.o src/getopt_long.o src/pm2.o src/pm2hist.o src/pm2tree.o src/support_utf8.o src/fnmatch.o

.PHONY: all clean
.SUFFIXES: .c .o

all: lha$(EXEC)

lha$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f lha lha.exe src/*.o

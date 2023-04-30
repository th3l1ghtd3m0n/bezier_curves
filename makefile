PKGS=sdl2 gl
CFLAGS=-Wall -Wextra -std=c11 -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs ${PKGS}` -lm

all: cpu gpu

cpu: cpu.c 
	cc $(CFLAGS) -o cpu cpu.c $(LIBS)

gpu: gpu.c 
	cc $(CFLAGS) -o gpu gpu.c $(LIBS)

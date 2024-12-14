.PHONY: all
all: amx.so
amx.so: amx.c amx.h
	clang -O3 -shared -o amx.so -fPIC amx.c

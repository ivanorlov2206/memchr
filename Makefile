all:
	riscv64-unknown-linux-gnu-as -march=rv64gc_zbb -o memchr.o memchr.S
	riscv64-unknown-linux-gnu-gcc -O3 memchr2.c memchr.o -o memchr
clean:
	rm -rf *.o

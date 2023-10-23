all:
	riscv64-unknown-linux-gnu-as -march=rv64gc -o memchr.o memchr_wb.S
	riscv64-unknown-linux-gnu-gcc -O3 memchr2.c memchr.o -o memchr
clean:
	rm -rf *.o

obj-m += ubuntu-hid-01.o ubuntu-hid-02.o

all:
	iasl dsdt01.asl dsdt02.asl
	make -C /lib/modules/`uname -r`/build M=`pwd` modules
clean:
	rm *.aml
	make -C /lib/modules/`uname -r`/build M=`pwd` clean

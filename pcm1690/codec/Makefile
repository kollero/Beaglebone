
obj-m := pcm1690-codec.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo cp pcm1690-codec.ko /lib/modules/$(shell uname -r)
	sudo depmod -a
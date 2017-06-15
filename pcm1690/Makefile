
#sudo apt-get install build-essential gcc-arm-linux-gnueabi git lzop u-boot-tools
#sudo apt-get install linux-headers-$(uname -r)
#sudo apt-get install linux-headers-3.8.0-44-generic

# BCM2835 Platform Support
#snd-soc-bcm2835-i2s-objs := bcm2835-i2s.o
#obj-$(CONFIG_SND_BCM2835_SOC_I2S) += snd-soc-bcm2835-i2s.o

#snd-soc-pcm1690-objs := pcm1690.o
#obj-$(CONFIG_SND_SOC_PCM1690)	+= snd-soc-pcm1690.o


obj-m := pcm1690.o
#KDIR := /lib/modules/$(shell uname -r)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo cp pcm1690.ko /lib/modules/$(shell uname -r)
	sudo depmod -a
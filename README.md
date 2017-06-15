# Beaglebone
pcm1690 drivers uses beaglebone internal 24.576MHz clock
pcm1690 only uses currently default audio format and will force it to correct TDM mode, as dmesg will tell

make sure to run:

sudo apt-get update
sudo apt-get install linux-headers-$(uname -r)

to install modules, ssh to beaglebone and:
git clone xx


cd pcm1690
make all install
cd driver
make all install
cd ..

dtc -O dtb -o pcm1690-overlay-00A0.dtbo -b 0 -@ pcm1690-overlay-00A0.dts
sudo cp pcm1690-overlay-00A0.dtbo /lib/firmware

edit: /boot/uEnv.txt

enable_uboot_overlays=1
dtb_overlay=/lib/firmware/pcm1690-overlay-00A0.dtbo

disable universal cape and hdmi audio

reboot and check if it works with:
speaker-test -t sine -f 10000 -c 1 -r 48000



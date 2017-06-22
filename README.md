# Beaglebone
pcm1690 drivers uses beaglebones internal 24MHz clock
pcm1690 only supports 24bit audio format in TDM mode

make sure to run:

sudo apt-get update
sudo apt-get install linux-headers-$(uname -r)

to install modules, ssh to beaglebone and:
git clone https://github.com/kollero/Beaglebone

cd Beaglebone
cd pcm1690
make all install
cd codec
make all install
cd ..

dtc -O dtb -o pcm1690-overlay-00A0.dtbo -b 0 -@ pcm1690-overlay-00A0.dts
sudo cp pcm1690-overlay-00A0.dtbo /lib/firmware

edit: /boot/uEnv.txt

enable_uboot_overlays=1
dtb_overlay=/lib/firmware/pcm1690-overlay-00A0.dtbo

disable universal cape and hdmi audio

to check if it really works cannot use speaker-test since it doesn't support 24bit...

//reboot and check if it works with:
speaker-test -t sine -f 10000 -c 1 -r 48000



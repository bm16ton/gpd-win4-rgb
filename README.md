First compile the module;

make

Then detech usbhid from the the third interface (could be done with udev rule)

sudo echo 1-3.2:1.2 | sudo tee /sys/bus/usb/drivers/usbhid/unbind

then load the driver

sudo insmod led-class.ko

Now in /sys/class/leds you will see 3 new entries, simply echo a value between 0-255 to the appropriate brightness file, IE for blue (as root)

echo 255 > usb-1-3.2:w:gpd-blue/brightness

TODO see what steps can be removed from the usb write/reads to make faster, and add keyboard remapping

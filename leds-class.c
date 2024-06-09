#include <linux/init.h>// macros used to markup functions e.g. __init, __exit
#include <linux/module.h>// Core header for loading LKMs into the kernel
#include <linux/kernel.h>// Contains types, macros, functions for the kernel
#include <linux/device.h>// Header to support the kernel Driver Model

#include <linux/usb.h> //for usb stuffs
#include <linux/slab.h> //for

#include <linux/workqueue.h> //for work_struct
#include <linux/leds.h> //for led

//one structure for each connected device
struct my_usb
{
   struct usb_device *udev;
   struct work_struct work;
   struct led_classdev led;
   struct led_classdev led2;
   struct led_classdev led3;
   struct urb		*int_in_urb;		/* the urb to read data with */
   unsigned char           *int_in_buf;	/* the buffer to receive data */
   struct usb_endpoint_descriptor *int_in_endpoint;
   uint8_t msgdata[66];
   uint8_t tmpsend[34];
   int disconnect;
   int dir;
};

#define MY_USB_VENDOR_ID 0x2F24
#define MY_USB_PRODUCT_ID 0x0135
static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(0x2f24, 0x0135) },
       { }
};
MODULE_DEVICE_TABLE(usb, my_usb_table);
static uint8_t usbdata[18][33] = {{0x01,0xa5,0x10,0x5a,0xef,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x10,0x5a,0xef,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x11,0x5a,0xee,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x11,0x5a,0xee,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x11,0x5a,0xee,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x11,0x5a,0xee,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x12,0x5a,0xed,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x20,0x5a,0xdf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x00,0x00,0xe8,0x00,0xe9,0x00,0x4a,0x00,0x4d,0x00,0x51,0x00,0x7d,0x00,0x7c,0x00,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x01,0x00,0x1a,0x00,0x16,0x00,0x04,0x00,0x07,0x00,0x2c,0x00,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x02,0x00,0x00,0x00,0xea,0x00,0xeb,0x00,0xec,0x00,0xed,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x03,0x00,0x00,0x00,0x46,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x04,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2c,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x2c,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x21,0x5a,0xde,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x22,0x5a,0xdd,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x01,0xa5,0x23,0x5a,0xdc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};


static void
usb_gpd_start_work(struct work_struct *work)
{
   struct my_usb *sd = container_of(work, struct my_usb, work);
	int retval;
	if (sd->disconnect == 1) {
		return;
	}

	if (sd->dir == 1) {
   		  retval = usb_control_msg(sd->udev,
                   usb_sndctrlpipe(sd->udev, 0),
                   0x009, 0x21,
                   0x0301, 2,
                   sd->tmpsend, 33,
                   1000);

	if(retval < 0) {
		printk("usb_gpd_start_work\n");
	}

	} else if (sd->dir == 0) {
		int retval;

		retval = usb_control_msg_recv(sd->udev,
				      0,
				      0x01,
				      0xa1,
				      0x0101,
				      2,
				      sd->msgdata, 65,
				      5000, GFP_KERNEL);

	if(retval < 0) {
		printk("fuck the read report request fucked itself!\n");
	}
  }
}

static void presend(struct my_usb *data) {
	int i;
	int j;

	if (data->disconnect == 1) {
		return;
	}

	for (j = 0; j <= 17; j++) { 
    	for (i = 0; i < 33; i++) { 
        	data->tmpsend[i] = usbdata[j][i]; 
    	} 
        data->dir = 1;
		schedule_work(&data->work);
		msleep(100);

		if ((j == 0) || (j == 1) || (j == 2) || (j == 3) || (j == 4) || (j == 5) || (j == 6) || (j == 7)) {

			data->dir = 0;
			schedule_work(&data->work);
//			msleep(100);
		}
	}
}

//blue  first XX
static void
usb_blue_set(struct led_classdev *led,
                enum led_brightness brightness)
{
   struct my_usb *data = container_of(led, struct my_usb, led);

	if (data->disconnect == 1) {
		return;
	}

   usbdata[12][13] = brightness;
   presend(data);

}

//red 2nd xx
static void
usb_red_set(struct led_classdev *led2,
                enum led_brightness brightness)
{
   struct my_usb *data = container_of(led2, struct my_usb, led2);

	if (data->disconnect == 1) {
		return;
	}

   usbdata[12][14] = brightness;
   presend(data);

}

//green 3rd xx
static void
usb_green_set(struct led_classdev *led3,
                enum led_brightness brightness)
{
   struct my_usb *data = container_of(led3, struct my_usb, led3);

	if (data->disconnect == 1) {
		return;
	}

   usbdata[12][15] = brightness;
   presend(data);
}

static void
int_cb (struct urb *urb)
{
;
}

//called when a usb device is connected to PC
static int
my_usb_probe(struct usb_interface *interface,
             const struct usb_device_id *id)
{
   struct usb_device *udev = interface_to_usbdev(interface);
   struct usb_host_interface *iface_desc;
   struct usb_endpoint_descriptor *endpoint;
   struct my_usb *data;
   int i;

   printk(KERN_INFO "manufacturer: %s", udev->manufacturer);
   printk(KERN_INFO "product: %s", udev->product);

   iface_desc = interface->cur_altsetting;
   printk(KERN_INFO "vusb led %d probed: (%04X:%04X)",
          iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
   printk(KERN_INFO "bNumEndpoints: %d", iface_desc->desc.bNumEndpoints);

	int inf;
	inf = interface->cur_altsetting->desc.bInterfaceNumber;
	if (inf != 2) {
		dev_info(&interface->dev, "Ignoring Interface\n");
		return -ENODEV;
	}

   data = kzalloc(sizeof(struct my_usb), GFP_KERNEL);
   if (data == NULL)
     {
        //handle error
     }

   data->disconnect = 0;

   //increase ref count, make sure u call usb_put_dev() in disconnect()
   data->udev = usb_get_dev(udev);

   for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
     {
        endpoint = &iface_desc->endpoint[i].desc;

        printk(KERN_INFO "ED[%d]->bEndpointAddress: 0x%02X\n",
               i, endpoint->bEndpointAddress);
        printk(KERN_INFO "ED[%d]->bmAttributes: 0x%02X\n",
               i, endpoint->bmAttributes);
        printk(KERN_INFO "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n",
               i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);

		if (endpoint->bEndpointAddress == 0x83) {
			data->int_in_endpoint = endpoint;
			data->int_in_endpoint->bEndpointAddress = endpoint->bEndpointAddress;
			data->int_in_endpoint->bmAttributes = endpoint->bmAttributes;
		}
     }

	data->int_in_urb = usb_alloc_urb (0, GFP_KERNEL);
	data->int_in_buf = kmalloc (0x0021, GFP_KERNEL);
		usb_fill_int_urb (data->int_in_urb, data->udev,
					 usb_rcvintpipe (data->udev, 0x83),
					 data->int_in_buf,
					 0x0021,
					 int_cb, data,
					 (data->int_in_endpoint->
					 bInterval));
					    
   usb_set_intfdata(interface, data);

   printk(KERN_INFO "usb device is connected");

	int b;
	b = usb_submit_urb (data->int_in_urb, GFP_KERNEL);
	if (b)
		{
			printk (KERN_ALERT "Failed to submit urb \n");
		}

   data->led.name = kasprintf(GFP_KERNEL, "%s-%s:w:gpd-red",
                              dev_driver_string(&data->udev->dev),
                              dev_name(&data->udev->dev));
   data->led.brightness_set = usb_blue_set;
   data->led.brightness = 0;
   data->led.max_brightness = 255;

   data->led2.name = kasprintf(GFP_KERNEL, "%s-%s:w:gpd-green",
                              dev_driver_string(&data->udev->dev),
                              dev_name(&data->udev->dev));
   data->led2.brightness_set = usb_red_set;
   data->led2.brightness = 0;
   data->led2.max_brightness = 255;

   data->led3.name = kasprintf(GFP_KERNEL, "%s-%s:w:gpd-blue",
                              dev_driver_string(&data->udev->dev),
                              dev_name(&data->udev->dev));
   data->led3.brightness_set = usb_green_set;
   data->led3.brightness = 0;
   data->led3.max_brightness = 255;

   INIT_WORK(&data->work, usb_gpd_start_work);
   //register led device class
   led_classdev_register(&data->udev->dev, &data->led);
   led_classdev_register(&data->udev->dev, &data->led2);
   led_classdev_register(&data->udev->dev, &data->led3);

   return 0;
}

//called when unplugging a USB device
static void
my_usb_disconnect(struct usb_interface *interface)
{
   struct my_usb *data;

   data = usb_get_intfdata(interface);
   data->disconnect = 1;
   flush_work(&data->work);
   cancel_work_sync(&data->work);
   usb_kill_urb (data->int_in_urb);
   usb_free_urb (data->int_in_urb);
   
   led_classdev_unregister(&data->led);
   led_classdev_unregister(&data->led2);
   led_classdev_unregister(&data->led3);

   usb_set_intfdata(interface, NULL);

   //deref the count
   usb_put_dev(data->udev);

   kfree(data); //deallocate, allocated by kzmalloc()

   printk(KERN_INFO "led usb device is disconnected");
}

static struct usb_driver my_usb_driver = {
     .name = "gpd-rgb-led",
     .id_table = my_usb_table,
     .probe = my_usb_probe,
     .disconnect = my_usb_disconnect,
};


module_usb_driver(my_usb_driver);

MODULE_AUTHOR("Ben Maddocks <bm16ton@gmail.com>");
MODULE_DESCRIPTION("GPD win 4 led driver v1.0");
MODULE_LICENSE("GPL");

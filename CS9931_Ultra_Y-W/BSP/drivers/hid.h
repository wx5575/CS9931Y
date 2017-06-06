/* ********************************************************************************************************************* */
/* 常用USB结构和相关常量 */

typedef struct _USB_SETUP_REQ {
	UINT8	bType;
	UINT8	bReq;
	UINT8	wValueL;
	UINT8	wValueH;
	UINT8	wIndexL;
	UINT8	wIndexH;
	UINT8	wLengthL;
	UINT8	wLengthH;
} USB_SETUP_REQ, *PUSB_SETUP_REQ;

typedef struct _USB_DEVICE_DESCRIPTOR {
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bcdUSBL;
    UINT8 bcdUSBH;
    UINT8 bDeviceClass;
    UINT8 bDeviceSubClass;
    UINT8 bDeviceProtocol;
    UINT8 bMaxPacketSize0;
    UINT8 idVendorL;
    UINT8 idVendorH;
    UINT8 idProductL;
    UINT8 idProductH;
    UINT8 bcdDeviceL;
    UINT8 bcdDeviceH;
    UINT8 iManufacturer;
    UINT8 iProduct;
    UINT8 iSerialNumber;
    UINT8 bNumConfigurations;
} USB_DEV_DESCR, *PUSB_DEV_DESCR;

typedef struct _USB_CONFIG_DESCRIPTOR {
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 wTotalLengthL;
    UINT8 wTotalLengthH;
    UINT8 bNumInterfaces;
    UINT8 bConfigurationValue;
    UINT8 iConfiguration;
    UINT8 bmAttributes;
    UINT8 MaxPower;
} USB_CFG_DESCR, *PUSB_CFG_DESCR;

typedef struct _USB_INTERF_DESCRIPTOR {
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bInterfaceNumber;		//设备的接口号
    UINT8 bAlternateSetting;
    UINT8 bNumEndpoints;
    UINT8 bInterfaceClass;
    UINT8 bInterfaceSubClass;
    UINT8 bInterfaceProtocol;	 //设备类型：0x01 键盘，0x02 鼠标
    UINT8 iInterface;
} USB_ITF_DESCR, *PUSB_ITF_DESCR;

typedef struct _USB_ENDPOINT_DESCRIPTOR {
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bEndpointAddress;		 //设备端点地址
    UINT8 bmAttributes;
    UINT8 wMaxPacketSize;			 //设备端点大小
    UINT8 wMaxPacketSize1;
    UINT8 bInterval;
} USB_ENDP_DESCR, *PUSB_ENDP_DESCR;

typedef struct _USB_CONFIG_DESCRIPTOR_LONG {
	USB_CFG_DESCR	cfg_descr;
	USB_ITF_DESCR	itf_descr;
	USB_ENDP_DESCR	endp_descr[2];
} USB_CFG_DESCR_LONG, *PUSB_CFG_DESCR_LONG;


typedef struct _USB_HID_CLASS_DESCRIPTOR{
    UINT8  bLength;
	UINT8  bDescriptorType;
	UINT16 bcdHID;
	UINT8  bCountryCode;
	UINT8  bNumDescriptors;
	UINT8  bDescriptType;
	UINT8 wDescriptorLength;	 //报表长度
}USB_HID_CLASS_DESCR,*PUSB_HID_CLASS_DESCR;



typedef struct _HID_DEVICE{
   	USB_CFG_DESCR        hid_cfg_descr;
	USB_ITF_DESCR        hid_itf_descr;
	USB_HID_CLASS_DESCR	 hid_class_descr;
	USB_ENDP_DESCR       endp_descr;
}HID_DEVICE,*PHID_DEVICE;


typedef struct _USB_HID_COMPOSITE_DEVICE1{
	USB_CFG_DESCR        hid_cfg_descr;
	USB_ITF_DESCR        hid_itf_descr1;
	USB_HID_CLASS_DESCR	 hid_class_descr1;
	USB_ENDP_DESCR       endp_descr1;
	USB_ITF_DESCR        hid_itf_descr2;
	USB_HID_CLASS_DESCR	 hid_class_descr2;
	USB_ENDP_DESCR       endp_descr2;
}HID_COMPOSITE_DEVICE1,*PHID_COMPOSITE_DEVICE1;

typedef struct _USB_HID_COMPOSITE_DEVICE2{
   USB_CFG_DESCR        hid_cfg_descr;
   USB_ITF_DESCR        hid_itf_descr1; 
   USB_HID_CLASS_DESCR	hid_class_descr1;
   USB_ENDP_DESCR       endp_descr1;
   USB_ITF_DESCR        hid_itf_descr1_1; 
   USB_ENDP_DESCR       endp_descr1_1;

   USB_ITF_DESCR        hid_itf_descr2; 
   USB_HID_CLASS_DESCR	hid_class_descr2;
   USB_ENDP_DESCR       endp_descr2;
   USB_ITF_DESCR        hid_itf_descr2_1; 
   USB_ENDP_DESCR       endp_descr2_1;

}HID_COMPOSITE_DEVICE2,*PHID_COMPOSITE_DEVICE2;

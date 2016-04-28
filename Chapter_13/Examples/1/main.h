static const char *topology_str =
/* T:  Bus=dd Lev=dd Prnt=dd Port=dd Cnt=dd Dev#=ddd Spd=dddd MxCh=dd */
"\nT:  Bus=%2.2d Lev=%2.2d Prnt=%2.2d Port=%2.2d Cnt=%2.2d Dev#=%3d Spd=%-4s MxCh=%2d\n";

static const char *format_bandwidth =
/* B:  Alloc=ddd/ddd us (xx%), #Int=ddd, #Iso=ddd */
  "B:  Alloc=%3d/%3d us (%2d%%), #Int=%3d, #Iso=%3d\n";

static const char *format_device1 =
/* D:  Ver=xx.xx Cls=xx(sssss) Sub=xx Prot=xx MxPS=dd #Cfgs=dd */
  "D:  Ver=%2x.%02x Cls=%02x(%-5s) Sub=%02x Prot=%02x MxPS=%2d #Cfgs=%3d\n";

static const char *format_device2 =
/* P:  Vendor=xxxx ProdID=xxxx Rev=xx.xx */
  "P:  Vendor=%04x ProdID=%04x Rev=%2x.%02x\n";

static const char *format_string_manufacturer =
/* S:  Manufacturer=xxxx */
  "S:  Manufacturer=%.100s\n";

static const char *format_string_product =
/* S:  Product=xxxx */
  "S:  Product=%.100s\n";

struct class_info {
        int class;
        char *class_name;
};

static const struct class_info clas_info[] = {
        /* max. 5 chars. per name string */
        {USB_CLASS_PER_INTERFACE,       ">ifc"},
        {USB_CLASS_AUDIO,               "audio"},
        {USB_CLASS_COMM,                "comm."},
        {USB_CLASS_HID,                 "HID"},
        {USB_CLASS_PHYSICAL,            "PID"},
        {USB_CLASS_STILL_IMAGE,         "still"},
        {USB_CLASS_PRINTER,             "print"},
        {USB_CLASS_MASS_STORAGE,        "stor."},
        {USB_CLASS_HUB,                 "hub"},
        {USB_CLASS_CDC_DATA,            "data"},
        {USB_CLASS_CSCID,               "scard"},
        {USB_CLASS_CONTENT_SEC,         "c-sec"},
        {USB_CLASS_VIDEO,               "video"},
        {USB_CLASS_WIRELESS_CONTROLLER, "wlcon"},
        {USB_CLASS_MISC,                "misc"},
        {USB_CLASS_APP_SPEC,            "app."},
        {USB_CLASS_VENDOR_SPEC,         "vend."},
        {-1,                            "unk."}         /* leave as last */
};

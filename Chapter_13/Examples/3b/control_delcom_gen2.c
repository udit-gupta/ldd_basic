// http://www.delcomproducts.com/downloads/control_delcom_gen2.zip
// Requires libhid (http://libhid.alioth.debian.org/)
#include <hid.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main (argc, argv)
    int argc;
    char **argv; {

    // First, grab the user's options.
    static int blue_flag;
    static int get_flag;
    static int green_flag;
    static int help_flag;
    static int off_flag;
    static int red_flag;

    int c;
    while (1) {
        static struct option long_options[] = {
            // These options set a flag.
            {"blue",  no_argument,  &blue_flag,  1},
            {"get",   no_argument,  &get_flag,   1},
            {"green", no_argument,  &green_flag, 1},
            {"help",  no_argument,  &help_flag,  1},
            {"off",   no_argument,  &off_flag,   1},
            {"red",   no_argument,  &red_flag,   1},
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
        }
    }

    // Print usage if the user didn't supply a valid option or asked for help.
    if (
        (! blue_flag && ! red_flag && ! green_flag && ! off_flag && ! get_flag)
        || help_flag
    ) {
        printf(
            "Usage: %s [ --blue | --red | --green | --off | --get ]\n",
            argv[0]
        );

        exit(1);
    }

    // Set up the libhid interface.
    HIDInterface* hid;
    int iface_num = 0;
    hid_return ret;

    // These are for a delcom gen 2 usb light. YMMV.
    unsigned short vendor_id  = 0x0fc5;
    unsigned short product_id = 0xb080;

    HIDInterfaceMatcher matcher = { vendor_id, product_id, NULL, NULL, 0 };

    ret = hid_init();
    if (ret != HID_RET_SUCCESS) {
        fprintf(stderr, "hid_init failed with return code %x\n", ret);
        return 1;
    }

    hid = hid_new_HIDInterface();
    if (hid == 0) {
        fprintf(stderr, "hid_new_HIDInterface() failed, out of memory?\n");
        return 1;
    }

    ret = hid_force_open(hid, iface_num, &matcher, 3);
    if (ret != HID_RET_SUCCESS) {
        fprintf(stderr, "hid_force_open failed with return code %d\n", ret);
        return 1;
    }

    // Path to the LEDs
    int path[2];
    path[0] = 0xff000000;
    path[1] = 0x00000000;

    // Set up the packet
    char* buf = malloc(8);
    int i;
    for (i = 0; i < 8; i++) {
        buf[i] = 0;
    }

    if (get_flag) {
        // We are reading the feature to find out the current status.
        buf[0] = 100;
        hid_get_feature_report(hid, path, 1, buf, 8);

        if (ret != HID_RET_SUCCESS) {
           fprintf(stderr, "hid_get_feature_report failed()\n");
           return 1;
        }

        // Note, this assumes you only have one set of LEDs on.
        if (buf[2] == 0xFFFFFFFE) {
            printf("green");
        }
        else if (buf[2] == 0xFFFFFFFD) {
            printf("red");
        }
        else if (buf[2] == 0xFFFFFFFB) {
            printf("blue");
        }
        else {
            printf("off");
        }
    }
    else {
        // We are doing an 8 byte write feature to set the active LED.
        buf[0] = 101;
        buf[1] = 12;
        // buf[2] is the LSB, buf[3] is the MSB
        if (green_flag) {
            buf[2] = 0x01<<0;
            buf[3] = 0xFF;
        }
        if (red_flag) {
            buf[2] = 0x01<<1;
            buf[3] = 0xFF;
        }
        if (blue_flag) {
            buf[2] = 0x01<<2;
            buf[3] = 0xFF;
        }
        if (off_flag) {
            buf[3] = 0xFF;
        }

        hid_set_feature_report(hid, path, 1, buf, 8);
        if (ret != HID_RET_SUCCESS) {
           fprintf(stderr, "hid_set_feature_report failed()\n");
           return 1;
        }
    }
}

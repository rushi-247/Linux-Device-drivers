struct pcdev_platform_data{
	int size;
	int perm;
	const char *serial_number;
};

#define RDWR 0x11
#define RD 0x01
#define WR 0x10

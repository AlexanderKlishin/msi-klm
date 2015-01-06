
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>

#include <hidapi/hidapi.h>

#define VENDOR_ID 0x1770
#define PRODUCT_ID 0xff00

enum light_areas {
	AREA_LEFT   = 0x01,
	AREA_MIDDLE = 0x02,
	AREA_RIGHT  = 0x03
};

enum light_colors {
	COLOR_BLACK = 0x00,
	COLOR_RED = 0x01,
	COLOR_ORANGE = 0x02,
	COLOR_YELLOW = 0x03,
	COLOR_GREEN = 0x04,
	COLOR_SKY = 0x05,
	COLOR_BLUE = 0x06,
	COLOR_PURPLE = 0x07,
	COLOR_WHITE = 0x08
};

enum light_modes {
	MODE_DISABLE = 0x00,
	MODE_NORMAL = 0x01,
	MODE_GAMING = 0x02,
	MODE_BREATHE = 0x03,
	MODE_DEMO = 0x04,
	MODE_WAVE = 0x05,
	MODE_DUAL_COLOR  = 0x06
};

enum commands {
	CMD_COMMIT     = 0x41,
	CMD_COLOR      = 0x42,
	CMD_COLOR_SPEC = 0x43
};

const struct name2color {
	const char *name;
	enum light_colors light_color;
} n2c[] = {
	{"black", COLOR_BLACK },
	{"red", COLOR_RED },
	{"orange", COLOR_ORANGE },
	{"yellow", COLOR_YELLOW },
	{"green", COLOR_GREEN },
	{"sky", COLOR_SKY },
	{"blue", COLOR_BLUE },
	{"purple", COLOR_PURPLE },
	{"white", COLOR_WHITE }
};
const size_t n2c_size = sizeof(n2c)/sizeof(n2c[0]);

enum light_colors get_light_color(const char *name)
{
	int i;

	for (i = 0; i < n2c_size; ++i)
		if (strcmp(name, n2c[i].name) == 0)
			return n2c[i].light_color;

	fprintf(stderr, "unknown color %s, set to %s\n", name, n2c[0].name);
	return n2c[0].light_color;
}

const struct name2mode {
	const char *name;
	enum light_modes light_mode;
} n2m[] = {
	{"disable", MODE_DISABLE },
	{"normal",  MODE_NORMAL },
	{"gaming",  MODE_GAMING },
	{"breathe", MODE_BREATHE },
	{"demo",    MODE_DEMO },
	{"wave",    MODE_WAVE },
	{"dual",    MODE_DUAL_COLOR },
};
const size_t n2m_size = sizeof(n2m)/sizeof(n2m[0]);

enum light_modes get_light_mode(const char *name)
{
	int i;

	for (i = 0; i < n2m_size; ++i)
		if (strcmp(name, n2m[i].name) == 0)
			return n2m[i].light_mode;

	fprintf(stderr, "unknown mode %s, set to %s\n", name, n2m[0].name);
	return n2m[0].light_mode;
}

#define HID_CHECK_ERROR(x,s) \
	if(x == -1) { \
		fprintf(stderr, "error %s\n", s); \
		return 1; \
	}

#define CMD_SIZE 8

int area(hid_device *handle, enum commands cmd,
	enum light_areas area, enum light_colors color,
	unsigned char level, unsigned char blue)
{
	int err;
	unsigned char data[CMD_SIZE] = {
		0x01, /* Fixed report value */
		0x02, /* Fixed report value */
		cmd,
		area,
		color,
		level,
		blue, /* blue component gain speed for special modes */
		0xec }; /* EOR */

	err = hid_send_feature_report(handle, data, CMD_SIZE + 1);
	HID_CHECK_ERROR(err, "send");

	return 0;
}

int commit(hid_device *handle, enum light_modes mode)
{
	int err;
	unsigned char data[CMD_SIZE] = {
		0x01, 0x02,
		CMD_COMMIT,
		mode,
		0x00, 0x00, 0x00, 0xec /* EOR */ };

	err = hid_send_feature_report(handle, data, CMD_SIZE + 1);
	HID_CHECK_ERROR(err, "send");

	return 0;
}

void usage()
{
	int i;

	printf("msi-klm -t mode [-c color] "
	       "[-l color] [-m color] [-r color] "
	       "[-p sec] [-e level]");

	printf("\n\tmodes:");
	for (i = 0; i < n2m_size; ++i)
		printf(" %s", n2m[i].name);

	printf("\n\tcolors:");
	for (i = 0; i < n2c_size; ++i)
		printf(" %s", n2c[i].name);

	printf("\nexamples:"
		"\n\tturn off:   msi-klm -t disable"
		"\n\tturn off 2: msi-klm -t normal -l black -m black -r black"
		"\n\tnormal:     msi-klm -t normal -l red -m green -r sky"
		"\n\tred wave:   msi-klm -t wave -c red"
		"\n");
}

int main(int argc, char **argv)
{
	int err;
	hid_device *handle;
	enum light_modes light_mode = MODE_NORMAL;
	enum light_colors
		color_left = COLOR_RED, color_left2 = COLOR_BLACK,
		color_middle = COLOR_GREEN, color_middle2 = COLOR_BLACK,
		color_right = COLOR_BLUE, color_right2 = COLOR_BLACK;
	unsigned char speed;
	unsigned char level = 0x01;
	unsigned period_sec = 1;
	int opt;

	while ((opt = getopt(argc, argv, "ht:c:C:l:L:m:M:r:R:p:e:")) != -1) {
		switch (opt) {
			case 't': light_mode = get_light_mode(optarg); break;
			case 'c':
				color_left = color_middle = color_right = get_light_color(optarg);
				break;
			case 'C':
				color_left2 = color_middle2 = color_right2 = get_light_color(optarg);
				break;
			case 'l': color_left = get_light_color(optarg); break;
			case 'L': color_left2 = get_light_color(optarg); break;
			case 'm': color_middle = get_light_color(optarg); break;
			case 'M': color_middle2 = get_light_color(optarg); break;
			case 'r': color_right = get_light_color(optarg); break;
			case 'R': color_right2 = get_light_color(optarg); break;
			case 'p': period_sec = atoi(optarg); break;
			case 'e': level = atoi(optarg); break;
			case 'h':
				usage();
				return 0;
			default:
				break;
		}
	}

	err = hid_init();
	HID_CHECK_ERROR(err, "init");

	handle = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
	if (!handle) {
		fprintf(stderr, "cannot open");
		return 1;
	}

	switch (light_mode) {
	case MODE_DISABLE:
		break;

	case MODE_NORMAL:
		area(handle, CMD_COLOR, AREA_LEFT, color_left, level, 0x00);
		area(handle, CMD_COLOR, AREA_MIDDLE, color_middle, level, 0x00);
		area(handle, CMD_COLOR, AREA_RIGHT, color_right, level, 0x00);
		break;

	case MODE_GAMING:
		area(handle, CMD_COLOR, AREA_LEFT, color_left, level, 0x00);
		break;

	case MODE_BREATHE:
	case MODE_WAVE:
	case MODE_DUAL_COLOR:
		speed = period_sec;
		area(handle, CMD_COLOR_SPEC, AREA_LEFT, color_left, level, 0x00);
		area(handle, CMD_COLOR_SPEC, AREA_MIDDLE, color_left2, level, 0x00);
		area(handle, CMD_COLOR_SPEC, AREA_RIGHT, speed, speed, speed);

		speed = period_sec;
		area(handle, CMD_COLOR_SPEC, AREA_LEFT + 3, color_middle, level, 0x00);
		area(handle, CMD_COLOR_SPEC, AREA_MIDDLE + 3, color_middle2, level, 0x00);
		area(handle, CMD_COLOR_SPEC, AREA_RIGHT + 3, speed, speed, speed);

		speed = period_sec;
		area(handle, CMD_COLOR_SPEC, AREA_LEFT + 6, color_right, level, 0x00);
		area(handle, CMD_COLOR_SPEC, AREA_MIDDLE + 6, color_right2, level, 0x00);
		area(handle, CMD_COLOR_SPEC, AREA_RIGHT + 6, speed, speed, speed);
		break;

	case MODE_DEMO:
		/*TODO: does not work */
		break;

	default:
		break;
	}

	err = commit(handle, light_mode);
	HID_CHECK_ERROR(err, "commit");

	err = hid_exit();

	return 0;
}

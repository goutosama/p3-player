// 'loop1', 3x6px
const unsigned char bitmap_icon_loop1 [] PROGMEM = {
	0xc0, 0x80, 0x40, 0xc0, 0xc0, 0xc0
};
// 'loop', 14x11px
const unsigned char bitmap_icon_loop [] PROGMEM = {
	0xc0, 0x1c, 0x9f, 0xc8, 0x3f, 0xf0, 0xff, 0xe0, 0xff, 0xfc, 0xff, 0xfc, 0x1f, 0xfc, 0x3f, 0xf0, 
	0x4f, 0xe4, 0xe0, 0x0c, 0xff, 0xfc
};
// 'shuffle', 14x11px
const unsigned char bitmap_icon_shuffle [] PROGMEM = {
	0xff, 0xec, 0x3f, 0xf4, 0xdf, 0x80, 0xef, 0x74, 0xf6, 0xec, 0xf9, 0xfc, 0xf9, 0xec, 0xf6, 0xf4, 
	0xef, 0x00, 0x1f, 0xf4, 0xff, 0xec
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 128)
const int bitmap_icon_allArray_LEN = 3;
const unsigned char* bitmap_icon_allArray[3] = {
	bitmap_icon_loop,
	bitmap_icon_loop1,
	bitmap_icon_shuffle
};

// 'loop', 14x11px
const unsigned char bitmap_icon_loop [] PROGMEM = {
	0x3f, 0xe0, 0x60, 0x34, 0xc0, 0x0c, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0xc0, 0x0c, 
	0xb0, 0x18, 0x1f, 0xf0, 0x00, 0x00
};
// 'loop1', 3x6px
const unsigned char bitmap_icon_loop1 [] PROGMEM = {
	0x20, 0x60, 0xa0, 0x20, 0x20, 0x20
};
// 'shuffle', 14x11px
const unsigned char bitmap_icon_shuffle [] PROGMEM = {
	0x00, 0x10, 0xc0, 0x08, 0x20, 0x7c, 0x10, 0x88, 0x09, 0x10, 0x06, 0x00, 0x06, 0x10, 0x09, 0x08, 
	0x10, 0xfc, 0xe0, 0x08, 0x00, 0x10
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 128)
const int bitmap_icon_allArray_LEN = 3;
const unsigned char* bitmap_icon_allArray[3] = {
	bitmap_icon_loop,
	bitmap_icon_loop1,
	bitmap_icon_shuffle
};

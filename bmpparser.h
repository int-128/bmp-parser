#pragma once
#include <stdint.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <memory>


bool is_white(RGBQUAD* pixel);


class PixelData {

private:

	std::vector<std::vector<RGBQUAD> > pixel_data;
	uint16_t color_bit_count;
	uint32_t width;
	uint32_t height;
	bool pixel_order;
	uint16_t color_size;
	uint16_t alignment_byte_count;

public:
	
	PixelData(BITMAPINFO& bitmap_info);

	void read(std::ifstream& stream);

	void write(std::ofstream& stream);

	RGBQUAD* pixel(uint32_t x, uint32_t y);
};


class BMPParser {

private:

	BITMAPFILEHEADER bmp_file_header;
	BITMAPINFO bmp_info;
	std::unique_ptr<PixelData> pixel_data;
	bool bmp_loaded = false;

public:

	uint32_t width();

	uint32_t height();

	void load(const char* file_path);

	void save(const char* file_path);

	void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, bool color);

	void print(std::ostream& stream, char black_pixel=' ', char white_pixel='X');

};
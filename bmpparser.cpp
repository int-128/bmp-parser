#include "bmpparser.h"


bool is_white(RGBQUAD* pixel) {
	return (pixel->rgbRed + pixel->rgbGreen + pixel->rgbBlue > 382);
}


PixelData::PixelData(BITMAPINFO& bitmap_info) {
	color_bit_count = bitmap_info.bmiHeader.biBitCount;
	width = bitmap_info.bmiHeader.biWidth;
	height = abs(bitmap_info.bmiHeader.biHeight);
	pixel_order = bitmap_info.bmiHeader.biHeight > 0;
	color_size = color_bit_count >> 3;
	alignment_byte_count = (4 - (width * (color_bit_count >> 3)) % 4) % 4;
}

void PixelData::read(std::ifstream& stream) {
	pixel_data.resize(height);
	char byte;
	for (uint32_t i = 0, y; i < height; ++i) {
		if (pixel_order)
			y = height - i - 1;
		else
			y = i;
		pixel_data[y].resize(width);
		for (uint32_t x = 0; x < width; ++x) {
			stream.read((char*)&pixel_data[y][x], color_size);
		}
		for (uint32_t j = 0; j < alignment_byte_count; ++j)
			stream.read(&byte, sizeof(byte));
	}
}

void PixelData::write(std::ofstream& stream) {
	char byte = 0;
	for (uint32_t i = 0, y; i < height; ++i) {
		if (pixel_order)
			y = height - i - 1;
		else
			y = i;
		for (uint32_t x = 0; x < width; ++x) {
			stream.write((char*)&pixel_data[y][x], color_size);
		}
		for (uint32_t j = 0; j < alignment_byte_count; ++j)
			stream.write(&byte, sizeof(byte));
	}
}

RGBQUAD* PixelData::pixel(uint32_t x, uint32_t y) {
	return &pixel_data[y][x];
}


uint32_t BMPParser::width() {
	if (!bmp_loaded)
		return 0;
	return bmp_info.bmiHeader.biWidth;
}

uint32_t BMPParser::height() {
	if (!bmp_loaded)
		return 0;
	return abs(bmp_info.bmiHeader.biHeight);
}

void BMPParser::load(const char* file_path) {
	std::ifstream file = std::ifstream(file_path, std::ios::binary);
	file.read((char*)&bmp_file_header, sizeof(bmp_file_header));
	file.read((char*)&bmp_info.bmiHeader, sizeof(bmp_info.bmiHeader));
	pixel_data = std::make_unique<PixelData>(bmp_info);
	pixel_data->read(file);
	file.close();
	bmp_loaded = true;
}

void BMPParser::save(const char* file_path) {
	std::ofstream file = std::ofstream(file_path, std::ios::binary);
	file.write((char*)&bmp_file_header, sizeof(bmp_file_header));
	file.write((char*)&bmp_info.bmiHeader, sizeof(bmp_info.bmiHeader));
	pixel_data->write(file);
	file.close();
}

void BMPParser::draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, bool color) {
	if (x1 > x2) {
		uint32_t bf = x1;
		x1 = x2;
		x2 = bf;
		bf = y1;
		y1 = y2;
		y2 = bf;
	}
	uint32_t width = bmp_info.bmiHeader.biWidth;
	uint32_t height = abs(bmp_info.bmiHeader.biHeight);
	int32_t dx = x2 - x1;
	int32_t dy = y2 - y1;
	double mult = (double)dy / dx;
	for (uint32_t i = 0; i < dx + 1; ++i) {
		uint32_t x = x1 + i;
		uint32_t y = y1 + i * mult;
		pixel_data->pixel(x, y)->rgbRed = 255 * color;
		pixel_data->pixel(x, y)->rgbGreen = 255 * color;
		pixel_data->pixel(x, y)->rgbBlue = 255 * color;
	}
}

void BMPParser::print(std::ostream& stream, char black_pixel, char white_pixel) {
	uint32_t width = bmp_info.bmiHeader.biWidth;
	uint32_t height = abs(bmp_info.bmiHeader.biHeight);
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			if (is_white(pixel_data->pixel(x, y)))
				stream << white_pixel;
			else
				stream << black_pixel;
		}
		stream << '\n';
	}
}
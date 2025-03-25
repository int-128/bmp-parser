#pragma once
#include <stdint.h>
#include <windows.h>
#include <fstream>


#define read_from_binary_stream_code {\
	uint8_t byte;\
	(*field) = 0;\
	for (int i = 0; i < sizeof(*field); ++i) {\
		stream >> byte;\
		(*field) += (byte << (i << 3));\
	}\
}

void read_from_binary_stream(std::ifstream &stream, WORD* field) read_from_binary_stream_code
void read_from_binary_stream(std::ifstream &stream, DWORD* field) read_from_binary_stream_code
void read_from_binary_stream(std::ifstream &stream, LONG* field) read_from_binary_stream_code


#define write_to_binary_stream_code {\
	uint8_t byte;\
	for (int i = 0; i < sizeof(field); ++i) {\
		byte = (field >> (i << 3)) % 256;\
		stream << byte;\
	}\
}

void write_to_binary_stream(std::ofstream& stream, WORD& field) write_to_binary_stream_code
void write_to_binary_stream(std::ofstream& stream, DWORD& field) write_to_binary_stream_code
void write_to_binary_stream(std::ofstream& stream, LONG& field) write_to_binary_stream_code


bool is_white(RGBQUAD* pixel);

class PixelData {

private:

	RGBQUAD*** pixel_data = nullptr;
	uint16_t color_bit_count;
	uint32_t width;
	uint32_t height;
	bool pixel_order;

	void delete_pixel_data() {
		if (pixel_data == nullptr)
			return;
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				delete pixel_data[y][x];
			}
			delete[] pixel_data[y];
		}
		delete[] pixel_data;
		pixel_data = nullptr;
	}

public:
	
	PixelData(BITMAPINFO &bitmap_info) {
		color_bit_count = bitmap_info.bmiHeader.biBitCount;
		width = bitmap_info.bmiHeader.biWidth;
		height = abs(bitmap_info.bmiHeader.biHeight);
		pixel_order = bitmap_info.bmiHeader.biHeight > 0;
	}

	~PixelData() {
		delete_pixel_data();
	}

	void read(std::ifstream &stream) {
		delete_pixel_data();
		pixel_data = new RGBQUAD**[height];
		uint32_t alignment = (4 - (width * (color_bit_count >> 3)) % 4) % 4;
		uint8_t byte;
		for (uint32_t i = 0, y; i < height; ++i) {
			if (pixel_order)
				y = height - i - 1;
			else
				y = i;
			pixel_data[y] = new RGBQUAD*[width];
			for (uint32_t x = 0; x < width; ++x) {
				pixel_data[y][x] = new RGBQUAD;
				stream >> pixel_data[y][x]->rgbBlue;
				stream >> pixel_data[y][x]->rgbGreen;
				stream >> pixel_data[y][x]->rgbRed;
				if (color_bit_count == 32)
					stream >> pixel_data[y][x]->rgbReserved;
			}
			for (uint32_t j = 0; j < alignment; ++j)
				stream >> byte;
		}
	}

	void write(std::ofstream &stream) {
		uint32_t alignment = (4 - (width * (color_bit_count >> 3)) % 4) % 4;
		uint8_t byte = 0;
		for (uint32_t i = 0, y; i < height; ++i) {
			if (pixel_order)
				y = height - i - 1;
			else
				y = i;
			for (uint32_t x = 0; x < width; ++x) {
				stream << pixel_data[y][x]->rgbBlue;
				stream << pixel_data[y][x]->rgbGreen;
				stream << pixel_data[y][x]->rgbRed;
				if (color_bit_count == 32)
					stream << pixel_data[y][x]->rgbReserved;
			}
			for (uint32_t j = 0; j < alignment; ++j)
				stream << byte;
		}
	}

	RGBQUAD* pixel(uint32_t x, uint32_t y) {
		return pixel_data[y][x];
	}
};


bool is_white(RGBQUAD* pixel) {
	return (pixel->rgbRed + pixel->rgbGreen + pixel->rgbBlue > 382);
}


class BMPParser {

private:

	BITMAPFILEHEADER bmp_file_header;
	BITMAPINFO bmp_info;
	PixelData* pixel_data = nullptr;
	bool bmp_loaded = false;

	void free_allocated_memory() {
		if (pixel_data != nullptr) {
			delete pixel_data;
			pixel_data = nullptr;
		}
	}

public:

	~BMPParser() {
		free_allocated_memory();
	}

	uint32_t width() {
		if (!bmp_loaded)
			return 0;
		return bmp_info.bmiHeader.biWidth;
	}

	uint32_t height() {
		if (!bmp_loaded)
			return 0;
		return abs(bmp_info.bmiHeader.biHeight);
	}

	void load(char* file_path) {
		std::ifstream file = std::ifstream(file_path, std::ios::binary);
		read_from_binary_stream(file, &bmp_file_header.bfType);
		read_from_binary_stream(file, &bmp_file_header.bfSize);
		read_from_binary_stream(file, &bmp_file_header.bfReserved1);
		read_from_binary_stream(file, &bmp_file_header.bfReserved2);
		read_from_binary_stream(file, &bmp_file_header.bfOffBits);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biSize);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biWidth);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biHeight);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biPlanes);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biBitCount);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biCompression);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biSizeImage);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biXPelsPerMeter);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biYPelsPerMeter);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biClrUsed);
		read_from_binary_stream(file, &bmp_info.bmiHeader.biClrImportant);
		pixel_data = new PixelData(bmp_info);
		pixel_data->read(file);
		file.close();
		bmp_loaded = true;
	}

	void save(char* file_path) {
		std::ofstream file = std::ofstream(file_path, std::ios::binary);
		write_to_binary_stream(file, bmp_file_header.bfType);
		write_to_binary_stream(file, bmp_file_header.bfSize);
		write_to_binary_stream(file, bmp_file_header.bfReserved1);
		write_to_binary_stream(file, bmp_file_header.bfReserved2);
		write_to_binary_stream(file, bmp_file_header.bfOffBits);
		write_to_binary_stream(file, bmp_info.bmiHeader.biSize);
		write_to_binary_stream(file, bmp_info.bmiHeader.biWidth);
		write_to_binary_stream(file, bmp_info.bmiHeader.biHeight);
		write_to_binary_stream(file, bmp_info.bmiHeader.biPlanes);
		write_to_binary_stream(file, bmp_info.bmiHeader.biBitCount);
		write_to_binary_stream(file, bmp_info.bmiHeader.biCompression);
		write_to_binary_stream(file, bmp_info.bmiHeader.biSizeImage);
		write_to_binary_stream(file, bmp_info.bmiHeader.biXPelsPerMeter);
		write_to_binary_stream(file, bmp_info.bmiHeader.biYPelsPerMeter);
		write_to_binary_stream(file, bmp_info.bmiHeader.biClrUsed);
		write_to_binary_stream(file, bmp_info.bmiHeader.biClrImportant);
		pixel_data->write(file);
		file.close();
	}

	void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, bool color) {
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
		for (uint32_t i = 0; i < dx; ++i) {
			uint32_t x = x1 + i;
			uint32_t y = y1 + i * mult;
			pixel_data->pixel(x, y)->rgbRed = 255 * color;
			pixel_data->pixel(x, y)->rgbGreen = 255 * color;
			pixel_data->pixel(x, y)->rgbBlue = 255 * color;
		}
	}

	void print(std::ostream& stream, char black_pixel=' ', char white_pixel='X') {
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

};
#include <iostream>
#include <string.h>
#include "bmpparser.h"


int main() {

	int max_file_path_len = 256;
	char* file_path = new char[256];
	uint32_t i;

	std::cout << "Enter input BMP file name: ";
	fgets(file_path, max_file_path_len, stdin);
	i = strlen(file_path);
	file_path[i - 1] = 0;
	BMPParser bmp_parser = BMPParser();
	bmp_parser.load(file_path);
	bmp_parser.print(std::cout);

	uint32_t width = bmp_parser.width(), height = bmp_parser.height();
	bmp_parser.draw_line(0, 0, width - 1, height - 1, 1);
	bmp_parser.draw_line(0, height - 1, width - 1, 0, 1);
	bmp_parser.print(std::cout);

	std::cout << "Enter output BMP file name: ";
	fgets(file_path, max_file_path_len, stdin);
	i = strlen(file_path);
	file_path[i - 1] = 0;
	bmp_parser.save(file_path);

	delete[] file_path;

	return 0;
}
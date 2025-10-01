#include <iostream>
#include <string>
#include <string.h>
#include "bmpparser.h"


int main() {

	std::string file_path;
	uint32_t i;

	std::cout << "Enter input BMP file name>";
	std::getline(std::cin, file_path);
	BMPParser bmp_parser = BMPParser();
	bmp_parser.load(file_path.c_str());
	bmp_parser.print(std::cout);

	uint32_t width = bmp_parser.width(), height = bmp_parser.height();
	bmp_parser.draw_line(0, 0, width - 1, height - 1, 1);
	bmp_parser.draw_line(0, height - 1, width - 1, 0, 1);
	bmp_parser.print(std::cout);

	std::cout << "Enter output BMP file name>";
	std::getline(std::cin, file_path);
	bmp_parser.save(file_path.c_str());

	return 0;
}
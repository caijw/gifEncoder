#ifndef LOGGER_H
#define LOGGER_H
#include <vector>



class Logger {
	
	public:
		Logger();

		static int defaultNumPerLine;
		
		static void log(std::vector<uint8_t> data, int numPerLine, int itemSize, int maxLineNums);

		static void log(double network[][4], int);

		static void log(int *netindex, int size);

		static void log(std::vector<unsigned char> colorTab);

};


#endif
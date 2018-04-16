#include <iostream>
#include "Logger.h"
#include <iomanip>
using namespace std;


int Logger::defaultNumPerLine = 15;


Logger::Logger(){

};



void Logger::log(std::vector<uint8_t> data, int numPerLine, int itemSize, int maxLineNums){
	if(numPerLine <= 0){
		numPerLine = defaultNumPerLine;
	}

	int len = data.size();
	int lines = (int)(len / numPerLine);
	if(lines > maxLineNums){
		lines = maxLineNums;
	}

	int left = len - numPerLine * lines;
	if(lines >= maxLineNums){
		left = 0;
	}


	for(int i = 0; i < lines; i++){

		std::cout << std::dec << std::setfill('0') << std::setw(4) << i << ":  ";

		for(int j = 0; j < numPerLine; j++){

			std::cout << std::hex  << std::setw(2) << std::setfill('0') << (unsigned int)data[i * numPerLine + j];

			if( (j % itemSize) == (itemSize - 1)){

				std::cout << "  ";

			}else{
				
			}
			
		}
		std::cout << std::endl;
	}
	if(left > 0){
		std::cout << std::dec << std::setfill('0') << std::setw(4) << lines << ":  ";
		for(int i = 0; i < left; i++){
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)data[lines * numPerLine + i];
			if( (i % itemSize) == (itemSize - 1)){
				 std::cout << "  ";
			}else{

			}
			
		}
		std::cout << std::endl;
	}

}

void Logger::log(double network[][4], int netsize){

	for(int i = 0; i < netsize; i++){
		std::cout << std::dec << std::setfill('0') << std::setw(4) << i << ":  ";
		for(int j = 0; j < 4; j++){

			std::cout << std::fixed << std::setprecision(20) << network[i][j] << "  ";

		}
		std::cout << std::endl;
	}
}

void Logger::log(int *netindex, int size){
	for(int i = 0; i < size; i++){
		cout << netindex[i] << endl;
	}
}

void Logger::log(std::vector<unsigned char> colorTab){
	for(int i = 0; i < colorTab.size(); i++){
		cout << (int)colorTab[i] << endl;
	}
}
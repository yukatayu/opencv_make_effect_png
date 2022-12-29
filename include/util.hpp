#pragma once
#include <cstdio>
#include <sstream>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

// -+-+-+-+-+-+-+-+-+-+- //
//        Utility        //
// -+-+-+-+-+-+-+-+-+-+- //

std::string zero_ume(int i, int width = 6){
	std::ostringstream ss;
	ss << std::setfill('0') << std::right << std::setw(width) << i;
	return ss.str();
}

void progress_bar(long long int numerator, long long int denominator){
	struct winsize winsz;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);
	++numerator;
	std::ostringstream ss;
	ss << "\e[2K\r |";
	int max = winsz.ws_col - 20 - 2*std::log10(denominator);
	for(int i=0; i<max; ++i)
		ss << (numerator* max >= i * denominator ? "\e[42m" : "\e[41m") << " ";
	ss << "\e[0m| "
		<< numerator << " / " << denominator
		<< " (" << int(double(numerator*100) / denominator) << '%' << ")";
	std::cerr << ss.str() << std::flush;
};

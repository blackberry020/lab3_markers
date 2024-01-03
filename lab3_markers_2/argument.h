#pragma once
#include <windows.h>

struct arg {
	HANDLE* threads;
	HANDLE* stopEvents;
	HANDLE* continueEvents;
	bool* isActive;
	int cntNumbers;
	int* numbers;
	int cntThread;

	arg(HANDLE* _threads, HANDLE* _stopEvents, HANDLE* _cEvents, bool* _active, int _cnt, int* _num) :
		threads(_threads), stopEvents(_stopEvents), continueEvents(_cEvents), isActive(_active), cntNumbers(_cnt), numbers(_num), cntThread(-1) {};
};

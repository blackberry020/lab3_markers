#include <iostream>
#include <vector>

#include "argument.h"

CRITICAL_SECTION critical_section;

// signal to start threads' work
HANDLE hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);

// function for the threads
DWORD WINAPI func(LPVOID argument) {

	// wait for start signal to start the work
	WaitForSingleObject(hStartSignal, INFINITE);

	arg* info = static_cast<arg*>(argument);
	std::vector <int> marked;

	srand(info->cntThread);

	// while thread didn't find marked element
	while (1) {
		// randomly get an index of element to mark
		int x = rand();
		int index = x % info->cntNumbers;

		EnterCriticalSection(&critical_section);

		// if an element can be marked
		if (info->numbers[index] == 0) {

			Sleep(5);

			// mark the element
			info->numbers[index] = info->cntThread;
			marked.push_back(index);
			LeaveCriticalSection(&critical_section);

			Sleep(5);
		}
		else {
			// print all the information
			std::cout << "number of the thread: " << info->cntThread << std::endl;
			std::cout << "marked " << marked.size() << " elements" << std::endl;
			std::cout << "couldn't mark " << index << " element" << std::endl;

			LeaveCriticalSection(&critical_section);

			// tell main part about finding marked elment and wait for response
			SetEvent(info->stopEvents[info->cntThread - 1]);
			WaitForSingleObject(info->continueEvents[info->cntThread - 1], INFINITE);

			EnterCriticalSection(&critical_section);

			// if the thread is to be closed
			if (!info->isActive[info->cntThread - 1]) {

				// unmark the elements
				for (auto i : marked) {
					info->numbers[i] = 0;
				}

				LeaveCriticalSection(&critical_section);

				return 0;
			}

			// continue to work if the therad wasn't closed
			LeaveCriticalSection(&critical_section);
		}
	}

	return 0;
}

int main()
{
	InitializeCriticalSection(&critical_section);

	std::cout << "enter the amount of your numbers" << std::endl;
	int n;
	std::cin >> n;

	int* numbers = new int[n];
	for (int i = 0; i < n; i++)
		numbers[i] = 0;

	std::cout << "enter the amount of your threads" << std::endl;
	int cntThreads;
	std::cin >> cntThreads;

	// for the threads
	HANDLE* hThread = new HANDLE[cntThreads];
	DWORD* IDThread = new DWORD[cntThreads];
	bool* isActive = new bool[cntThreads];
	HANDLE* hStopEvents = new HANDLE[cntThreads];
	HANDLE* hContinueEvents = new HANDLE[cntThreads];

	// in the beginning all the threads are active
	for (int i = 0; i < cntThreads; i++) {
		isActive[i] = true;
	}

	// create events for each thread
	for (int i = 0; i < cntThreads; i++) {
		hStopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		hContinueEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	// create threads
	for (int i = 0; i < cntThreads; i++) {

		arg* argument = new arg(hThread, hStopEvents, hContinueEvents, isActive, n, numbers);
		argument->cntThread = i + 1;

		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (void*)argument, 0, &IDThread[i]);

		if (hThread == NULL) {
			std::cout << "could not create a thread";
			return 0;
		}
	}

	// start threads' work
	SetEvent(hStartSignal);

	int numToTerminate = 0;
	int cntActive = cntThreads;

	// main cycle
	while (cntActive > 0) {

		// wait for all threads to stop their work
		for (int i = 0; i < cntThreads; i++) {
			WaitForSingleObject(hStopEvents[i], INFINITE);
		}

		// print current marks to choose a thread to close
		std::cout << "current marks:" << std::endl;

		for (int i = 0; i < n; i++) {
			std::cout << numbers[i] << " ";
		}
		std::cout << std::endl;

		// get thread's number to close it
		bool correctInput = false;

		while (!correctInput) {
			std::cout << "enter the number of thread you want to terminate" << std::endl;
			std::cin >> numToTerminate;

			if (!(numToTerminate > 0 && numToTerminate <= cntThreads))
				std::cout << "out of range, enter again" << std::endl;
			else if (isActive[numToTerminate - 1] == false)
				std::cout << "already terminated, enter again" << std::endl;
			else correctInput = true;
		}

		isActive[numToTerminate - 1] = false;
		cntActive--;

		// wait for the chosen thread and close it
		SetEvent(hContinueEvents[numToTerminate - 1]);
		WaitForSingleObject(hThread[numToTerminate - 1], INFINITE);
		CloseHandle(hThread[numToTerminate - 1]);

		// let other threads to continue
		for (int i = 0; i < cntThreads; i++) {
			if (isActive[i])
				ResetEvent(hStopEvents[i]);
		}

		for (int i = 0; i < cntThreads; i++) {
			if (isActive[i])
				SetEvent(hContinueEvents[i]);
		}
	}

	DeleteCriticalSection(&critical_section);

	return 0;
}
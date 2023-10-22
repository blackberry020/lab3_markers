#include <windows.h>
#include <iostream>
#include <string>

CRITICAL_SECTION critical_section;

struct arg {
	HANDLE* threads;
	bool* isActive;
	int cntNumbers;
	int* numbers;
	int cntThread;
};

int countNonZeroElements(int* mas, int n) {
	int cnt = 0;

	for (int i = 0; i < n; i++) {
		if (mas[i]) cnt++;
	}

	return cnt;
}

DWORD WINAPI func(arg* info) {

	srand(info->cntThread);

	while (1) {
		int x = rand();
		int index = x % info->cntNumbers;

		EnterCriticalSection(&critical_section);

		if (info->numbers[index] == 0) {
			Sleep(5);
			info->numbers[index] = info->cntThread;
			Sleep(5);
		}
		else {

			WaitForSingleObject(info->threads[info->cntThread], INFINITE);

			if (!info->isActive[info->cntThread]) {
				std::cout << info->cntThread;
				std::cout << "marked " << countNonZeroElements(info->numbers, info->cntNumbers) << " elements" << std::endl;
				std::cout << "couldn't mark " << index << " element" << std::endl;

				return 0;
			}
		}

		LeaveCriticalSection(&critical_section);
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

	HANDLE* hThread = new HANDLE[cntThreads];
	DWORD* IDThread = new DWORD[cntThreads];
	bool* isActive = new bool[cntThreads];

	for (int i = 0; i < cntThreads; i++) {
		isActive[i] = true;
	}

	for (int i = 0; i < cntThreads; i++) {

		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (void*)i, 0, &IDThread[i]);

		if (hThread == NULL) {
			std::cout << "could not create a thread";
			return 0;
		}
	}

	int numToTerminate = 0;

	while (1) {
		WaitForMultipleObjects(cntThreads, hThread, TRUE, INFINITE);

		std::cout << "enter the number of thread you want to terminate" << std::endl;
		std::cin >> numToTerminate;

		isActive[numToTerminate] = false;

		SetSignal(hThread[numToTerminate]);

		for (int i = 0; i < n; i++) {
			std::cout << numbers[i] << " ";
		}
	}

	for (int i = 0; i < cntThreads; i++) {
		CloseHandle(hThread[i]);
	}

	DeleteCriticalSection(&critical_section);

	return 0;
}
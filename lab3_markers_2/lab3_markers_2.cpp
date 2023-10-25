#include <windows.h>
#include <iostream>
#include <vector>

CRITICAL_SECTION critical_section;
HANDLE hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);

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

DWORD WINAPI func(LPVOID argument) {

	WaitForSingleObject(hStartSignal, INFINITE);

	arg* info = static_cast<arg*>(argument);

	std::vector <int> marked;

	srand(info->cntThread);

	while (1) {
		int x = rand();
		int index = x % info->cntNumbers;

		EnterCriticalSection(&critical_section);

		if (info->numbers[index] == 0) {

			Sleep(5);
			info->numbers[index] = info->cntThread;
			marked.push_back(index);
			LeaveCriticalSection(&critical_section);

			Sleep(5);
		}
		else {

			std::cout << "number of the thread: " << info->cntThread << std::endl;
			std::cout << "marked " << marked.size() << " elements" << std::endl;
			std::cout << "couldn't mark " << index << " element" << std::endl;

			LeaveCriticalSection(&critical_section);

			SetEvent(info->stopEvents[info->cntThread - 1]);

			//std::cout << info->cntThread << " is waiting for the main" << std::endl;

			// wait for the signal
			WaitForSingleObject(info->continueEvents[info->cntThread - 1], INFINITE);

			EnterCriticalSection(&critical_section);

			if (!info->isActive[info->cntThread - 1]) {

				for (auto i : marked) {
					info->numbers[i] = 0;
				}

				LeaveCriticalSection(&critical_section);

				return 0;
			}

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

	HANDLE* hThread = new HANDLE[cntThreads];
	DWORD* IDThread = new DWORD[cntThreads];
	bool* isActive = new bool[cntThreads];
	HANDLE* hStopEvents = new HANDLE[cntThreads];
	HANDLE* hContinueEvents = new HANDLE[cntThreads];

	for (int i = 0; i < cntThreads; i++) {
		isActive[i] = true;
	}

	for (int i = 0; i < cntThreads; i++) {
		hStopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		hContinueEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	for (int i = 0; i < cntThreads; i++) {

		arg* argument = new arg(hThread, hStopEvents, hContinueEvents, isActive, n, numbers);
		argument->cntThread = i + 1;

		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (void*)argument, 0, &IDThread[i]);

		if (hThread == NULL) {
			std::cout << "could not create a thread";
			return 0;
		}
	}

	SetEvent(hStartSignal);

	int numToTerminate = 0;
	int cntActive = cntThreads;

	while (cntActive > 0) {

		//std::cout << "main is waiting" << std::endl;

		for (int i = 0; i < cntThreads; i++) {
			WaitForSingleObject(hStopEvents[i], INFINITE);
		}

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

		//if (isActive[0] == false) std::cout << "should be closed" << std::endl;

		cntActive--;

		SetEvent(hContinueEvents[numToTerminate - 1]);

		WaitForSingleObject(hThread[numToTerminate - 1], INFINITE);

		CloseHandle(hThread[numToTerminate - 1]);

		for (int i = 0; i < n; i++) {
			std::cout << numbers[i] << " ";
		}

		std::cout << std::endl;

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
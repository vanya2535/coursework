// Десятников И.С.
// Сортировка последовательности чисел по медиане
// Архитектура вычислительных систем
// ИВТ1-20
// 2021 г.

#pragma runtime_checks( "", off )
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>


using namespace std;

ofstream outputFile;

void _fastcall printNum(float num)
{
	cout << num << " ";
	outputFile << num << " ";
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	float nums[100];
	float sortedNums[100];

	string path = "D:\\test.txt"; // Переменная пути к файлу
	cout << "Введите путь к файлу: ";
	cin >> path; // Ввод пути к файлу .txt
	cout << endl;

	ifstream inputFile(path);
	if (!inputFile) // Проверка: открылся ли файл?
	{
		cout << "Не удалось открыть файл" << endl;
		return 0;
	}

	cout << endl << "Входные данные:" << endl;

	int massSize = 0;
	string tempElem;
	while (!inputFile.eof())
	{
		inputFile >> tempElem;
		if (sscanf(tempElem.c_str(), "%f", &nums[massSize++]) != 1)
		{
			cout << endl << "Файл невалиден" << endl;
			return 0;
		}
		cout << nums[massSize - 1] << " ";
	}

	cout << endl;
	if (massSize == 1)
	{
		cout << endl << "Файл невалиден" << endl;
		return 0;
	}
	outputFile.open(path.erase(path.find(".txt")) + "_result.txt"); // Открываем файл для записи
	memcpy(sortedNums, nums, sizeof(float) * 100);

	inputFile.close();
	
	float trash;
	int tempInt;

	// АСМ вставка для сортировки чисел
	__asm {

		mov eax, 0 // i
		mov ecx, massSize // size
		dec ecx	// size -1
		cycle1: // Внешний цикл
			
			push eax
			push ecx 

			dec ecx // size - 1
			sub ecx, eax // size - i
			mov eax, 0 // j

			cycle2:

				// j
				FLD [sortedNums + eax * 4] // Заносим в стек сопроцессора значение по j    | Условное обозначение st
				inc eax
				// j + 1
				FLD [sortedNums + eax * 4] // Заносим в стек сопроцессора значение по j + 1 | Условное обозначение st(1)
				FXCH st(1)
				dec eax

				// j > j + 1 ?
				FCOMI st, st(1) // Сравниваем эти 2 значения
				ja swapNums // Если st > st(1), то переходим к метке swapNums

				FSTP trash
				FSTP trash
				jmp nextIter // Иначе - переходим к метке nextIter
				
				swapNums:
				
				// Тут происходит свап
				inc eax
				FSTP [sortedNums + eax * 4] // Выгружаем st
				dec eax
				FSTP [sortedNums + eax * 4] // Выгружаем st(1)

				
				nextIter:
				inc eax
				cmp ecx, eax
				jb exitLoop2

			jmp cycle2

			exitLoop2:
			pop ecx
			pop eax
			inc eax

			cmp ecx,eax
			je exitLoop1

		jmp cycle1

		exitLoop1:

	}

	float median = 0;
	float var2 = 2.0;
	float var05 = 0.5;

	// АСМ вставка для поиска медианы
	__asm {

		FLD var2							// Загрузка 2 в СС
		FILD massSize						// Загрузка кол-ва элементов в массиве в СС
		
		FPREM								// massSize % var2 
		FLDZ								// Загружаем в стек 0
		FCOMIP st, st(1)					// Сравнивает результат FPREM (лежит в st(1)) и 0
		je chetNum 							// Переход если st(1) == 0, т.е. massSize % var2 - вернуло остаток 0

											// Иначе выполняем инструкции для нечётного кол-ва элементов в массиве

		FSTP trash							// Очистка СС на 1
		FILD massSize						// Загрузка кол-ва элементов в массиве
		FXCH st(1)							// Обмен значений	
		FDIVP st(1),st						// Делим кол-ва элементов в массиве на 2

		FLD var05							// Загрузка 0.5 в СС
		FSUBP st(1),st						// Вычитаем из полученного выше числа 0.5 и получаем индекс числа, который является медианой
		FISTP tempInt						// Выгружаем индекс в tempInt

		mov ecx, tempInt					// В ecx сохраняем tempInt(индекс)
		mov ecx, [sortedNums + ecx * 4]		// В ecx сохраняем число по индексу
		mov median, ecx						// В median сохраняем ecx
			
		jmp endMedianSearch					// Выходим из вставки

		chetNum:

		FSTP trash							// Очистка СС на 1
		FILD massSize						// Загрузка кол-ва элементов в массиве
		FXCH st(1)							// Обмен значений	
		FDIVP st(1), st						// Делим кол-ва элементов в массиве на 2 и находим первый индекс для нахождения медианы числа

		FISTP tempInt						// Выгружаем индекс в tempInt
		mov ecx, tempInt					// В ecx сохраняем tempInt(индекс)
		FLD [sortedNums + ecx * 4]			// Грузим в СС первое число
		FLD [sortedNums + ecx * 4 - 4]		// Грузим в СС второе число

		FADDP st(1),st						// Складываем их
		FLD var2							// Загрузка 2 в СС
		FDIVP st(1),st						// Делим сумму на 2 и получаем медиану для четного числа элементов в массиве

		FSTP median

		endMedianSearch:

	}

	cout << endl;
	cout << "Медиана: " << median << endl;

	// АСМ вставка для вывода чисел слева от медианы
	cout << endl;
	cout << "Числа слева от медианы: " << endl;

	__asm {

		mov ecx, massSize
		mov edx, 0

		lCycle:

			FLD [nums + edx * 4]
			FLD median
					
			FCOMIP st, st(1)
			jae printLS 
			jmp endIterLC

		printLS:

			mov eax, [nums + edx * 4]
			
			push edx
			push ecx
			push eax
			call printNum
					
			pop ecx
			pop edx
			add esp, 4

		endIterLC:

			FSTP trash							// Очистка СС на 1

			inc edx

		loop lCycle

	}

	// АСМ вставка для вывода чисел справа от медианы
	cout << endl;
	cout << "Числа справа от медианы: " << endl;

	__asm {

		mov ecx, massSize
		mov edx, 0

		rCycle:

			FLD [nums + edx * 4]
			FLD median
					
			FCOMIP st, st(1)
			jb printRS 
			jmp endIterRC

		printRS:

			mov eax, [nums + edx * 4]
			
			push edx
			push ecx
			push eax
			call printNum
					
			pop ecx
			pop edx
			add esp, 4

		endIterRC:

			FSTP trash							// Очистка СС на 1

			inc edx

		loop rCycle

	}

	cout << endl;

	outputFile.close();
	system("pause");
	return 0;
}


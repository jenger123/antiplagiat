
#include <iostream>
#include <windows.h>
#include <set>
using namespace std;
int main()
{
	setlocale(LC_ALL, "Russian");
	HANDLE slovar = CreateFileW(L"D:\\projects\\antiplagiat\\exclusions", GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (slovar == INVALID_HANDLE_VALUE)
	{
		cout << " Не удалось открыть словарь!\nВозможно неверно указан путь к файлу" << endl << endl;
		system("pause");
		exit(1);
	}
	BYTE * buf = new BYTE[65];
	DWORD b = 0;
	auto s = ReadFile(slovar, buf, 65, &b, NULL);
	if (!s)
	{
		cout << "Не удалось прочитать словарь, возможно неверно указан путь к файлу" << endl << endl;
		system("pause");
		exit(1);
	}
	CloseHandle(slovar);

	set <BYTE> exclusions;
	for (int i = 0; i < 65; i++)
		exclusions.insert(buf[i]);
	delete[] buf;
	
	WCHAR filename[200];
	cout << "Укажите путь к файлу" << endl << endl;
	wcin >> filename;
	cout << "Попытка открыть файл ";
	wcout<< filename << endl << endl;
	//LPCWSTR filename=L"D:\\projects\\antiplagiat\\test.txt";
	HANDLE fileHandle = CreateFileW(filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		cout << "Файл " << *filename << " не был открыт!\nВозможно неверно указан путь к файлу" << endl << endl;
		system("pause");
		exit(1);
	}
	int filesize = GetFileSize(fileHandle,NULL);
	cout << "Размер открытого файла равен " << filesize << " байт" << endl << endl;
	BYTE * buffer = new BYTE[filesize];
	DWORD BytesRead;
	BOOL r = ReadFile(fileHandle, buffer, filesize, &BytesRead, NULL);
	if (!r)
	{
		cout << "Файл не был прочитан, возможно неверно указан путь к файлу" << endl << endl;
		system("pause");
		exit(1);
	}
	else
	{
		cout << "Из файла "; wcout << filename; cout<< " прочитано " << BytesRead << " байтов" << endl << endl;
		cout << "Прочитанное содержимое файла: " << endl << endl;
		for (int i = 0; i < filesize; i++)
			cout << buffer[i];
		cout << endl;
	}
	delete[] buffer;
	CloseHandle(fileHandle);
	system("pause");
	return 0;
	
	
}
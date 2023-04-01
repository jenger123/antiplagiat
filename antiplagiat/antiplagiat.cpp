
#include <iostream>
#include <windows.h>
#include <set>
#include <vector>
#include "md5.h"
#include "sqlite3.h"


using namespace std;

string deletespaces(string str)
{
	string out;
	int it1 = 0, it2 = size(str) - 1;
	while (str[it1] == ' ')
		it1++;
	while (str[it2] == ' ')
		it2--;
	for (int i = it1; i <= it2; i++)
	{
		if (str[i] == ' ' && str[i + 1] == ' ')
			continue;
		else
			out.push_back(str[i]);
	}
	return out;

}


vector<std::string> DoShingle(string input, int shinglesize)
{
	vector<std::string> out;
	string output;
	int spaces = 0;
	int i = 0;
	int it = 0;
	while (input[i] != '\0')
	{
		if (input[i] == ' ')
			spaces++;
		if (spaces == 1 && input[i]==' ')
			it = i + 1;
		if (spaces == shinglesize)
		{
			spaces = 0;
			i = it;
			out.push_back(output);
			output.clear();
		}
		output.push_back(input[i]);
		i++;
	}
	return out;
}


int main()
{
	setlocale(LC_ALL, "Russian");
	// Читаем словарь исключений
	HANDLE slovar = CreateFileW(L"D:\\projects\\antiplagiat\\exclusions", GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (slovar == INVALID_HANDLE_VALUE) // обработка ошибок открытия файла
	{
		cout << " Не удалось открыть словарь!\nВозможно неверно указан путь к файлу" << endl << endl;
		std::system("pause");
		exit(1);
	}
	BYTE * buf = new BYTE[65]; // буфер для байтов - исключений
	DWORD b = 0;
	auto s = ReadFile(slovar, buf, 65, &b, NULL); // читаем файл
	if (!s) // Обрабатываем ошибки чтения
	{
		cout << "Не удалось прочитать словарь, возможно неверно указан путь к файлу" << endl << endl;
		std::system("pause");
		exit(1);
	}
	CloseHandle(slovar); // закрываем хэндл словаря

	set <BYTE> exclusions; // создаем множество исключений
	for (int i = 0; i < 65; i++)
		exclusions.insert(buf[i]); // добавляем исключения из буфера
	delete[] buf;
	int shingle_size;
	WCHAR filename[200];
	//wstring name;
	cout << "Укажите путь к файлу" << endl << endl;
	wcin >> filename; // получаем имя файла
	wcout << L"Попытка открыть файл "<< filename << endl << endl;

	HANDLE fileHandle = CreateFileW(filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		wcout << L"Файл " << filename << L" не был открыт!\nВозможно неверно указан путь к файлу" << endl << endl;
		std::system("pause");
		exit(1);
	}
	int filesize = GetFileSize(fileHandle,NULL); // Получаем размер открываемого файла в байтах
	cout << "Размер открытого файла равен " << filesize << " байт" << endl << endl;
	BYTE * buffer = new BYTE[filesize+1]; // сюда будем считывать необработанный текст
	buffer[filesize] = 0;
	DWORD BytesRead;
	BOOL r = ReadFile(fileHandle, buffer, filesize, &BytesRead, NULL);
	CloseHandle(fileHandle);
	if (!r)
	{
		cout << "Файл не был прочитан, возможно неверно указан путь к файлу" << endl << endl;
		std::system("pause");
		return 1;
	}
	string text((char*)buffer);
	for (int i = 0; i < size(text); i++)
	{
		if (exclusions.find(text[i]) != exclusions.end())
			text.replace(i,1," ");
		if (int(text[i]) <= 90 && int(text[i]) >= 65)
			text[i] = char(int(text[i] + 32));
	}
	auto worktext = deletespaces(text);
	worktext.push_back(' ');
	cout << "Обработанный текст:\n" << worktext<<endl;


	// Разбиение на шинглы

	cout << "Введите размер шингла(в словах): ";
	cin >> shingle_size;
	auto shingles = DoShingle(worktext, shingle_size);
	cout << "Получившиеся шинглы:" << endl;
	for (auto i : shingles)
		cout << i << "\t\t\t\tHASH:\t\t\t"<<md5(i)<< endl;

	// Работа с базой данных
	sqlite3* DB;
	auto q = sqlite3_open("shingles.db", &DB);
	if (q==0)
		std::cout << "Opened Database Successfully!" << std::endl;
	else
	{
		std::cerr << "Error open DB " << std::endl;
		return (-1);
	}
	const char * errmsg;
	sqlite3_stmt* pStatement;
	//cout <<  << endl;
	string SqlDrop = "DROP TABLE IF EXISTS file1;";

	int result = sqlite3_prepare_v2(DB, SqlDrop.c_str(), -1, &pStatement, NULL);
	if (result != SQLITE_OK)
		errmsg = sqlite3_errmsg(DB);
	result = sqlite3_step(pStatement);
	if (result != SQLITE_DONE)
	{
		cout << "Не удалось удалить существующую таблицу!" << endl;
		return 1;
	}
	sqlite3_finalize(pStatement);


	string SqlCreate = "CREATE TABLE IF NOT EXISTS `file1`(`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,`shingle` VARCHAR(255),`hash` VARCHAR(255));";
	result = sqlite3_prepare_v2(DB, SqlCreate.c_str(), -1, &pStatement, NULL);
	if (result != SQLITE_OK)
		errmsg = sqlite3_errmsg(DB);
	result = sqlite3_step(pStatement);
	if (result != SQLITE_DONE)
	{
		cout << "Таблица шинглов не была создана" << endl;
		return 1;
	}
	sqlite3_finalize(pStatement); 
	
	// Добавление шинглов в таблицу
	const char* errmsg_insert;
	sqlite3_stmt* pStatement_insert;
	string SqlInsert;
	for (auto i : shingles)
	{
		SqlInsert = "INSERT INTO file1(shingle,hash) VALUES (\'" + i + "\',\'" + md5(i) + "\');";

		result = sqlite3_prepare_v2(DB, SqlInsert.c_str(), -1, &pStatement_insert, NULL);
		if (result != SQLITE_OK)
			errmsg = sqlite3_errmsg(DB);
		result = sqlite3_step(pStatement_insert);
		if (result != SQLITE_DONE)
		{
			cout << "При записи шинглов в таблицу произошла ошибка!" << endl;
		}
		sqlite3_finalize(pStatement_insert);
	}
	
	sqlite3_close(DB);



	std::system("pause");
	return 0;
	
	
}
#pragma once

#include <Windows.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <functional>
#include <thread>

#include "Resource/resource.h"

class LoadingScreen
{
public:
	enum LoadType
	{
		Unknown = 0,
		Known = 1,
	};
private:
	static inline std::wstring FontFile = L"Resources\\CustomConsola.ttf";

	std::wstring SplashScreen;
	int SplashScreenYSize;
	float PercentageDone;
	std::wstring StatusMessage;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE ConsoleHandle;
	int columns, rows;

	LoadType BarType;

	template <typename Func, typename ... VariadicArgs>
	void KnownProgressLoad(Func&& callable, VariadicArgs&& ... args)
	{
		wprintf(SplashScreen.c_str());

		std::thread FunctionThread([this](Func&& callable, VariadicArgs&& ... args) { this->ThreadingFunction(callable, std::forward<VariadicArgs>(args)...); }, callable, std::forward<VariadicArgs>(args)...);

		GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

		std::wstring bar = L"";
		int Lenght = 50;

		SetConsoleCursorPosition(ConsoleHandle, { 0, (SHORT)SplashScreenYSize });
		while (PercentageDone < 1 && !CrossThreadFinishBoolean)
		{

			float left = PercentageDone * Lenght;

			bar += std::wstring(floor(left), L'█');

			left -= floor(left);
			bar += std::wstring(floor(left / 0.5), L'▌');

			wprintf((std::wstring(((columns / 2) - Lenght / 2), ' ') + bar + L'\n').c_str());
			wprintf((std::wstring(((columns / 2) - StatusMessage.length() / 2), ' ') + StatusMessage + L'\n').c_str());

			Sleep(100);
			LoadingScreen::ClearCurrentLine(SplashScreenYSize);
			bar = L"";
		}

		FunctionThread.join();
	}

	template <typename Func, typename ... VariadicArgs>
	void UnknownProgressLoad(Func&& callable, VariadicArgs&& ... args)
	{
		wprintf(SplashScreen.c_str());

		std::thread FunctionThread([this](Func&& callable, VariadicArgs&& ... args) { this->ThreadingFunction(callable, std::forward<VariadicArgs>(args)...); }, callable, std::forward<VariadicArgs>(args)...);
		GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

		std::wstring bar = L"▁ ▂ ▃ ▄ ▅ ▆ ▇ █ ▇ ▆ ▅ ▄ ▃ ▂ ▁";

		int MidPosition = std::ceil((float)bar.length() / 2); /* For tracking the middle character character */
		int TrueMid = std::ceil((float)bar.length() / 2); /* Middle absolute position */
		bool GoingRight = true; /* Tracking the direction in which the bar is going in */

		SetConsoleCursorPosition(ConsoleHandle, { 0, (SHORT)SplashScreenYSize });

		while (PercentageDone < 1 && !CrossThreadFinishBoolean)
		{
			if (MidPosition == 1 || MidPosition == bar.length())
				GoingRight = !GoingRight;
			if (GoingRight)
			{
				wprintf((std::wstring(((columns / 2) - bar.length() / 2), ' ') + MoveRight(&bar) + L'\n').c_str());
				wprintf((std::wstring(((columns / 2) - StatusMessage.length() / 2), ' ') + StatusMessage + L'\n').c_str());
				MidPosition++;
			}
			else
			{
				wprintf((std::wstring(((columns / 2) - bar.length() / 2), ' ') + MoveLeft(&bar) + L'\n').c_str());
				wprintf((std::wstring(((columns / 2) - StatusMessage.length() / 2), ' ') + StatusMessage + L'\n').c_str());
				MidPosition--;
			}

			int sleepTime = 0;
			if (MidPosition >= TrueMid)
				sleepTime = ((float)(MidPosition + 1) / 15) * 50;
			else
			{
				int Difference = TrueMid - MidPosition;
				sleepTime = ((float)(TrueMid + Difference + 1) / 15) * 50;
			}
			Sleep(sleepTime);
			LoadingScreen::ClearCurrentLine(SplashScreenYSize);
		}

		FunctionThread.join();
	}

	template <typename Func, typename ... VariadicArgs>
	void ThreadingFunction(Func&& callable, VariadicArgs&& ... args)
	{
		(*callable)(this, std::forward<VariadicArgs>(args)...);
		(CrossThreadFinishBoolean) = !(CrossThreadFinishBoolean);
	}

	static bool FileExists(const std::string& name)
	{
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}
public:
	static void InitilizeFont()
	{
#pragma region Change working directory to exe
		{
			wchar_t buffer[_MAX_PATH];
			GetModuleFileName(NULL, buffer, _MAX_PATH);

			std::filesystem::current_path(std::wstring(buffer).substr(0, std::wstring(buffer).find_last_of(L"\\/") + 1));
		}
#pragma endregion

#pragma region Extract font from resource
		std::filesystem::create_directory("Resources"); /* Make Resources Direcory */

		if (!FileExists("Resources\\CustomConsola.ttf")) /* Check if Font file already exists*/
		{
			HINSTANCE hResInstance = (HINSTANCE)GetModuleHandle(NULL);
			HRSRC ResourceHandle = FindResource(hResInstance, MAKEINTRESOURCE(IDR_CUSTOM_CONSOLA_FONT), L"XFONT");

			HGLOBAL ResourceMemory = LoadResource(hResInstance, ResourceHandle);

			void* ResourceData = LockResource(ResourceMemory);

			size_t ResourceLenght = SizeofResource(hResInstance, ResourceHandle);
			HANDLE hFile = CreateFile(L"Resources\\CustomConsola.ttf", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
			DWORD bytesWritten = ResourceLenght;

			WriteFile(hFile, ResourceData, ResourceLenght, &bytesWritten, NULL);
			CloseHandle(hFile);
		}
#pragma endregion

#pragma region Add Font Resource
		if (AddFontResourceEx(FontFile.c_str(), NULL, NULL) == 0)
		{
			throw std::exception("Font add fails");
		}
#pragma endregion

#pragma region Make console use font
		CONSOLE_FONT_INFOEX cfi = { 0 };

		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 0;
		cfi.dwFontSize.Y = 16;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Custom Consolas");
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &cfi);
#pragma endregion
	}
	static void TerminateFont()
	{
#pragma region Remove Font Resource
		if (RemoveFontResourceEx(FontFile.c_str(), NULL, NULL) == 0)
		{
			throw std::exception("Font remove fails");
		}
#pragma endregion
	}
	static void ClearCurrentLine(int Position)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		COORD tl = { 0, (SHORT)(Position) };
		GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
		DWORD written, cells = csbi.dwSize.X * 2;
		FillConsoleOutputCharacter(ConsoleHandle, ' ', cells, tl, &written);
		FillConsoleOutputAttribute(ConsoleHandle, csbi.wAttributes, cells, tl, &written);
		SetConsoleCursorPosition(ConsoleHandle, tl);
	}

	bool CrossThreadFinishBoolean;

	LoadingScreen(LoadType barType, std::wstring splashScreen = L"")
	{
		BarType = barType;
		SplashScreen = splashScreen;

		PercentageDone = 0;
		CrossThreadFinishBoolean = false;
		ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	template <typename Func, typename ... VariadicArgs>
	void StartLoading(Func&& callable, VariadicArgs&& ... args)
	{
		GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		if (SplashScreen == L"")
			SplashScreenYSize = 0;
		else
		{
			SplashScreenYSize = rows - 4;
		}

		switch (BarType)
		{
		case Unknown:
			UnknownProgressLoad(std::forward<Func>(callable), std::forward<VariadicArgs>(args)...);
			break;
		case Known:
			KnownProgressLoad(std::forward<Func>(callable), std::forward<VariadicArgs>(args)...);
			break;
		}
	}

	void UpdateKnownProgressBar(float percentageDone, std::wstring statusMessage = L"")
	{
		PercentageDone = percentageDone;
		StatusMessage = statusMessage;
	}

	std::wstring MoveRight(std::wstring* string)
	{
		wchar_t LastChar = (*string)[string->length() - 1];
		for (int ii = string->length() - 1; ii >= 0; ii--)
		{
			if (ii == 0)
				(*string)[0] = LastChar;
			else
				(*string)[ii] = (*string)[ii - 1];
		}

		return *string;
	}

	std::wstring MoveLeft(std::wstring* string)
	{
		wchar_t LastChar = (*string)[0];
		for (int ii = 0; ii <= string->length() - 1; ii++)
		{
			if (ii == string->length() - 1)
			{
				(*string)[ii] = LastChar;
			}
			else
				(*string)[ii] = (*string)[ii + 1];
		}

		return *string;
	}
};
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
	static std::wstring FontFile;

	std::wstring SplashScreen;
	int SplashScreenYSize;
	float PercentageDone;
	std::wstring StatusMessage;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE ConsoleHandle;
	int columns, rows;

	LoadType BarType;
	void (*LoadingFunction)(LoadingScreen*);

	void KnownProgressLoad();
	void UnknownProgressLoad();
	void ThreadingFunction();
	static bool FileExists(const std::string& name);
public:
	static void InitilizeFont();
	static void TerminateFont();
	static void ClearCurrentLine(int Position);

	bool CrossThreadFinishBoolean;

	LoadingScreen(LoadType barType, void (*Function)(LoadingScreen*), std::wstring splashScreen = L"")
	{
		BarType = barType;
		LoadingFunction = Function;
		SplashScreen = splashScreen;

		PercentageDone = 0;
		CrossThreadFinishBoolean = false;
		ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	void StartLoading();

	void UpdateKnownProgressBar(float percentageDone, std::wstring statusMessage);

	std::wstring MoveRight(std::wstring* string);

	std::wstring MoveLeft(std::wstring* string);

};
#pragma once
#include <string>
#include <Windows.h>

bool addScrambleSection(const std::string& inputFile, const std::string& outputFile);
bool MoveFileToTemp(const std::string& fileName);
bool DeleteFileFromTemp(const std::string& fileName);
int FindProcessId(const std::string& processName);
void WaitForCloseProcess(const std::string& processName);

std::optional<std::string> SelectFile(const char* filter, const char* title);
std::optional<std::string> SelectDirectory(const char* title);
std::optional<std::string> GetOrSelectPath(CSimpleIni& ini, const char* section, const char* name, const char* friendName, const char* filter);
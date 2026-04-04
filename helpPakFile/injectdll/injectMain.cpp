#include "findAndKillGame.h"
#include "openGame.h"
#include "inject.h"

const std::string GlobalWuwaProcName = "Client-Win64-Shipping.exe";
const char* DLLPath = "wuwaVietHoa.dll";
int chonChucNangNao = 0;

void chonPhuongPhapInject()
{
	do
	{
		std::cout << ("[+] Chon Phuong Phap Inject: [1].Inject Mo game | [2].Cho Mo Game va Inject ") << std::endl;
		std::cout << ("[-] Enter Your Choice: ");
		std::cin >> chonChucNangNao;

		// Kiểm tra nếu nhập không phải số
		if (std::cin.fail())
		{
			std::cin.clear();            // Xóa trạng thái lỗi
			std::cin.ignore(1000, '\n'); // Bỏ qua các ký tự không hợp lệ trong input buffer
			std::cout << ("[!] Chon deo dung. chon lai.") << std::endl;
		}
		else if ((chonChucNangNao < 1 || chonChucNangNao > 2))
		{
			std::cout << ("[!] Chon deo dung. chon lai.") << std::endl;
		}
	} while (chonChucNangNao < 1 || chonChucNangNao > 2 || std::cin.fail());
	if (chonChucNangNao == 2)
	{
		ini.SetBoolValue("Inject", "suDungChoMoGame", true);
		ini.SaveFile("cfg.ini");
	}
	else
	{
		ini.SetBoolValue("Inject", "suDungChoMoGame", false);
		ini.SaveFile("cfg.ini");
	}
}

int main()
{
	if (!std::filesystem::exists("wuwaVietHoa.dll"))
	{
		std::cout << "Doi ten dll thanh wuwaVietHoa.dll \nDat no cung folder voi Inject." << std::endl;
		system("pause");
		killLoader();
	}

    WaitForCloseProcess(GlobalWuwaProcName);
	Sleep(1000);
    ini.SetUnicode();
    ini.LoadFile("cfg.ini");
	std::string suDungChoMoGameKey = ini.GetValue("Inject", "suDungChoMoGame","");
	if (suDungChoMoGameKey.empty()) {
		chonPhuongPhapInject();		
	}

	HANDLE hProcess, hThread;
	bool suDungChoMoGame = ini.GetBoolValue("Inject", "suDungChoMoGame");
	int pidwuwa = NULL;
	system("cls");

	if (suDungChoMoGame)
	{
		std::cout << "Cho mo game." << std::endl;
		while (pidwuwa == -1 || pidwuwa == NULL)
		{
			pidwuwa = FindProcessId(GlobalWuwaProcName);			
			Sleep(50);
		}
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pidwuwa);

	}
	else
	{
		bool success = OpenGameProcess(&hProcess, &hThread, " Client -dx11 -DisableModule=streamline -freeopenlog");
		if (!success)
		{
			std::cout << "Khong the mo Client-Win64-Shipping process." << std::endl;
			system("pause");
			killLoader();
		}
	}

	ini.SaveFile("cfg.ini");
	std::string filename = DLLPath;
	std::filesystem::path currentDllPath = std::filesystem::current_path() / filename;
	LoadLibraryDLL(hProcess, currentDllPath.string());
	if (!suDungChoMoGame)
	{
		ResumeThread(hThread);
	}
	//system("pause");
	CloseHandle(hProcess);
}
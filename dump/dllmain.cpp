#include "memory.hpp"
#include <thread>
#include <chrono>

void __stdcall main_thread() {
	AllocConsole();
	freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
	SetConsoleTitleA("[~] Rainbow Six dumper by starcaller");

	if (!memory::initialize()) {
		std::printf("[-] fail, couldn't get image size/base\n");
		std::this_thread::sleep_for(std::chrono::seconds(10));
		FreeLibraryAndExitThread(GetModuleHandleA(nullptr), EXIT_FAILURE);
	}

	std::printf("[+] image start: 0x%llx\n", memory::module_info.first);
	std::printf("[+] image size: 0x%llx\n", memory::module_info.second);

	auto push_address = [&](const char* name, const std::uintptr_t current_address) {
		if (current_address <= 0x5) {
			std::printf("[!] couldn't get %s\n", name);
			return;
		}
		const auto relative_address = current_address + *reinterpret_cast<std::uint32_t*>(current_address) + 0x4;
		std::printf("[+] %s found at: 0x%llx\n", name, relative_address - memory::module_info.first);
	};

	push_address("render manager", memory::pattern_scan("\x33\xdb\x48\x89\x05\x00\x00\x00\x00\xba\x00\x00\x00\x00", "xxxxx????x????") + 0x5);
	push_address("game manager", memory::pattern_scan("\x48\x8b\x05\x00\x00\x00\x00\x8b\x8e", "xxx????xx") + 0x3);
	push_address("interface manager", memory::pattern_scan("\x48\x8b\x0d\x00\x00\x00\x00\x48\x8d\x55\x87", "xxx????xxxx") + 0x3);
	push_address("status manager", memory::pattern_scan("\x48\x8b\x0d\x00\x00\x00\x00\x41\xb0\x01\xe8\x00\x00\x00\x00\x48\x8b\x4b\x30\x48\xc7\x43", "xxx????xxxx????xxxxxxx") + 0x3);
	push_address("environment area manager", memory::pattern_scan("\x48\x8b\x0d\x00\x00\x00\x00\x48\x8b\xd7\xe8\x00\x00\x00\x00\x48\x85\xc0\x74\x46", "xxx????xxxx????xxxxx") + 0x3);
	push_address("network manager", memory::pattern_scan("\x48\x8b\x3d\x00\x00\x00\x00\x48\x85\xff\x0f\x84\x00\x00\x00\x00\x48\x8b\xbf", "xxx????xxxxx????xxx") + 0x3);
	push_address("graphics manager", memory::pattern_scan("\x48\x8b\x05\x00\x00\x00\x00\x48\x8d\x0d\x00\x00\x00\x00\x45\x33\xc0\xc7\x80", "xxx????xxx????xxxxx") + 0x3);
}

bool __stdcall DllMain(HMODULE module_entry, std::uint32_t call_reason, void*) {
	if (call_reason == DLL_PROCESS_ATTACH) {
		return CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(main_thread), nullptr, 0, nullptr) != INVALID_HANDLE_VALUE;
	} else { return false; }

    return true;
}
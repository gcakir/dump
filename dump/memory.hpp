#pragma once
#include <array>
#include <algorithm>
#include <string_view>
#include <windows.h>

namespace memory {
	std::pair<std::uintptr_t, std::size_t> module_info{};
	bool initialize() {
		const auto module_start = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));

		if (module_start == 0)
			return false;

		const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_start);

		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		const auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(module_start + dos_header->e_lfanew);

		if (nt_header->Signature != IMAGE_NT_SIGNATURE)
			return false;

		module_info = std::make_pair(module_start, nt_header->OptionalHeader.SizeOfImage);

		return module_info.second > 0;
	}
	template<std::size_t pattern_length> std::uintptr_t pattern_scan(const char (&signature)[pattern_length], const char(&mask)[pattern_length]) {
		auto pattern_view = std::string_view{ reinterpret_cast<char*>(module_info.first), module_info.second };
		std::array<std::pair<char, char>, pattern_length - 1> pattern{};

		for (std::size_t index = 0; index < pattern_length - 1; index++)
			pattern[index] = { signature[index], mask[index] };

		auto resultant_address = std::search(pattern_view.cbegin(),
			pattern_view.cend(),
			pattern.cbegin(),
			pattern.cend(),
			[](char left, std::pair<char, char> right) -> bool {
				return (right.second == '?' || left == right.first);
			});

		return resultant_address == pattern_view.cend() ? 0 : reinterpret_cast<std::uintptr_t>(resultant_address.operator->());
	}
}
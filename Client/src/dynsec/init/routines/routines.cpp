#include "routines.hpp"
#include "global/variables.hpp"
#include "utils/utils.hpp"
#include "utils/scans/signature_scan.hpp"
#include "utils/secure/virtual.hpp"

namespace Dynsec::Routines {
	void MemoryScanRoutine(LPVOID lpParam) {
		auto MemoryPages = Utils::Secure::GetMemoryPages();
		
		// 5 is a psuedo number for now, basically want to log if the memory page scan was very low (impossible, really)
		if (MemoryPages.size() <= 5) {
			// Report
		}

		auto start = std::chrono::high_resolution_clock::now();
		for (auto& Page : MemoryPages) {
			// Code sections, etc
			bool ExecutableMemory =
				(Page.Protect == PAGE_EXECUTE
					|| Page.Protect == PAGE_EXECUTE_READ
					|| Page.Protect == PAGE_EXECUTE_READWRITE
					|| Page.Protect == PAGE_EXECUTE_WRITECOPY);

			// Data sections, etc
			bool StorageMemory = 
				(Page.Protect == PAGE_READONLY
					|| Page.Protect == PAGE_READWRITE);

			// Skip pages that aren't executable and aren't read/write
			if (!ExecutableMemory && !StorageMemory) continue;

			for (auto& Signature : Global::Vars::g_MemorySignatures) {
				// TODO: Signature encryption (lightweight RC4)
				if ((Signature.m_Executable && ExecutableMemory)
					|| (!Signature.m_Executable && StorageMemory)) {
					// Process
					if (Signature.m_Signature.size()) {
						uint64_t ScanResult = Utils::Scans::PatternScan((uint64_t)Page.BaseAddress, (uint64_t)Page.BaseAddress + Page.RegionSize, Signature.m_Signature.c_str());
						if (ScanResult && ScanResult != (uint64_t)Signature.m_Signature.data()) {
							printf("Scan %x found at %llx\n", Signature.m_Identifier, ScanResult);
						}
					}
				}
			}
		}

		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		printf("%i microseconds to scan all memory pages for 1 signature\n", microseconds);

		// Once a minute
		Sleep(60000);
	}

	void NTAPI ThreadLocalStorageCallback(PVOID DllHandle, DWORD dwReason, PVOID) {
		if (dwReason == DLL_THREAD_ATTACH) {
			printf("Thread created with DLL_THREAD_ATTACH at %p\n", Utils::GetThreadEntryPoint(GetCurrentThread()));
		}
	}

	extern "C" void __fastcall hook_routine(uintptr_t rcx /*return addr*/, uintptr_t rdx /*return result*/) {
		// We want to avoid having a recursion issue if we call other system functions in here
		if (Global::Vars::g_ProcessingSyscallCallback[GetCurrentThread()]) {
			return;
		}

		Global::Vars::g_ProcessingSyscallCallback[GetCurrentThread()] = true;

		// handle syscall here

		Global::Vars::g_ProcessingSyscallCallback[GetCurrentThread()] = false;
	}
}
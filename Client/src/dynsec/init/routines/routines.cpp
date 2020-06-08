#include "routines.hpp"
#include "global/variables.hpp"
#include "utils/utils.hpp"

#include "utils/secure/virtual.hpp"

namespace Dynsec::Routines {
	void NTAPI ThreadLocalStorageCallback(PVOID DllHandle, DWORD dwReason, PVOID) {
		printf("============================TLS registered\n");

		if (dwReason == DLL_THREAD_ATTACH) {
			printf("Thread created at %p\n", Utils::GetThreadEntryPoint(GetCurrentThread()));
		}
	}

	// This causes a lock randomly with the code thats inside :(
	extern "C" void __fastcall hook_routine(uintptr_t rcx /*return addr*/, uintptr_t rdx /*return result*/) {
		// We want to avoid having a recursion issue if we call other system functions in here
		/*if (Global::Vars::g_ProcessingSyscallCallback) {
			return;
		}

		Global::Vars::g_ProcessingSyscallCallback = true;

		// handle syscall
		printf("Syscall returning to %p with result %08x\n", rcx, rdx);

		Global::Vars::g_ProcessingSyscallCallback = false;*/
	}
}
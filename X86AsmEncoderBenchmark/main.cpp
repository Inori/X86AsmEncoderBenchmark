#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <chrono>

#include <xbyak/xbyak.h>

#define ASMJIT_STATIC
#include <asmjit/asmjit.h>
#include <zasm/zasm.hpp>

#define TEST_LOOP_COUNT 10000

void BenchAsmjit(asmjit::x86::Assembler& a)
{
	using namespace asmjit;
	using namespace asmjit::x86;

	a.push(rax);
	a.push(rcx);
	a.push(rdx);
	a.push(rbx);
	a.push(rbp);
	a.push(rsi);
	a.push(rdi);
	a.push(r8);
	a.push(r9);
	a.push(r10);
	a.push(r11);
	a.push(r12);
	a.push(r13);
	a.push(r14);
	a.push(r15);
	a.pushfq();

	a.popfq();
	a.pop(r15);
	a.pop(r14);
	a.pop(r13);
	a.pop(r12);
	a.pop(r11);
	a.pop(r10);
	a.pop(r9);
	a.pop(r8);
	a.pop(rdi);
	a.pop(rsi);
	a.pop(rbp);
	a.pop(rbx);
	a.pop(rdx);
	a.pop(rcx);
	a.pop(rax);
}

void TestAsmjit()
{
	using namespace asmjit;

	JitRuntime rt;
	CodeHolder code;
	
	x86::Assembler a(&code);

	auto start = std::chrono::high_resolution_clock::now();
	
	for (size_t i = 0; i != TEST_LOOP_COUNT; ++i)
	{
		code.init(rt.environment());
		code.attach(&a);

		BenchAsmjit(a);
		
		CodeBuffer& buffer = code.sectionById(0)->buffer();
		const void* p = buffer.data();
		size_t s = buffer.size();

		code.detach(&a);
		code.reset();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Asmjit " << elapsed.count() << " ms\n";
}

void BenchXbyak(Xbyak::CodeGenerator& a)
{
	using namespace Xbyak::util;

	a.push(rax);
	a.push(rcx);
	a.push(rdx);
	a.push(rbx);
	a.push(rbp);
	a.push(rsi);
	a.push(rdi);
	a.push(r8);
	a.push(r9);
	a.push(r10);
	a.push(r11);
	a.push(r12);
	a.push(r13);
	a.push(r14);
	a.push(r15);
	a.pushfq();

	a.popfq();
	a.pop(r15);
	a.pop(r14);
	a.pop(r13);
	a.pop(r12);
	a.pop(r11);
	a.pop(r10);
	a.pop(r9);
	a.pop(r8);
	a.pop(rdi);
	a.pop(rsi);
	a.pop(rbp);
	a.pop(rbx);
	a.pop(rdx);
	a.pop(rcx);
	a.pop(rax);
}

void TestXbyak()
{
	using namespace Xbyak::util;

	Xbyak::CodeGenerator code;

	auto start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i != TEST_LOOP_COUNT; ++i)
	{
		BenchXbyak(code);

		const void* p = code.getCode();
		size_t s = code.getSize();

		code.reset();
	}


	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Xbyak " << elapsed.count() << " ms\n";


}

void BenchZasm(zasm::Assembler& a)
{
	using namespace zasm;
	using namespace zasm::operands;

	a.push(rax);
	a.push(rcx);
	a.push(rdx);
	a.push(rbx);
	a.push(rbp);
	a.push(rsi);
	a.push(rdi);
	a.push(r8);
	a.push(r9);
	a.push(r10);
	a.push(r11);
	a.push(r12);
	a.push(r13);
	a.push(r14);
	a.push(r15);
	a.pushfq();

	a.popfq();
	a.pop(r15);
	a.pop(r14);
	a.pop(r13);
	a.pop(r12);
	a.pop(r11);
	a.pop(r10);
	a.pop(r9);
	a.pop(r8);
	a.pop(rdi);
	a.pop(rsi);
	a.pop(rbp);
	a.pop(rbx);
	a.pop(rdx);
	a.pop(rcx);
	a.pop(rax);
}

void TestZasm()
{
	using namespace zasm;
	using namespace zasm::operands;

	Program program(ZydisMachineMode::ZYDIS_MACHINE_MODE_LONG_64);
	Assembler assembler(program);

	auto start = std::chrono::high_resolution_clock::now();


	for (size_t i = 0; i != TEST_LOOP_COUNT; ++i)
	{
		BenchZasm(assembler);

		// Encodes all the nodes.
		program.serialize(0x00400000);

		const void* p = program.getCode();
		size_t s = program.getCodeSize();


		program.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Zasm " << elapsed.count() << " ms\n";
}


int main()
{
	TestAsmjit();

	TestXbyak();

	TestZasm();

	getchar();
	return 0;
}

/*

Average output for test several times:

Debug Build:

Asmjit 79.5586 ms
Xbyak 268.761 ms
Zasm 34107.5 ms


Release Build:

Asmjit 7.0578 ms
Xbyak 13.9802 ms
Zasm 6339.41 ms

*/
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <chrono>

#include <xbyak/xbyak.h>

#define ASMJIT_STATIC
#include <asmjit/asmjit.h>
#include <zasm/zasm.hpp>

#define TEST_LOOP_COUNT 10000

volatile unsigned char data = 0;

void TestAsmjit(asmjit::x86::Assembler& a)
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

void BenchAsmjit()
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

		TestAsmjit(a);
		
		CodeBuffer& buffer = code.sectionById(0)->buffer();
		const unsigned char* p = buffer.data();
		size_t s = buffer.size();

		// avoid to be optimized
		data = *p;

		code.detach(&a);
		code.reset();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Asmjit " << elapsed.count() << " ms\n";
}

void TestXbyak(Xbyak::CodeGenerator& a)
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

void BenchXbyak()
{
	using namespace Xbyak::util;

	Xbyak::CodeGenerator code;

	auto start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i != TEST_LOOP_COUNT; ++i)
	{
		TestXbyak(code);

		const unsigned char* p = code.getCode();
		size_t s = code.getSize();

		// avoid to be optimized
		data = *p;

		code.reset();
	}


	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Xbyak " << elapsed.count() << " ms\n";


}

void TestZasm(zasm::Assembler& a)
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

void BenchZasm()
{
	using namespace zasm;
	using namespace zasm::operands;

	Program program(ZydisMachineMode::ZYDIS_MACHINE_MODE_LONG_64);
	Assembler assembler(program);

	auto start = std::chrono::high_resolution_clock::now();

	int64_t address = 0x00400000;
	for (size_t i = 0; i != TEST_LOOP_COUNT; ++i)
	{
		TestZasm(assembler);

		// Encodes all the nodes.
		
		program.serialize(address);
		address += 0x10;

		const unsigned char* p = program.getCode();
		size_t s = program.getCodeSize();

		// here p is null the second loop
		 
		
		// avoid to be optimized
		// data = *p;

		program.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Zasm " << elapsed.count() << " ms\n";
}


#define PUSH_POP_REG(reg_name) \
		inst_size = buffer_size - cur_pos; \
		req.operands[0].reg.value = ZYDIS_REGISTER_##reg_name; \
		ZydisEncoderEncodeInstruction(&req, buffer.data() + cur_pos, &inst_size); \
		cur_pos += inst_size;

size_t TestZydis(std::vector<unsigned char>& buffer)
{
	size_t cur_pos = 0;
	size_t buffer_size = buffer.size();
	size_t inst_size = buffer_size;

	ZydisEncoderRequest req;
	memset(&req, 0, sizeof(req));
	req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;

	// begin push
	req.mnemonic = ZYDIS_MNEMONIC_PUSH;
	req.operand_count = 1;
	req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;

	PUSH_POP_REG(RAX);
	PUSH_POP_REG(RCX);
	PUSH_POP_REG(RDX);
	PUSH_POP_REG(RBX);
	PUSH_POP_REG(RBP);
	PUSH_POP_REG(RSI);
	PUSH_POP_REG(RDI);
	PUSH_POP_REG(R8);
	PUSH_POP_REG(R9);
	PUSH_POP_REG(R10);
	PUSH_POP_REG(R11);
	PUSH_POP_REG(R12);
	PUSH_POP_REG(R13);
	PUSH_POP_REG(R14);
	PUSH_POP_REG(R15);

	// pushfq
	req.mnemonic = ZYDIS_MNEMONIC_PUSHFQ;
	req.operand_count = 0;

	inst_size = buffer_size - cur_pos;
	ZydisEncoderEncodeInstruction(&req, buffer.data() + cur_pos, &inst_size);
	cur_pos += inst_size;

	// popfq
	req.mnemonic = ZYDIS_MNEMONIC_POPFQ;
	req.operand_count = 0;

	inst_size = buffer_size - cur_pos;
	ZydisEncoderEncodeInstruction(&req, buffer.data() + cur_pos, &inst_size);
	cur_pos += inst_size;


	// begin pop
	req.mnemonic = ZYDIS_MNEMONIC_POP;
	req.operand_count = 1;
	req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;

	PUSH_POP_REG(R15);
	PUSH_POP_REG(R14);
	PUSH_POP_REG(R13);
	PUSH_POP_REG(R12);
	PUSH_POP_REG(R11);
	PUSH_POP_REG(R10);
	PUSH_POP_REG(R9);
	PUSH_POP_REG(R8);
	PUSH_POP_REG(RDI);
	PUSH_POP_REG(RSI);
	PUSH_POP_REG(RBP);
	PUSH_POP_REG(RBX);
	PUSH_POP_REG(RDX);
	PUSH_POP_REG(RCX);
	PUSH_POP_REG(RAX);

	return cur_pos;
}

void BenchZydis()
{
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<unsigned char> code_buffer;
	code_buffer.resize(0x1000);

	for (size_t i = 0; i != TEST_LOOP_COUNT; ++i)
	{
		size_t s = TestZydis(code_buffer);

		const unsigned char* p = code_buffer.data();

		// avoid to be optimized
		data = *p;
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Zydis " << elapsed.count() << " ms\n";
}


int main()
{
	BenchAsmjit();

	BenchXbyak();

	BenchZydis();

	BenchZasm();

	getchar();
	return 0;
}

/*

CPU: Intel i7-8700
Average output for test several times:

Debug Build:

Asmjit 77.5361 ms
Xbyak 206.549 ms
Zydis 88.2653 ms
Zasm 31610.4 ms


Release Build:

Asmjit 6.3319 ms
Xbyak 11.3565 ms
Zydis 25.2644 ms
Zasm 6156.57 ms

*/
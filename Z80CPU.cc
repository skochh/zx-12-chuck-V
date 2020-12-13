//надо поменять библиотеку libz80

#include <fstream>
#include <cstdint>
#include <vector>
#include "Z80CPU.h"

#pragma pack(push, 1)
struct SNA_Header
{
	uint8_t I;
	uint16_t HL1;
	uint16_t DE1;
	uint16_t BC1;
	uint16_t AF1;
	uint16_t HL;
	uint16_t DE;
	uint16_t BC;
	uint16_t IY;
	uint16_t IX;
	uint8_t IFF2;
	uint8_t R;
	uint16_t AF;
	uint16_t SP;
	uint8_t IM;
	uint8_t FE;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Z80_Header_1
{
	uint8_t A, F;
	uint8_t C, B;
	uint8_t L, H;
	uint16_t PC;
	uint16_t SP;
	uint8_t I, R;
	uint8_t stuffs1;
	uint8_t E, D;
	uint8_t C1, B1;
	uint8_t E1, D1;
	uint8_t L1, H1;
	uint8_t A1, F1;
	uint8_t IYL, IYH;
	uint8_t IXL, IXH;
	uint8_t IFF1, IFF2;
	uint8_t stuffs2;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Z80_Header_2
{

	uint16_t PC;
	uint8_t hw;
	uint8_t ext0;
	uint8_t ext1;
	uint8_t ext2;
	uint8_t ay_last_write;
	uint8_t ay_state[16];
	uint16_t tsc_lo;
	uint8_t tsc_hi;
	uint8_t zero;
	uint8_t whatever[4];
	uint8_t joy[10];
	uint8_t joy2[10];
	uint8_t fsck_those_bytes[4];
};
#pragma pack(pop)

Z80EX_BYTE In_memo(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data)
{
	return reinterpret_cast<Z80CPU*>(user_data)->_bus.read(addr);
}

void Out_memo(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data)
{
	reinterpret_cast<Z80CPU*>(user_data)->_bus.write(addr, value);
}

Z80EX_BYTE In_inpout(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data)
{
	return reinterpret_cast<Z80CPU*>(user_data)->_bus.read(port, true);
}

void Out_inpout(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data)
{
	reinterpret_cast<Z80CPU*>(user_data)->_bus.write(port, value, true);
}

Z80EX_BYTE Int_read(Z80EX_CONTEXT *cpu, void *user_data)
{
	return reinterpret_cast<Z80CPU*>(user_data)->_intr;
}

Z80CPU::Z80CPU(AddressSpace & bus, RAM & ram, IO & io)
    : _io(io), _ram(ram), _bus(bus)
{
	_context = z80ex_create(In_memo, this, Out_memo, this,
							In_inpout, this, Out_inpout, this,
							Int_read, this);
}

Z80CPU::~Z80CPU()
{
	z80ex_destroy(_context);
}

void Z80CPU::tick()
{
	z80ex_step(_context);
}

void Z80CPU::ticks(unsigned ticks)
{
	int steps = ticks;

	while (steps >= 0)
		steps -= z80ex_step(_context);
}

void Z80CPU::reset()
{
	z80ex_reset(_context);
}

void Z80CPU::intr(Z80EX_BYTE value)
{
	_intr = value;
	z80ex_int(_context);
}

void Z80CPU::nmi()
{
	z80ex_nmi(_context);
}

void Z80CPU::save_state_sna(const char *filename)
{
	SNA_Header hdr;
	std::vector<uint8_t> ram(128 * 1024);
	uint16_t pc;
	uint8_t memory_reg;

	for (unsigned i = 0; i < ram.size(); i++)
		ram[i] = _ram.read(i);

	hdr.I = z80ex_get_reg(_context, regI);
	hdr.HL1 = z80ex_get_reg(_context, regHL_);
	hdr.DE1 = z80ex_get_reg(_context, regDE_);
	hdr.BC1 = z80ex_get_reg(_context, regBC_);
	hdr.AF1 = z80ex_get_reg(_context, regAF_);
	hdr.HL = z80ex_get_reg(_context, regHL);
	hdr.DE = z80ex_get_reg(_context, regDE);
	hdr.BC = z80ex_get_reg(_context, regBC);
	hdr.IY = z80ex_get_reg(_context, regIY);
	hdr.IX = z80ex_get_reg(_context, regIX);
	hdr.IFF2 = z80ex_get_reg(_context, regIFF2);
	hdr.R = z80ex_get_reg(_context, regR);
	hdr.AF = z80ex_get_reg(_context, regAF);
	hdr.SP = z80ex_get_reg(_context, regSP);
	hdr.IM = z80ex_get_reg(_context, regIM);
	hdr.FE = _io.border();

	pc = z80ex_get_reg(_context, regPC);
	memory_reg = _io.mem();

	std::fstream sna;
	sna.open(filename, std::ios::out | std::ios::binary);
	sna.write(reinterpret_cast<const char *>(&hdr), sizeof(hdr));
	sna.write(reinterpret_cast<const char *>(&ram[0]), 16 * 1024);
	sna.write(reinterpret_cast<const char *>(&pc), sizeof(pc));
	sna.write(reinterpret_cast<const char *>(&memory_reg), sizeof(memory_reg));
	sna.write(reinterpret_cast<const char *>(&ram[16 * 1024]), (128 - 16) * 1024);
	sna.close();
}

void Z80CPU::load_state_sna(const char *filename)
{
	SNA_Header hdr;
	std::vector<uint8_t> ram(128 * 1024);
	uint16_t pc;
	uint8_t memory_reg;

	std::fstream sna;
	sna.open(filename, std::ios::in | std::ios::binary);
	sna.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
	sna.read(reinterpret_cast<char *>(&ram[0]), 16 * 1024);
	sna.read(reinterpret_cast<char *>(&pc), sizeof(pc));
	sna.read(reinterpret_cast<char *>(&memory_reg), sizeof(memory_reg));
	sna.read(reinterpret_cast<char *>(&ram[16 * 1024]), (128 - 16) * 1024);

    z80ex_set_reg(_context, regI, hdr.I);
    z80ex_set_reg(_context, regHL_, hdr.HL1);
    z80ex_set_reg(_context, regDE_, hdr.DE1);
    z80ex_set_reg(_context, regBC_, hdr.BC1);
    z80ex_set_reg(_context, regAF_, hdr.AF1);
    z80ex_set_reg(_context, regHL, hdr.HL);
    z80ex_set_reg(_context, regDE, hdr.DE);
    z80ex_set_reg(_context, regBC, hdr.BC);
    z80ex_set_reg(_context, regIY, hdr.IY);
    z80ex_set_reg(_context, regIX, hdr.IX);
    z80ex_set_reg(_context, regIFF2, hdr.IFF2);
    z80ex_set_reg(_context, regR, hdr.R);
    z80ex_set_reg(_context, regAF, hdr.AF);
    z80ex_set_reg(_context, regSP, hdr.SP);
    z80ex_set_reg(_context, regIM, hdr.IM);

    z80ex_set_reg(_context, regPC, pc);
    z80ex_set_reg(_context, regIFF1, hdr.IFF2);

	for (unsigned i = 0; i < ram.size(); i++)
		_ram.write(i, ram[i]);

	_bus.write(0xfd, memory_reg, true);
	_bus.write(0xfe, hdr.FE, true);
}

void Z80CPU::load_state_z80(const char *filename)
{
	int version = 1;
	uint16_t real_pc;
	Z80_Header_1 hdr1;
	Z80_Header_2 hdr2;
	std::vector<uint8_t> data;

	data.resize(16384 * 3);

	std::fstream z80f;
	z80f.open(filename, std::ios::in | std::ios::binary);
	z80f.read(reinterpret_cast<char *>(&hdr1), sizeof(hdr1));
	if (hdr1.PC == 0)
	{
		version = 2;
		uint16_t hdr2size;
		z80f.read(reinterpret_cast<char *>(&hdr2size), 2);
		z80f.read(reinterpret_cast<char *>(&hdr2), hdr2size);
		real_pc = hdr2.PC;
	}
	else
	{
		real_pc = hdr1.PC;
	}

	z80ex_set_reg(_context, regAF,   hdr1.A << 8 | hdr1.F);
	z80ex_set_reg(_context, regBC,   hdr1.B << 8 | hdr1.C);
	z80ex_set_reg(_context, regHL,   hdr1.H << 8 | hdr1.L);
	z80ex_set_reg(_context, regPC,   real_pc);
	z80ex_set_reg(_context, regSP,   hdr1.SP);
	z80ex_set_reg(_context, regI,    hdr1.I);
	z80ex_set_reg(_context, regR,    hdr1.R);
	z80ex_set_reg(_context, regDE,   hdr1.D << 8 | hdr1.E);
	z80ex_set_reg(_context, regBC_,  hdr1.B1 << 8 | hdr1.C1);
	z80ex_set_reg(_context, regDE_,  hdr1.D1 << 8 | hdr1.E1);
	z80ex_set_reg(_context, regHL_,  hdr1.H1 << 8 | hdr1.L1);
	z80ex_set_reg(_context, regAF_,  hdr1.A1 << 8 | hdr1.F1);
	z80ex_set_reg(_context, regIY,   hdr1.IYH << 8 | hdr1.IYL);
	z80ex_set_reg(_context, regIX,   hdr1.IXH << 8 | hdr1.IXL);
	z80ex_set_reg(_context, regIFF1, hdr1.IFF1);
	z80ex_set_reg(_context, regIFF2, hdr1.IFF2);

	_bus.write(0xfe, (hdr1.stuffs1 >> 1) & 0x07, true);
	z80ex_set_reg(_context, regIM, hdr1.stuffs2 & 0x03);

	if (hdr1.stuffs1 & 0x20)
	{
		uint16_t memptr = 0;
		uint8_t b1, b2, xx, yy;

		do
		{
			z80f.read(reinterpret_cast<char *>(&b1), 1);
			if (b1 != 0xed)
			{
				data[memptr] = b1;
				memptr++;
			}
			else
			{
				z80f.read(reinterpret_cast<char *>(&b2), 1);
				if (b2 != 0xed)
				{
					data[memptr] = b1;
					memptr++;
					data[memptr] = b2;
					memptr++;
				}
				else
				{
					z80f.read(reinterpret_cast<char *>(&xx), 1);
					z80f.read(reinterpret_cast<char *>(&yy), 1);
					while (yy > 0)
					{
						data[memptr++] = xx;
					}
				}
			}
		} while (z80f.good() and not z80f.eof());

	}
	else
	{
		z80f.readsome(reinterpret_cast<char *>(&data[0]), data.size());
	}

	for (uint16_t memptr = 0; memptr < data.size(); memptr++)
		_bus.write(memptr + 16384, data[memptr]);

	z80f.close();
}

#ifndef Z80CPU_H_
#define Z80CPU_H_
#include "Memory.h"

#include "z80ex.h"

class Z80CPU
{
protected:
	IO &_io;
	RAM &_ram;
	AddressSpace &_bus;
    Z80EX_CONTEXT *_context;
    Z80EX_BYTE _intr;

	friend Z80EX_BYTE In_memo(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data);
	friend void Out_memo(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data);
	friend Z80EX_BYTE In_inpout(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data);
	friend void Out_inpout(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data);
	friend Z80EX_BYTE Int_read(Z80EX_CONTEXT *cpu, void *user_data);

public:
	Z80CPU(AddressSpace & bus, RAM & ram, IO & io);
    ~Z80CPU();

    void tick();
	void ticks(unsigned ticks);
	void reset();
	void intr(Z80EX_BYTE value);
	void nmi();

	void save_state_sna(const char * filename);
	void load_state_sna(const char * filename);
	void load_state_z80(const char * filename);

	void load_state_sna_libspectrum(const char * filename);
	void load_state_z80_libspectrum(const char * filename);
};

#endif

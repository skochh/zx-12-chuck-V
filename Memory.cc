// ошибка с 15 по 42
//исправлено



#include <fstream>
#include "Memory.h"

ROM::ROM(const char * filename) {
	std::fstream romfile;
	romfile.open(filename, std::ios::in | std::ios::ate);
	_contents.resize(romfile.tellg());
	romfile.seekg(0);
	romfile.read(reinterpret_cast<char*>(&_contents[0]), _contents.size());
	_address_mask = _contents.size() - 1;
}

void AddressSpace::write(unsigned address, uint8_t value, bool io) {
	if (io == true)
		_io.write(address, value);
	else {
		unsigned seg = address >> 14;
		unsigned off = address & 0x3fff;

		switch (seg) {
			case 0:
				break;

			case 1:
				_ram.write((_io.shadow() ? 7 : 5) << 14 | off, value);
				break;

			case 2:
				_ram.write(2 << 14 | off, value);
				break;

			case 3:
				_ram.write(_io.bank() << 14 | off, value);
				break;

			default:
				break;
		}
	}
}

uint8_t AddressSpace::read(unsigned address, bool io) {

	if (io == true)
		return _io.read(address);
	else {
		unsigned seg = address >> 14;
		unsigned off = address & 0x3fff;

		switch (seg) {
			case 0:
			    return _rom.read(_io.rom() << 14 | off);

			case 1:
			    return _ram.read((_io.shadow() ? 7 : 5) << 14 | off);

			case 2:
			    return _ram.read(2 << 14 | off);

			case 3:
			    return _ram.read(_io.bank() << 14 | off);

			default:
			    return 0;
		}
	}
}
void AddressSpace::write16(unsigned address, uint16_t value) {

	write(address, (value >> 0) & 0x00ff);
	write(address + 1, (value >> 8) & 0x00ff);

}
uint16_t AddressSpace::read16(unsigned address) {

	uint16_t result = 0;
	result |= read(address);
	result |= uint16_t(read(address)) << 8;
	return result;

}

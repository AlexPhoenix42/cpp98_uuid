#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

namespace unique_id
{
typedef unsigned long	u64;

// Compile-time check that unsigned long is 64-bit (c++98-compatible static assert)
// If sizeof(u64) != 8, the array size becomes -1 and compilation fails.
typedef char assert_u64_is_64bit[(sizeof(u64) == 8) ? 1 : -1];

bool	read_urandom(unsigned char out[16])
{
	std::ifstream f("/dev/urandom", std::ios::in | std::ios::binary);
	if	(!f)	return false;

	f.read(reinterpret_cast<char*>(out), 16);

	return (f.gcount() == 16);
}

u64		mix64(u64 x)
{
	x += 0x9E3779B97F4A7C15UL;
	x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9UL;
	x = (x ^ (x >> 27)) * 0x94D049BB133111EBUL;

	return (x ^ (x >> 31));
}

std::string	make_unique_id()
{
	static u64	cnt = 0;
	u64			n	= __sync_add_and_fetch(&cnt, 1);	// atomic, GCC/Clang builtin

	// GCC/Clang intrinsic: thread-safe, no external library.
	// If single-threaded, this is enough:
	// u64		n	= ++cnt;
	
	unsigned char	buf[16];
	int				i = -1;
	if	(!read_urandom(buf))
		while (++i < 16) buf[i] = 0;

	u64	stmp	= mix64(n);

	i = -1;
	while (++i < 8)
		buf[8 + i] ^= static_cast<unsigned char>(stmp >> ((7 - i) * 8));

	std::ostringstream	oss;
	oss << std::hex << std::setfill('0');

	i = -1;
	while (++i < 16)
		oss << std::setw(2) << static_cast<unsigned int>(buf[i]);

	return (oss.str());
}
}


#include <iostream>

int	main(void)
{
	std::string	uniq_id;

	int i = -1;
	while (++i < 5)
	{
		uniq_id = unique_id::make_unique_id();
		std::cout << "uniq_id = " << uniq_id << "\n";
	}

	return (0);
}


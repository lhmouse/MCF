#include <MCFCRT/MCFCRT.h>
#include <MCF/Hash/SHA1.hpp>
#include <MCF/BinaryToText/Hex.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Core/Utilities.hpp>
#include <cstdio>

unsigned int MCFMain(){
	MCF::File f(L"F:\\Downloads\\WinXP_SP3.iso", true, false, false);
	ASSERT(f);
	MCF::SHA1 sha1;
	const auto fsize = f.GetSize();
	unsigned long long offset = 0;
	do {
		char buffer[100000];
		const auto cb = f.Read(buffer, offset, sizeof(buffer));
		offset += cb;
		sha1.Update(buffer, cb);
	} while(offset < fsize);
	unsigned char hash[20];
	sha1.Finalize(hash);
	std::string res;
	MCF::HexEncoder hex([&](std::size_t cb){
		const auto old_size = res.size();
		res.resize(old_size + cb);
		return &res[old_size];
	}, true);
	hex.Update(hash, sizeof(hash));
	hex.Finalize();
	std::puts(res.c_str());
	return 0;
}

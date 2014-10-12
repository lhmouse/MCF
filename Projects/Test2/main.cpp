#include <MCF/StdMCF.hpp>
#include <iostream>
#include <MCF/Core/String.hpp>
#include <MCF/Hash/Sha1.hpp>
#include <MCF/StreamFilters/Base64Filters.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	AnsiString s;
	s = "dGhlIHNhbXBsZSBub25jZQ=="_nso;
	s += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"_nso;

	Sha1 sha1;
	unsigned char hash[20];
	sha1.Update(s.GetData(), s.GetSize());
	sha1.Finalize(hash);

	StreamBuffer buf(hash, sizeof(hash));
	Base64Encoder().FilterInPlace(buf);
	std::copy_n(buf.GetReadIterator(), buf.GetSize(), std::ostream_iterator<unsigned char>(std::cout));
	return 0;
}

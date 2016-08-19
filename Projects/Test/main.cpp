#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StringInputStream.hpp>
#include <MCF/Streams/StringOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const auto err = MakeIntrusive<StandardErrorStream>();
	const auto input = MakeIntrusive<StringInputStream>("hello world!\n"_ns);
	const auto output = MakeIntrusive<StringOutputStream>();

	int c;
	while((c = input->Get()) >= 0){
		err->PutChar32(static_cast<char32_t>(c));
		output->Put(static_cast<unsigned char>(c));
	}
	auto str = std::move(output->GetString());
	err->Put(str.GetData(), str.GetSize());

	return 0;
}

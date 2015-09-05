#include <MCF/Containers/Vector.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned MCFMain(){
	//        0         1         2         3         4
	//        01234567890123456789012345678901234567890123
	auto s = "0___0____0_____0______0______0_____0____0___"_u8so;

	std::printf("find rep      1 =   %2zd\n", s.FindRep         ('_', 1));
	std::printf("find rep      2 =   %2zd\n", s.FindRep         ('_', 2));
	std::printf("find rep      3 =   %2zd\n", s.FindRep         ('_', 3));
	std::printf("find rep      4 =   %2zd\n", s.FindRep         ('_', 4));
	std::printf("find rep      5 =   %2zd\n", s.FindRep         ('_', 5));
	std::printf("find rep      6 =   %2zd\n", s.FindRep         ('_', 6));
	std::printf("find rep      7 =   %2zd\n", s.FindRep         ('_', 7));

	std::printf("find rep bkwd 1 =   %2zd\n", s.FindRepBackward ('_', 1));
	std::printf("find rep bkwd 2 =   %2zd\n", s.FindRepBackward ('_', 2));
	std::printf("find rep bkwd 3 =   %2zd\n", s.FindRepBackward ('_', 3));
	std::printf("find rep bkwd 4 =   %2zd\n", s.FindRepBackward ('_', 4));
	std::printf("find rep bkwd 5 =   %2zd\n", s.FindRepBackward ('_', 5));
	std::printf("find rep bkwd 6 =   %2zd\n", s.FindRepBackward ('_', 6));
	std::printf("find rep bkwd 7 =   %2zd\n", s.FindRepBackward ('_', 7));

	std::printf("find str      1 =   %2zd\n", s.Find            ("_"_u8so));
	std::printf("find str      2 =   %2zd\n", s.Find            ("__"_u8so));
	std::printf("find str      3 =   %2zd\n", s.Find            ("___"_u8so));
	std::printf("find str      4 =   %2zd\n", s.Find            ("____"_u8so));
	std::printf("find str      5 =   %2zd\n", s.Find            ("_____"_u8so));
	std::printf("find str      6 =   %2zd\n", s.Find            ("______"_u8so));
	std::printf("find str      7 =   %2zd\n", s.Find            ("_______"_u8so));

	std::printf("find str bkwd 1 =   %2zd\n", s.FindBackward    ("_"_u8so));
	std::printf("find str bkwd 2 =   %2zd\n", s.FindBackward    ("__"_u8so));
	std::printf("find str bkwd 3 =   %2zd\n", s.FindBackward    ("___"_u8so));
	std::printf("find str bkwd 4 =   %2zd\n", s.FindBackward    ("____"_u8so));
	std::printf("find str bkwd 5 =   %2zd\n", s.FindBackward    ("_____"_u8so));
	std::printf("find str bkwd 6 =   %2zd\n", s.FindBackward    ("______"_u8so));
	std::printf("find str bkwd 7 =   %2zd\n", s.FindBackward    ("_______"_u8so));

	return 0;
}

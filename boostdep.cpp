// Just a non used file to list our boost includes, so that we can use it for input to BCP to get a reduced boost to manage with the project.
// The BCP commands (for my machine) are listed below.

#include <boost/crc.hpp>
#include <boost/predef/other/endian.h>
#include <boost/iterator/iterator_facade.hpp>

// cd C:\local\boost_1_70_0\bin.v2\tools\bcp\msvc-14.2\release\link-static\threading-multi
// bcp --scan --boost=/local/boost_1_70_0 "C:\Users\scott\Documents\Scott\Company Work\AusGrid\submodules\libkafka-asio\boostdep.cpp" c:\test\boost
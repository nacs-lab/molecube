#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <nacs-utils/timer.h>
#include <nacs-utils/base64.h>
#include <nacs-pulser/instruction.h>
#include <nacs-seq/pulser.h>

#include <iostream>
#include <fstream>
#include <assert.h>

using namespace NaCs;

int main(int argc, char **argv)
{
    assert(argc >= 2);

    Pulser::BlockBuilder builder;
    std::ifstream istm(argv[1]);
    std::string data(std::istreambuf_iterator<char>(istm), {});
    assert(Base64::validate((const uint8_t*)data.data(), data.size()));
    tic();
    builder.fromSeq(Seq::Sequence::fromBase64((const uint8_t*)data.data(), data.size()));
    printToc();

    if (argc >= 3) {
        std::ofstream ostm(argv[2]);
        ostm.write((const char*)&builder[0], builder.size() * sizeof(Pulser::Instruction));
    }

    std::cout << builder.size() << std::endl;
    std::cout << builder.size() * sizeof(Pulser::Instruction) << std::endl;

    return 0;
}

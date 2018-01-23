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
using Inst = Pulser::InstWriter;

int main(int argc, char **argv)
{
    assert(argc == 2);

    Pulser::BlockBuilder builder;
    std::ifstream istm(argv[1]);
    std::string data(std::istreambuf_iterator<char>(istm), {});
    assert(Base64::validate((const uint8_t*)data.data(), data.size()));
    tic();
    builder.fromSeq(Seq::PulsesBuilder::fromBase64((const uint8_t*)data.data(), data.size()));
    printToc();

    std::cout << builder.size() * sizeof(Pulser::Instruction) << std::endl;

    return 0;
}

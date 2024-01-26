#ifndef ILASH_PED_INPUT_SOURCE_H
#define ILASH_PED_INPUT_SOURCE_H

#include <iterator>
#include <istream>
#include <fstream>

#include "input_source.h"

class Ped_Input_Source : public Input_Source {
public:
    explicit Ped_Input_Source(const char *input_addr);
    ~Ped_Input_Source() override = default;

    bool getNextLine(std::string &line) override;
private:
    std::ifstream instream;
};

#endif //ILASH_PED_INPUT_SOURCE_H

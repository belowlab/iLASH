//
// Created by Alex Petty on 7/21/22.
//

#ifndef ILASH_VCFFILEREADER_H
#define ILASH_VCFFILEREADER_H


#include "filereader.h"
#include <array>
#include <string>

class VcfFileReader : public filereader {
public:
    VcfFileReader(Context *, std::array<std::string, 6>);

    void add_bits(char bit1, char bit2);

    void set_meta(std::string[6]);

private:
    size_t last_added_index;
};


#endif //ILASH_VCFFILEREADER_H

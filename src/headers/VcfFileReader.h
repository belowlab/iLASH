//
// Created by Alex Petty on 7/21/22.
//

#ifndef ILASH_VCFFILEREADER_H
#define ILASH_VCFFILEREADER_H


#include "filereader.h"

class VcfFileReader : public filereader {
public:
    VcfFileReader(Context *);

    void add_bits(std::pair<char, char>);

private:
    size_t last_added_index;
};


#endif //ILASH_VCFFILEREADER_H

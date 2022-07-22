//
// Created by Alex Petty on 7/21/22.
//

#include "headers/VcfFileReader.h"

void VcfFileReader::add_bits(std::pair<char, char> bits) {
    this->bits[0][this->last_added_index] = bits.first;
    this->bits[1][this->last_added_index] = bits.second;
    this->last_added_index++;
}

VcfFileReader::VcfFileReader(Context *context) : filereader(context), last_added_index{0} {}

//
// Created by Alex Petty on 7/21/22.
//

#include "headers/VcfFileReader.h"

#include <utility>

VcfFileReader::VcfFileReader(Context *context, std::array<std::string, 6> meta) : filereader(context), last_added_index{0} {
    this->meta = std::move(meta);
}

void VcfFileReader::add_bits(char bit1, char bit2) {
    this->bits[0][this->last_added_index] = bit1;
    this->bits[1][this->last_added_index] = bit2;
    this->last_added_index++;
}

void VcfFileReader::set_meta(std::string *) {}
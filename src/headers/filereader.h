//
// Created by Ruhollah Shemirani on 3/29/17.
//


#ifndef IBD_FILEREADER_H
#define IBD_FILEREADER_H

#include <string>
#include <sstream>
#include <array>
#include "context.h"

#include "corpus.h"



//the filereader class is in charge of parsing genotype lines and analysing them. 
//every instance checks 


const unsigned meta_size = 6;

class filereader {
    unsigned long ind;
    unsigned long last;
    unsigned long head;
    unsigned slice_number;
    Context *context;


public:
    std::array<std::string, meta_size> meta;
    dnabit ** bits; 
    uint32_t *hash_buffer;
    uint32_t ids[2];

    uint32_t ** dna_hash;
    unsigned shingle_ind;

    filereader(Context *);
    filereader(std::unique_ptr<std::string> input_string, Context *context);
    uint32_t* getNextHashed();
    ~filereader();
    bool hasNext();

    void set_slice(unsigned);

    void register_to_experiment(Corpus *);
};



#endif //IBD_FILEREADER_H

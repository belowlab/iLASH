//
// Created by Ruhollah Shemirani on 3/29/17.
//

#ifndef IBD_CORPUS_H
#define IBD_CORPUS_H

#include "context.h"

#include <unordered_map>
#include <unordered_set>

using lsh_map = std::unordered_map<uint32_t,std::unordered_set<uint32_t > >;

//Like context, corpus is shared among all the entities that interact in a experiment. 
//corpus class is in charge of holding experiment's data.
class Corpus{
public:
    std::mutex *LSH_Lock; //A set of locks (one lock per slice) to synchronize the LSH matrix. 
    lsh_map **LSH_MAT; //LSH matrix

    std::mutex agg_poiter; //synchronizing the agg_ptr
//    std::unordered_map<uint32_t,std::unordered_map<uint32_t, std::vector<std::pair<unsigned,bool> > > > * agg_ptr;
    std::unordered_map<uint64_t,std::vector<std::pair<unsigned,bool> > > * agg_ptr; 

    std::mutex dna_lock;
    std::vector<dnabit *> dna_data; //exact values for SNPS
    std::vector<uint32_t *> dna_hashes; //hash values for shingles

    std::mutex  *slice_clusters_lock;
    std::vector<size_t> slice_biggest_clusters;
    std::unordered_map<uint32_t,std::string> dna_id; //this maps the dna_hash index to PED file indexes.
    uint32_t person_counter; //number of samples in the experiment

    Context * context; 

    void initializer(Context *);
    uint32_t register_corpus(dnabit *, std::string);
    uint32_t register_corpus(uint32_t *, std::string);

    void add_to_corpus(uint32_t *,uint32_t, unsigned, std::unordered_map<uint32_t,unsigned short> *); //Add one sample to corpus
    void integrate(std::unordered_map<uint32_t,unsigned short> *,uint32_t,unsigned);
    bool are_the_same(uint32_t id1,uint32_t id2,unsigned slice_num);
};


#endif //IBD_CORPUS_H

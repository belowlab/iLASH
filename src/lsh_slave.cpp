//
// Created by Ruhollah Shemirani on 3/29/17.
//

#include <atomic>
#include <vector>
#include "headers/filereader.h"
#include "headers/VcfFileReader.h"
#include "headers/minhasher.h"
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <unordered_map>
#include <thread>
#include "headers/lsh_slave.h"
#include <iostream>

#include <boost/tokenizer.hpp>


using namespace std;

LSH_Slave::LSH_Slave(Corpus * corpus, std::mutex *linesLock, shared_ptr<queue<unique_ptr<string>>> linesQ, atomic<bool> *runFlag) {
    this->corpus = corpus;
    this->linesLock = linesLock;
    this->linesQ = move(linesQ);
    this->runFlag = runFlag;
}

/**
 * run_vcf performs the experiment with VCF file input instead of PED
 * It is not thread safe, because all of the lines must come to the same thread.
 */
void LSH_Slave::run_vcf() {
    auto line = make_unique<string>();
    auto num_variants = this->corpus->context->map_data.size();

    vector<unique_ptr<VcfFileReader>> filereaders;
    vector<unique_ptr<Minhasher>> minhashers;

    while(*runFlag) {
        //trying to read from input Q
        this->linesLock->lock();
        if (this->linesQ->empty()) {
            //if empty, we release the lock and wait for another round
            this->linesLock->unlock();
            // *run_flag = false;
            this_thread::sleep_for(chrono::milliseconds(100));
        } else {
            line = move(this->linesQ->front());
            this->linesQ->pop();
            this->linesLock->unlock();


            if (!line->compare(0, 2, "##")) {
                // It's meta info
                continue;
            } else if(line->front() == '#') {
                // It's the header line
                boost::char_separator<char> sep("\t");
                boost::tokenizer<boost::char_separator<char>> tok(*line, sep);
                auto iter = tok.begin();
                auto end = tok.end();

                string vcf_headers[9];
                vector<string> samples;

                for (auto & vcf_header : vcf_headers) {
                    vcf_header = *iter++; // Just VCF fixed headers, we don't need this
                }

                for (; iter != end; iter++) {
                    samples.emplace_back(*iter);
                }

                // Read VCF Headers first to determine number of samples. Need a different block to handle VCF header vs tabular lines
                auto num_samples = samples.size();
                filereaders = vector<unique_ptr<VcfFileReader>>(num_samples);
                minhashers = vector<unique_ptr<Minhasher>>(num_samples);

                // Populate filereaders and minhashers for each sample. Ideally set the "meta" fields here too
                for (size_t i = 0; i < num_samples; i++) {
                    array<string, 6> meta = {"0", samples[i], "0", "0", "0", "-9"};

                    filereaders[i] = make_unique<VcfFileReader>(this->corpus->context, meta);
                    minhashers[i] = make_unique<Minhasher>(this->corpus->context);
                }
            } else {
                // VCF Content Lines
                // First 9 columns are: CHROM POS ID REF ALT QUAL FILTER INFO FORMAT
                boost::char_separator<char> sep("\t|");
                boost::tokenizer<boost::char_separator<char>> tok(*line, sep);
                auto iter = tok.begin();
                auto end = tok.end();

                // Info columns
                for (int i = 0; i < 9; i++) {
                    iter++;
                }

                for (auto & filereader : filereaders) {
                    filereader->add_bits(iter++->front(), iter++->front());
                }
            }
            // We just finished reading in input here
        }
    }


    // After all of the lines are read in, then we can process them
    // So after runFlag is false, then we do the execution
    // Loop over every one
    for ( int j = 0; j < filereaders.size(); j++) {
        auto parser = move(filereaders[j]);
        auto minhash = move(minhashers[j]);

        parser->register_to_experiment(this->corpus); //registers the two haplotypes read into the corpus.

        uint32_t * hash_buffer; //minhash buffer
        uint32_t ** lsh_buffer; //LSH temporary buffer
        std::unordered_map<uint32_t ,unsigned short> relatives[2]; //LSH hits for each haplotype.


        for(unsigned i = 0; i < this->corpus->context->slice_idx.size(); i++) {

            //setting the slice number will let the parser know what part of the coordinates to use
            parser->set_slice(i);
            minhash->set_slice_num(i);
            //int counter = 0;

            //first, we parse
            if ((!this->corpus->context->auto_slice) || this->corpus->context->minhashable[i]) {
                while (parser->hasNext()) {
                    //counter++;
                    //gets the hash buffer from the parser and feeds it into the minhash analyzer.
                    hash_buffer = parser->getNextHashed();
                    minhash->insert(hash_buffer);
                    //once all the hash values are inserted in the minhash analyzer, minhash analyzer can calculate the LSH values.
                }

                //preparing for LSH
                //based on the hashes fed into the minhash analyzer, it generates the lsh signature and returns in in a buffer.
                lsh_buffer = minhash->calculate_lsh();

                relatives[0].clear();
                relatives[1].clear();

                this->corpus_generator(lsh_buffer, relatives, i, parser->ids);
                this->aggregator(relatives, i, parser->ids);
            } else {
                while (parser->hasNext()) {
                    //counter++;
                    parser->getNextHashed();
                }
            }
        }
    }
}

void LSH_Slave::run() { 
    auto line = make_unique<string>();
    while(*runFlag) {
        //trying to read from input Q
        this->linesLock->lock();
        if (this->linesQ->empty()) {
            //if empty, we release the lock and wait for another round
            this->linesLock->unlock();
            // *run_flag = false;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else {
            line = move(this->linesQ->front());
            this->linesQ->pop();
            this->linesLock->unlock();

            auto *parser = new filereader(std::move(line),this->corpus->context);
            auto *minhash = new Minhasher(this->corpus->context);

            parser->register_to_experiment(this->corpus); //registers the two haplotypes read into the corpus. 

            uint32_t * hash_buffer; //minhash buffer
            uint32_t ** lsh_buffer; //LSH temporary buffer
            std::unordered_map<uint32_t ,unsigned short> relatives[2]; //LSH hits for each haplotype. 


            for(unsigned i = 0; i < this->corpus->context->slice_idx.size(); i++){
                
                //setting the slice number will let the parser know what part of the coordinates to use
                parser->set_slice(i); 
                minhash->set_slice_num(i);
                //int counter = 0;

                //first, we parse
                if( (!this->corpus->context->auto_slice) || this->corpus->context->minhashable[i]){
                    while(parser->hasNext()){
                        //counter++;
                        //gets the hash buffer from the parser and feeds it into the minhash analyzer.
                        hash_buffer = parser->getNextHashed(); 
                        minhash->insert(hash_buffer);
                        //once all the hash values are inserted in the minhash analyzer, minhash analyzer can calculate the LSH values. 
                    }

                    //preparing for LSH
                    //based on the hashes fed into the minhash analyzer, it generates the lsh signature and returns in in a buffer. 
                    lsh_buffer = minhash->calculate_lsh();

                    relatives[0].clear();
                    relatives[1].clear();

                    this->corpus_generator(lsh_buffer,relatives,i,parser->ids);
                    this->aggregator(relatives,i,parser->ids);


                }else{
                    while(parser->hasNext()){
                        //counter++;
                        parser->getNextHashed();

                    }
                }

            }
            delete parser;
            delete minhash;
        }
    }

}

inline void LSH_Slave::corpus_generator(uint32_t **lsh_matrix, std::unordered_map<uint32_t, unsigned short> * relatives, unsigned slice_num, uint32_t *ids) {

    for(int i = 0 ; i < 2 ; i++){
        this->corpus->add_to_corpus(lsh_matrix[i],ids[i],slice_num,relatives+i);
    }

}

inline void LSH_Slave::aggregator(std::unordered_map<uint32_t, unsigned short> * relatives, unsigned slice_num, uint32_t * ids) {
    for(int i = 0 ; i < 2; i++){
        this->corpus->integrate(relatives+i,ids[i],slice_num);
    }
}
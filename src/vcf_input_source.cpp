#include <fstream>
#include <istream>
#include <vector>
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

#include "headers/vcf_input_source.h"

Vcf_Input_Source::Vcf_Input_Source(const char *input_addr) : instream{std::make_unique<std::ifstream>(input_addr, std::ifstream::in)} {
    // see if stream is compressed
    // set up appropriate decompressor

    // Count how many individuals are in the file to determine end


    lines = transposeVcfToPed(*instream);
    iter = lines.begin();
}

Vcf_Input_Source::Vcf_Input_Source(std::unique_ptr<std::istream> &&input_stream) : instream{std::move(input_stream)} {
    lines = transposeVcfToPed(*instream);
    iter = lines.begin();
}

/**
 * Puts the next line into the passed std::string variable.
 * Moves it out of the vector, meaning that the content of the string is not copied.
 * @param line - The line to put the next individual's data into
 * @return true if the line was updated
 */
bool Vcf_Input_Source::getNextLine(std::string &line) {
    if (iter == lines.end()) {
        // Try getting next chunk

        // If that was last chunk, then return false
        return false;
    }

    line = std::move(*iter++);
    return true;
}

/**
 * Reads the entire vcf into memory, and transposes it into 1 line per individual.
 * This uses significant memory, based on the size of the input VCF.
 *
 * A more memory-efficient approach would be chunking it, making several passes over the input file each of which gets
 * a set of columns and building the phased .ped file in chunks.
 *
 * We could also store the genetic info in a more compact from, and build the full line string on demand.
 * @param infile - An ifstream of the VCF file to read in
 * @return a vector with one line per individual as a string
 */
std::vector<std::string> Vcf_Input_Source::transposeVcfToPed(std::istream &infile) {
    std::vector<std::string> pedLines;

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty() || (line)[0] == '#') {
            continue;
        }

        boost::char_separator<char> sep("\t");

        boost::tokenizer<boost::char_separator<char>> tok(line, sep);
        boost::tokenizer<boost::char_separator<char>>::iterator iter = tok.begin();
        auto end = tok.end();

        for (int i = 0; i < 9; ++i) {
            iter++; // Skip to the genetic data
        }

        size_t i = 0;
        for (; iter != end; iter++) {
            if (pedLines.size() <= i) {
                auto individualData = boost::format("0 %s 0 0 0 -9") % i;
                pedLines.emplace_back(individualData.str());
            }

            auto pair = *iter;
            auto first = pair[0];
            auto second = pair[2];

            if (pair[1] != '|') {
                throw std::runtime_error("This VCF is not phased.");
            }

            pedLines[i].push_back(' ');
            pedLines[i].push_back(first);
            pedLines[i].push_back(' ');
            pedLines[i].push_back(second);

            ++i;
        }
    }

    return pedLines;
}

/**
 * Read a chunk of the VCF into pedLines, from index start to end
 * Index 0 will be the first individual's records in the VCF.
 *
 * A more memory-efficient approach would be chunking it, making several passes over the input file each of which gets
 * a set of columns and building the phased .ped file in chunks.
 *
 * We could also store the genetic info in a more compact from, and build the full line string on demand.
 * @param infile - An ifstream of the VCF file to read in
 * @param start - The start index of the chunk, inclusive
 * @param end - The end index of the chunk, exclusive
 * @return a vector with one line per individual as a string
 */
std::vector<std::string> Vcf_Input_Source::getNextChunk(std::istream &infile, int64_t start, int64_t end) {

    if (start >= end) {
        throw std::invalid_argument("Start index of chunk must be smaller than end.");
    }

    std::vector<std::string> pedLines;
    std::string line;

    while (std::getline(infile, line)) {
        if (line.empty() || (line)[0] == '#') {
            continue;
        }

        boost::char_separator<char> sep("\t");

        boost::tokenizer<boost::char_separator<char>> tok(line, sep);
        boost::tokenizer<boost::char_separator<char>>::iterator lineIter = tok.begin();
        auto end_tok = tok.end();

        for (int i = 0; i < 9; ++i) {
            lineIter++; // Skip to the genetic data
        }

        size_t i = 0;

        boost::tokenizer<boost::char_separator<char>>::iterator chunk_end = tok.begin();

        for (auto j = 0; j < start && lineIter != end_tok; ++j) {
            ++lineIter;
        }

        for (auto j = 0; j < end && chunk_end != end_tok; ++j) {
            ++chunk_end;
        }

        for (; lineIter != chunk_end; lineIter++) {
            if (pedLines.size() <= i) {
                auto individualData = boost::format("0 %s 0 0 0 -9") % i;
                pedLines.emplace_back(individualData.str());
            }

            auto pair = *lineIter;
            auto first = pair[0];
            auto second = pair[2];

            if (pair[1] != '|') {
                throw std::runtime_error("This VCF is not phased.");
            }

            pedLines[i].push_back(' ');
            pedLines[i].push_back(first);
            pedLines[i].push_back(' ');
            pedLines[i].push_back(second);

            ++i;
        }
    }

    return pedLines;
}
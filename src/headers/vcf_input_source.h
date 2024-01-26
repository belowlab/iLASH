#ifndef ILASH_VCF_INPUT_SOURCE_H
#define ILASH_VCF_INPUT_SOURCE_H

#include <iterator>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "input_source.h"

class Vcf_Input_Source : public Input_Source {
public:
    explicit Vcf_Input_Source(const char *input_addr);
    explicit Vcf_Input_Source(std::unique_ptr<std::istream> &&input_stream);
    ~Vcf_Input_Source() override = default;

    bool getNextLine(std::string &line) override;
private:
    std::unique_ptr<std::istream> instream;
    std::vector<std::string> lines;
    std::vector<std::string>::iterator iter;
    int64_t chunk_start;
    int64_t chunk_end;
    int64_t vcf_end;
    std::vector<std::string> getNextChunk(std::istream &infile, int64_t start, int64_t end);

    static std::vector<std::string> transposeVcfToPed(std::istream &infile);
};

#endif //ILASH_VCF_INPUT_SOURCE_H

#include <iostream>
#include <vector>


class CompressedByteArray
{
private:
    std::vector<uint8_t> array; /* compressed in-memory sequence */
    std::vector<uint8_t> symbol_table; /* symbol table */
    size_t bits_per_symbol; /* bits needed per symbol */
    size_t length; /* length of uncompressed sequence */

    bool _load_uncompressed(std::FILE*); /* load uncompressed sequence */
    bool _load_compressed(std::FILE*); /* load compressed sequence */

public:
    size_t size(); /* returns size of sequence */
    void clear(); /* empty the sequence */
    bool load(std::FILE* file, bool is_compressed); /* load sequence from file */
    bool dump(std::FILE* file); /* dump compressed sequence to file */
    uint8_t operator[](int i); /* random access ith symbol */
};

#include "compressed_byte_array.h"

const std::size_t BUFSIZE = 1<<20;

size_t CompressedByteArray::size()
{
    return length;
}


void CompressedByteArray::clear()
{
    array.clear();
    symbol_table.clear();
    bits_per_symbol = 1;
    length = 0;
}


bool CompressedByteArray::_load_uncompressed(std::FILE* file)
{
    /* Read input (assuming it can fit in memory) */
    std::vector<uint8_t> input;
    uint8_t buf[BUFSIZE];
    size_t nc;
    while ((nc = std::fread(buf, sizeof(uint8_t), BUFSIZE, file)) > 0)
        input.insert(input.end(), buf, buf + nc);
    input.shrink_to_fit(); /* reduce memory usage a bit */
    length = input.size();
    std::cerr << "input size: " << input.size() << std::endl;

    /* Build symbol table */
    int symbol_index[256]; /* map from symbol to index in symbol table */
    for (auto& sym : symbol_index)
        sym = -1;
    for (auto sym : input) {
        if (symbol_index[sym] == -1) { /* unseen symbol */
            symbol_index[sym] = symbol_table.size();
            symbol_table.push_back(sym);
        }
    }
    std::cerr << "num of symbols: " << symbol_table.size() << std::endl;

    /* Find number of bits needed per symbol */
    while ((1 << bits_per_symbol) < symbol_table.size())
        bits_per_symbol++;
    std::cerr << "bits per symbol: " << bits_per_symbol << std::endl;

    /* Encode one symbol at a time */
    int bits_ready = 0; /* number of finished bits in the current byte */
    uint8_t byte = 0; /* current byte */
    int encoded = 0; /* number of symbols encoded */

    while (encoded < input.size()) {
        int code = symbol_index[input[encoded]]; /* encoded symbol */
        /* Take 'bits_per_symbol' bits from the input */
        for (int i = 0; i < bits_per_symbol; i++) {
            if (bits_ready == 8) {
                /* ready to dump a byte */
                array.push_back(byte);
                byte = 0;
                bits_ready = 0;
            }
            byte = (byte << 1) | (code >> (bits_per_symbol-1));
            code <<= 1;
            bits_ready++;
        }
        encoded++;
    }

    /* Dump last byte if needed */
    if (bits_ready > 0) {
        array.push_back(byte << (8 - bits_ready));
    }

    return true;
}


bool CompressedByteArray::_load_compressed(std::FILE* file)
{
    /* Read symbol table length */
    size_t symbol_table_size;
    if (std::fread(&symbol_table_size, 1, sizeof(size_t), file) != sizeof(size_t))
        return false;

    /* Read symbol table */
    symbol_table.resize(symbol_table_size);
    if (std::fread(symbol_table.data(), sizeof(uint8_t), symbol_table_size, file) !=
            sizeof(uint8_t) * symbol_table_size)
        return false;
    std::cerr << "num of symbols: " << symbol_table.size() << std::endl;

    /* Read bits per symbol */
    if (std::fread(&bits_per_symbol, 1, sizeof(size_t), file) != sizeof(size_t))
        return false;
    std::cerr << "bits per symbol: " << bits_per_symbol << std::endl;

    /* Read sequence length */
    if (std::fread(&length, 1, sizeof(size_t), file) != sizeof(size_t))
        return false;
    std::cerr << "sequence length: " << length << std::endl;

    /* Read sequence */
    uint8_t buf[BUFSIZE];
    size_t nc;
    while ((nc = std::fread(buf, sizeof(uint8_t), BUFSIZE, file)) > 0)
        array.insert(array.end(), buf, buf + nc);
    array.shrink_to_fit(); /* reduce memory usage a bit */

    return true;
}


bool CompressedByteArray::load(std::FILE* file, bool is_compressed)
{
    clear();

    bool ret = true;

    if (is_compressed) {
        ret = _load_compressed(file);
    } else {
        ret = _load_uncompressed(file);
    }

    return ret;
}


bool CompressedByteArray::dump(std::FILE* file)
{
    /* Dump symbol table length */
    size_t s = symbol_table.size();
    if (std::fwrite(&s, 1, sizeof(size_t), file) != sizeof(size_t))
        return false;

    /* Dump symbol table */
    if (std::fwrite(symbol_table.data(), sizeof(uint8_t), symbol_table.size(), file) !=
            sizeof(uint8_t) * symbol_table.size())
        return false;

    /* Dump bits per symbol */
    if (std::fwrite(&bits_per_symbol, 1, sizeof(size_t), file) != sizeof(size_t))
        return false;

    /* Dump sequence length */
    if (std::fwrite(&length, 1, sizeof(size_t), file) != sizeof(size_t))
        return false;

    /* Dump sequence */
    if (std::fwrite(array.data(), sizeof(uint8_t), array.size(), file) !=
            sizeof(uint8_t) * array.size())
        return false;

    return true;
}


uint8_t CompressedByteArray::operator[](int i)
{
    int bit_idx = bits_per_symbol * i;
    int byte_idx = bits_per_symbol * i / 8;
    int bit_offset_in_byte = bit_idx - byte_idx * 8;

    uint8_t out = array[byte_idx];

    /* check if this symbol spills across two bytes */
    int spill = bit_offset_in_byte + bits_per_symbol - 8;
    if (spill > 0) {
        out <<= bit_offset_in_byte; /* clear preceding bits */
        out >>= (bit_offset_in_byte - spill);
        out |= (array[byte_idx+1] >> (8 - spill)); /* add spill bits */
    } else {
        out <<= bit_offset_in_byte; /* clear preceding bits */
        out >>= (8 - bits_per_symbol);
    }

    return symbol_table[out];
}

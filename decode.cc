#include <iostream>
#include "compressed_byte_array.h"


int main()
{
    std::cerr << "--- decoding ---" << std::endl;

    /* read compressed file from stdin */
    CompressedByteArray array;
    if (!array.load(stdin, true)) {
        std::cerr << "error loading compressed file" << std::endl;
        return -1;
    }

    /* write decompressed file to stdout */
    for (int i = 0; i < array.size(); i++) {
        if (std::fputc(array[i], stdout) == EOF) {
            std::cerr << "error writing decompressed file" << std::endl;
            return -1;
        }
    }

    return 0;
}

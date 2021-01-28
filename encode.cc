#include <iostream>
#include "compressed_byte_array.h"


int main()
{
    std::cerr << "--- encoding ---" << std::endl;

    /* read uncompressed file from stdin */
    CompressedByteArray array;
    if (!array.load(stdin, false)) {
        std::cerr << "error loading file" << std::endl;
        return -1;
    }

    /* write compressed file to stdout */
    if (!array.dump(stdout)) {
        std::cerr << "error writing compressed file" << std::endl;
        return -1;
    }

    return 0;
}

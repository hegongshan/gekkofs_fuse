/**
 * Compute the base2 logarithm for 64 bit integers
 */
inline int log2(uint64_t n){

    /* see http://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers */
    static const int table[64] = {
        0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3, 61,
        51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4, 62,
        57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56,
        45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5, 63 };

    assert(n > 0); // TODO This needs to be removed and a check for CHUNKSIZE has to be put somewhere

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;

    return table[(n * 0x03f6eaf2cd271461) >> 58];
}


/**
 * Align an @offset to the closest left side block boundary
 */
inline off64_t lalign(const off64_t offset, const size_t block_size) {
    return offset & ~(block_size - 1);
}


/**
 * Align an @offset to the closest right side block boundary
 */
inline off64_t ralign(const off64_t offset, const size_t block_size) {
    return lalign(offset + block_size, block_size);
}


/**
 * Return the padding (bytes) that separates the @offset from the closest
 * left side block boundary
 *
 * If @offset is a boundary the resulting padding will be 0
 */
inline size_t lpad(const off64_t offset, const size_t block_size) {
    return offset % block_size;
}


/**
 * Return the padding (bytes) that separates the @offset from the closest
 * right side block boundary
 *
 * If @offset is a boundary the resulting padding will be 0
 */
inline size_t rpad(const off64_t offset, const size_t block_size) {
    return (-offset) % block_size;
}


/**
 * Given an @offset calculates the block number to which the @offset belongs
 *
 * block_num(8,4) = 2;
 * block_num(7,4) = 1;
 * block_num(2,4) = 0;
 * block_num(0,4) = 0;
 */
inline uint64_t block_num(const off64_t offset, const size_t block_size){
    return lalign(offset, block_size) >> log2(block_size);
}


/**
 * Return the number of blocks involved in an operation that operates
 * from @offset for a certain amount of bytes (@count).
 */
inline uint64_t blocks_count(const off64_t offset, const size_t count, const size_t block_size) {

    off64_t block_start = lalign(offset, block_size);
    off64_t block_end = lalign(offset + count - 1, block_size);

    return (block_end >> log2(block_size)) -
        (block_start >> log2(block_size)) + 1;
}

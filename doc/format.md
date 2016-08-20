# File formats {#page_formats}

## Generalities

There are two file formats used by `librsync` and `rdiff`: the
*signature* file, which summarizes a data file, and the *delta* file,
which describes the edits from one data file to another.

librsync does not know or care about any formats in the data files.

All integers are big-endian.

## Magic numbers

All librsync files start with a `uint32` magic number identifying
them. These are declared in `librsync.h`:

### RS_DELTA_MAGIC

A delta file. At present, there's only one delta format.

    RS_DELTA_MAGIC = 0x72730236  // "rs\x026"

### RS_MD4_SIG_MAGIC

A signature file using original rollsum and MD4 hash.

Backward compatible with librsync < 1.0, but strongly deprecated
because it creates a security vulnerability on files containing partly
untrusted data. See <https://github.com/librsync/librsync/issues/5>.

    RS_MD4_SIG_MAGIC = 0x72730136  // "rs\x016"

### RS_BLAKE2_SIG_MAGIC

A signature file using original rollsum and BLAKE2 hash.

Supported from librsync 1.0. with much better security than MD4.

    RS_BLAKE2_SIG_MAGIC = 0x72730137  // "rs\x017"

### RS_ROLLSUM2_MD4_SIG_MAGIC

A signature file using rollsum2 and MD4 hash.

Supported from librsync XXX. Uses less secure MD4 with stronger
rollsum2 for reduced weaksum collisions calculating deltas. This
should be the fastest for files with large diffs using a small
blocksize.

    RS_ROLLSUM2_MD4_SIG_MAGIC = 0x72730138  // "rs\x018"

### RS_ROLLSUM2_BLAKE2_SIG_MAGIC

A signature file using rollsum2 and BLAKE2 hash.

Supported from librsync XXX. Uses secure BLAKE2 with stronger rollsum2
for reduced weaksum collisions calculating deltas. This should be the
most secure and fast for files with large diffs.

    RS_ROLLSUM2_BLAKE2_SIG_MAGIC = 0x72730139  // "rs\x019"

## Signatures

Signatures consist of a header followed by a number of block
signatures.

Each block signature gives signature hashes for one block of
`block_len` bytes from the input data file. The final data block
may be shorter. The number of blocks in the signature is therefore

    ceil(input_len/block_len)

The signature header is (see `rs_sig_s_header`):

    u32 magic;     // one of the RS_*_SIG_MAGIC magic numbers.
    u32 block_len; // bytes per block
    u32 strong_sum_len;  // bytes per strong sum in each block

The block signature contains a rolling or weak checksum used to find
moved data, and a strong hash used to check the match is correct. The
weak checksum is either the original rollsum in `rollsum.c` or
stronger rollsum2 in `rollsum2.c`. The strong hash is either MD4 or
BLAKE2 depending on the magic number.

To make the signatures smaller at a cost of a greater chance of
collisions, the `strong_sum_len` in the header can cause the strong
sum to be truncated to the left after computation.

Each signature block format is (see `rs_sig_do_block`):

    u32 weak_sum;
    u8[strong_sum_len] strong_sum;

## Delta files

TODO(https://github.com/librsync/librsync/issues/46): Document delta format.

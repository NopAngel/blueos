#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "LZ4"

/**
 * lz4_decompress_unknown_size: Implements native fast block streaming parsing.
 */
int lz4_decompress_unknown_size(const char *source, char *dest, int isize,
                                int max_output) {
  int src_ptr = 0;
  int dest_ptr = 0;

  boot_msg(MODULE_NAME,
           "Streaming raw byte tokens directly through LZ4 decompression "
           "pipeline...",
           0);

  while (src_ptr < isize) {
    uint8_t token = (uint8_t)source[src_ptr++];
    int literal_len = token >> 4;

    /* Parse extended literal lengths */
    if (literal_len == 15) {
      uint8_t s;
      do {
        s = (uint8_t)source[src_ptr++];
        literal_len += s;
      } while (s == 255);
    }

    /* Copy literal bytes from source directly into destination frame */
    for (int i = 0; i < literal_len; i++) {
      if (dest_ptr >= max_output)
        return -1;
      dest[dest_ptr++] = source[src_ptr++];
    }

    if (src_ptr >= isize)
      break;

    /* Unpack match distance offset (2-byte lower endian scalar) */
    uint16_t offset =
        (uint16_t)source[src_ptr] | ((uint16_t)source[src_ptr + 1] << 8);
    src_ptr += 2;

    int match_len = token & 0x0F;
    if (match_len == 15) {
      uint8_t s;
      do {
        s = (uint8_t)source[src_ptr++];
        match_len += s;
      } while (s == 255);
    }
    match_len += 4; /* Minimum match factor length constant */

    /* Reclaim data bytes from previously unpacked dictionary sequences */
    int match_back_ptr = dest_ptr - offset;
    for (int i = 0; i < match_len; i++) {
      if (dest_ptr >= max_output)
        return -1;
      dest[dest_ptr++] = dest[match_back_ptr++];
    }
  }

  return dest_ptr; /* Total raw bytes written into RAM */
}
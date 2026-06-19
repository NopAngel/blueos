#include <kernel/snd.h>
#include <lib/string.h>
#include <mm/heap.h>

static list_t *_snd_sessions;
static list_t *_snd_devices;

static ssize_t snd_dsp_write(fs_node_t *node, size_t size, uint8_t *buffer) {
  struct dsp_session *session = (struct dsp_session *)node->priv_data;

  size_t written = ring_buffer_write(session->rb, size, buffer);
  return written;
}

int snd_mix_audio(uint8_t *out_buffer, uint32_t size) {
  memset(out_buffer, 0, size);
  int16_t *mix_ptr = (int16_t *)out_buffer;
  int16_t temp_buf[256];

  foreach (node, _snd_sessions) {
    struct dsp_session *session = (struct dsp_session *)node->value;
    size_t available = ring_buffer_unread(session->rb);
    size_t to_read = (available < size) ? available : size;

    if (to_read > 0) {
      ring_buffer_read(session->rb, to_read, (uint8_t *)temp_buf);

      for (size_t i = 0; i < to_read / 2; i++) {
        int32_t mixed = mix_ptr[i] + (temp_buf[i] / 2);
        if (mixed > 32767)
          mixed = 32767;
        if (mixed < -32768)
          mixed = -32768;
        mix_ptr[i] = (int16_t)mixed;
      }
    }
  }
  return size;
}

void hardware_audio_interrupt_handler() {
  uint8_t dma_buffer[1024];

  snd_mix_audio(dma_buffer, 1024);
  copy_to_hardware_dma(dma_buffer, 1024);
}

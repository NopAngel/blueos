#ifndef SND_H
#define SND_H

#include <stdint.h>
#include <lib/list.h>
#include <fs/vfs.h>
#include <lib/ringbuffer.h>

#define SND_BUF_SIZE 0x8000 
#define MAX_SND_DEVICES 4

typedef struct snd_device {
    char name[32];
    uint32_t id;
    int (*request_samples)(struct snd_device* dev, uint32_t size, uint8_t* buffer);
    void* priv_data;
} snd_device_t;

struct dsp_session {
    ring_buffer_t* rb;
    int active;
};

void snd_init(void);
int  snd_register_device(snd_device_t* dev);
int  snd_mix_audio(uint8_t* out_buffer, uint32_t size);

#endif
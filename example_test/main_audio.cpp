#include "mbed.h"
#include "USBAudio.h"
#include "test_util.h"
#include <math.h>

int16_t square_wave(uint32_t freq_hz, uint16_t amplitude, float time_s)
{
    float period = (float)1 / freq_hz;
    if (fmod(time_s, period) > period / 2) {
        return amplitude / 2;
    } else {
        return -(amplitude / 2);
    }
}

volatile bool loopback_main_done;
void audio_loopback_main(USBAudio *audio) {
    static uint8_t buf[128];
    while (!loopback_main_done) {
        if (!audio->read(buf, sizeof(buf))) {
            audio->read_wait_ready();
            continue;
        }
        uint32_t actual;
        audio->write_nb(buf, sizeof(buf), &actual);
    }
}

void audio_out_main(USBAudio *audio, uint32_t audio_freqency, float play_time) {

    float cur_time = 0;
    while (cur_time < play_time) {
        uint16_t samples[64];
        for (int i = 0; i < 64; i++) {
            samples[i] = square_wave(100, 5000, cur_time);
            cur_time += 1.0 / audio_freqency;
        }
        if (!audio->write((uint8_t *)&samples, sizeof(samples))) {
            audio->write_wait_ready();
        }
    }
}

void audio_channel_test()
{
    uint32_t tx_freq = 16000;
    USBAudio audio(true, 8000, 2, tx_freq, 1, 10, 0x7bb8, 0x1112, 0x0100);

    printf("Unselect mbed audio device and press enter\r\n");
    test_util_wait_line();

    // Assert that audio channels are closed
    MBED_ASSERT(!audio.write_ready());
    MBED_ASSERT(!audio.read_ready());
    printf("Audio channels are closed as expected\r\n\r\n");

    printf("Select mbed as your audio output device and play sound\r\n");
    audio.read_wait_ready();

    MBED_ASSERT(audio.read_ready());
    printf("Audio output channel opened successfully\r\n\r\n");

    printf("Start recording Audio\r\n");
    audio.write_wait_ready();
    MBED_ASSERT(audio.write_ready());
    printf("Audio input channel opened successfully and playing sound\r\n\r\n");

    audio_out_main(&audio, tx_freq, 5.0);
    printf("Was the recorded audio a 100Hz square wave?\r\n");
    test_util_get_response();
}

void audio_volume_test()
{
    USBAudio audio(true, 44100, 2, 44100, 2, 10, 0x7bb8, 0x1113, 0x0100);

    printf("Select mbed as your audio output device and play sound\r\n");
    audio.read_wait_ready();

    printf("Turn sound to full\r\n");
    float prev = -1;
    while (true) {
        float volume = audio.get_volume();
        if (volume != prev) {
            printf("Current: %f       \r", volume);
            fflush(stdout);
            prev = volume;
        }
        if (volume > 0.999) {
            printf("\r\nSound turned to full\r\n\r\n");
            break;
        }
    }

    printf("Mute sound\r\n");
    prev = -1;
    while (true) {
        float volume = audio.get_volume();
        if (volume != prev) {
            printf("Current: %f       \r", volume);
            fflush(stdout);
            prev = volume;
        }
        if (volume < 0.001) {
            printf("\r\nSound muted\r\n\r\n");
            break;
        }
    }
}

void audio_loopback_test()
{
    USBAudio audio(false, 44100, 2, 44100, 2, 10, 0x7bb8, 0x1113, 0x0100);
    audio.connect();

    loopback_main_done = false;
    Thread loopback(osPriorityHigh);
    loopback.start(callback(audio_loopback_main, &audio));

    printf("Select mbed as your audio output device and both play and record a sound\r\n");
    printf("When the sound is played back does it sound correct?\r\n");
    MBED_ASSERT(test_util_get_response());

    // No overflows or underflows allowed
    uint32_t read_overflows = audio.read_overflows();
    uint32_t write_underflows = audio.write_underflows();
    if (read_overflows > 0) {
        printf("Read overflows  %lu\r\n", read_overflows);
    }
    if (write_underflows > 0) {
        printf("Write underflow  %lu\r\n", write_underflows);
    }
    MBED_ASSERT(read_overflows == 0);
    MBED_ASSERT(write_underflows == 0);

    printf("Exiting test\r\n");
    loopback_main_done = true;
    audio.disconnect();
    loopback.join();
    printf("Returning from test\r\n");
}

int main_audio() {
   printf("Starting Audio test\r\n");

   audio_channel_test();

   audio_volume_test();

   audio_loopback_test();

   printf("Audio test passed\r\n");

   return 0;
}

#include <altera_up_avalon_audio.h>
#include <altera_avalon_pio_regs.h>
#include <sys/alt_stdio.h>
#include <stdlib.h>
#include <system.h>

int main(void) {
    alt_up_audio_dev *audio_dev = alt_up_audio_open_dev("/dev/audio_0");
    if (audio_dev == NULL) {
        alt_printf("Error: could not open audio device\n");
        return -1;
    } else
        alt_printf("Opened audio device\n");

    const int run_time_in_seconds = 30;
    const int run_time_in_samples = run_time_in_seconds * 48000;
    int sample_count = 0;
    do {
        int fifospace_right = alt_up_audio_read_fifo_avail(audio_dev, ALT_UP_AUDIO_RIGHT);
        if (fifospace_right > 0) { // check if data is available
        	sample_count++;
        	// read audio buffer
        	unsigned int r_buf = alt_up_audio_read_fifo_head(audio_dev, ALT_UP_AUDIO_RIGHT);
            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, abs((short)r_buf)>>5); // light up the leds
            // write audio buffer
            alt_up_audio_write_fifo_head(audio_dev, r_buf, ALT_UP_AUDIO_RIGHT);
        }
        int fifospace_left = alt_up_audio_read_fifo_avail(audio_dev, ALT_UP_AUDIO_LEFT);
        if (fifospace_left > 0) { // check if data is available
        	unsigned int l_buf = alt_up_audio_read_fifo_head(audio_dev, ALT_UP_AUDIO_LEFT);
    	    alt_up_audio_write_fifo_head(audio_dev, l_buf, ALT_UP_AUDIO_LEFT);
		}
    } while (sample_count < run_time_in_samples);
    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0); // switch off the leds
}

#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef struct
{
    char riff[4];       // "RIFF"
    uint32_t file_size; // File size - 8
    char wave[4];       // "WAVE"

    char fmt[4];              // "fmt "
    uint32_t fmt_size;        // 16 for PCM
    uint16_t audio_format;    // 1 = PCM
    uint16_t num_channels;    // 1 = mono
    uint32_t sample_rate;     // 44100
    uint32_t byte_rate;       // sample_rate * channels * bits/8
    uint16_t block_align;     // channels * bits/8
    uint16_t bits_per_sample; // 16

    char data[4];       // "data"
    uint32_t data_size; // Number of bytes of PCM data
} WAVHeader;

#define NUM_CHORDS 4
int main()
{
    float samples_ps = 44100.00;
    int max_encode = 32767; // 16-bit PCM, so each side of the wave should be at most 2^15:- 0->32767
    int duration = 2;
    int samples_per_chord = duration * samples_ps;
    int total_samples = NUM_CHORDS * samples_per_chord;

    float fade_start = (samples_per_chord) * 0.9;
    float fade_end = samples_per_chord;

    float chord_freq[NUM_CHORDS][3] = {
        {261.63, 329.63, 392.00},
        {349.23, 440.00, 523.25},
        {440.00, 523.25, 659.25},
        {392.00, 493.88, 587.33}};

    FILE *f;
    f = fopen("sine_cfamg.wav", "wb");
    if (f == NULL)
    {
        printf("\nfiles not opened for us to save it at, lol\n");
        return -1;
    }

    WAVHeader header = {
        .riff = {'R', 'I', 'F', 'F'},
        .file_size = 36 + total_samples * 2,
        .wave = {'W', 'A', 'V', 'E'},

        .fmt = {'f', 'm', 't', ' '},
        .fmt_size = 16,
        .audio_format = 1,
        .num_channels = 1,
        .sample_rate = 44100,
        .byte_rate = 44100 * 16 / 8,
        .block_align = 16 / 8,
        .bits_per_sample = 16,

        .data = {'d', 'a', 't', 'a'},
        .data_size = total_samples * 2};

    fwrite(&header, sizeof(header), 1, f);
    printf("WAV header size = %zu bytes\n", sizeof(WAVHeader));
    float time = 0.00;
    int outer = 0;
    for (outer; outer < 4; outer++)
    {
        // starting off at the 0th sample
        int current_sample = 0;

        while (current_sample < samples_per_chord)
        {
            short int sample = 0;
            // the notion here is we need to express time relative to where we are at in terms of our sampling.
            // At half the samples, we are at half the time, and so on
            time = current_sample / samples_ps;
            short int sample_next = 0;
            float current_weight, next_weight;

            /*
            Basically, generate a sample via the formula sin(2pi*f*(n/44100))

            This then normalizes the max_encoding we have for PCM-16.

            Now, since we are generating these for 3 frequencies per chord, we iterate over this thrice,
            and take an average of these to represent their weight equally in the chord_sample
            */
            for (int i = 0; i < 3; i++)
            {
                sample += (sin((2 * M_PI * chord_freq[outer][i]) * time)) * max_encode / 3;
            }

            /*
            Cross fade zone!

            This means we are in the last 10% of time left for the chord's life.
            */
            if (current_sample >= fade_start && outer < 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    sample_next += (sin((2 * M_PI * chord_freq[outer + 1][i]) * time)) * max_encode / 3;
                }
                current_weight = 1 - ((current_sample - fade_start) / (fade_end - fade_start));
                next_weight = 1 - current_weight;
                sample = sample * current_weight + sample_next * next_weight;
            }
            fwrite(&sample, 2, 1, f);
            current_sample = current_sample + 1;
        }
    }
    fclose(f);
    return 0;
}
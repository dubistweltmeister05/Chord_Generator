#include <stdio.h>
#include <stdint.h>
#include <math.h>

/*
A little bit about wave headers

Positions   Sample Value         Description
1 - 4       "RIFF"               Marks the file as a riff file. Characters are each 1. byte long.
5 - 8       File size (integer)  Size of the overall file - 8 bytes, in bytes (32-bit integer). Typically, you'd fill this in after creation.
9 -12       "WAVE"               File Type Header. For our purposes, it always equals "WAVE".
13-16       "fmt "               Format chunk marker. Includes trailing null
17-20       16                   Length of format data as listed above
21-22       1                    Type of format (1 is PCM) - 2 byte integer
23-24       2                    Number of Channels - 2 byte integer
25-28       44100                Sample Rate - 32 bit integer. Common values are 44100 (CD), 48000 (DAT). Sample Rate = Number of Samples per second, or Hertz.
29-32       176400               (Sample Rate * BitsPerSample * Channels) / 8.
33-34       4                    (BitsPerSample * Channels) / 8.1 - 8 bit mono2 - 8 bit stereo/16 bit mono4 - 16 bit stereo
35-36       16                   Bits per sample
37-40       "data"               "data" chunk header. Marks the beginning of the data section.
41-44       File size (data)     Size of the data section, i.e. file size - 44 bytes header.
*/
typedef struct
{
    char riff[4];       // "RIFF" — identifies this as a RIFF container
    uint32_t file_size; // Overall file size minus 8 bytes, in bytes

    char wave[4]; // "WAVE" — identifies the RIFF file as a WAV file

    char fmt[4];       // "fmt " — marks the beginning of the format chunk
    uint32_t fmt_size; // Size of the format chunk data (16 bytes for PCM)

    uint16_t audio_format; // Audio format: 1 = PCM
    uint16_t num_channels; // Number of audio channels: 1 = mono, 2 = stereo

    uint32_t sample_rate; // Samples per second, e.g. 44100 Hz
    uint32_t byte_rate;   // Bytes per second = sample_rate * num_channels * bits_per_sample / 8

    uint16_t block_align;     // Bytes per sample frame = num_channels * bits_per_sample / 8
    uint16_t bits_per_sample; // Number of bits used to represent each individual sample

    char data[4];       // "data" — marks the beginning of the PCM data chunk
    uint32_t data_size; // Number of bytes of PCM audio data following this header

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
        .file_size = 36 + total_samples * 4,
        .wave = {'W', 'A', 'V', 'E'},

        .fmt = {'f', 'm', 't', ' '},
        .fmt_size = 16,
        .audio_format = 1,
        .num_channels = 2,
        .sample_rate = 44100,
        .byte_rate = 44100 * 2 * 16 / 8,
        .block_align = 2 * 16 / 8,
        .bits_per_sample = 16,

        .data = {'d', 'a', 't', 'a'},
        .data_size = total_samples * 4};

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

            short int sample_left = 0;
            short int sample_right = 0;
            // the notion here is we need to express time relative to where we are at in terms of our sampling.
            // At half the samples, we are at half the time, and so on
            time = current_sample / samples_ps;
            short int sample_next_left = 0;
            short int sample_next_right = 0;
            float current_weight, next_weight;

            /*
            Basically, generate a sample via the formula sin(2pi*f*(n/44100))

            This then normalizes the max_encoding we have for PCM-16.

            Now, since we are generating these for 3 frequencies per chord, we iterate over this thrice,
            and take an average of these to represent their weight equally in the chord_sample
            */
            for (int i = 0; i < 3; i++)
            {
                sample_left += (sin((2 * M_PI * chord_freq[outer][i]) * time)) * max_encode / 3;
                sample_right += (sin((2 * M_PI * chord_freq[outer][i]) * time)) * max_encode / 3;
            }

            /*
            Cross fade zone!

            This means we are in the last 10% of time left for the chord's life.
            */
            if (current_sample >= fade_start && outer < 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    sample_next_left += (sin((2 * M_PI * chord_freq[outer + 1][i]) * time)) * max_encode / 3;
                    sample_next_right += (sin((2 * M_PI * chord_freq[outer + 1][i]) * time)) * max_encode / 3;
                }
                current_weight = 1 - ((current_sample - fade_start) / (fade_end - fade_start));
                next_weight = 1 - current_weight;
                sample_left = sample_left * current_weight + sample_next_left * next_weight;
                sample_right = sample_right * current_weight + sample_next_right * next_weight;
            }
            fwrite(&sample_left, sizeof(sample_left), 1, f);
            fwrite(&sample_right, sizeof(sample_right), 1, f);
            current_sample = current_sample + 1;
        }
    }
    fclose(f);
    return 0;
}
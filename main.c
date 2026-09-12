#include <stdio.h>
#include <math.h>

#define NUM_CHORDS 4
int main()
{
    float samples_ps = 44100.00;
    int max_encode = 32767; // 16-bit PCM, so each side of the wave should be at most 2^15:- 0->32767
    int duration = 2;
    int total_samples = duration * samples_ps;

    float fade_start = (total_samples) * 0.9;
    float fade_end = total_samples;

    float chord_freq[NUM_CHORDS][3] = {
        {261.63, 329.63, 392.00},
        {349.23, 440.00, 523.25},
        {440.00, 523.25, 659.25},
        {392.00, 493.88, 587.33}};

    FILE *f;
    f = fopen("sine_cfamg.raw", "wb");
    if (f == NULL)
    {
        printf("\nfiles not opened for us to save it at, lol\n");
        return -1;
    }
    float time = 0.00;
    int outer = 0;
    for (outer; outer < 4; outer++)
    {
        // starting off at the 0th sample
        int current_sample = 0;

        while (current_sample < total_samples)
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
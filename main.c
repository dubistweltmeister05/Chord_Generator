#include <stdio.h>
#include <math.h>
#include <direct.h>

/*
I want to implement crossfade between these chords. 
 */

int main(){
    char cwd[1024];

    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Working directory: %s\n", cwd);
    }
    float samples_ps = 44100.00;
    int max_encode = 32767; // 16-bit PCM, so each side of the wave should be at most 2^15:- 0->32767
    int duration = 2;
    int total_samples = duration*samples_ps;

    float fade_start = (total_samples)*0.9;
    float fade_end   = total_samples;

    float chord_freq[4][3] = {
                              {261.63,329.63,392.00}, 
                              {349.23, 440.00, 523.25}, 
                              {440.00, 523.25, 659.25}, 
                              {392.00, 493.88, 587.33}
                            };
    

    FILE *f;
    f = fopen("sine_cfamg.raw", "wb");
    if(f==NULL){
        printf("\nfiles not opened for us to save it at, lol\n");
        return -1;
    }
    float time =  0.00;
    float angle[4][3]={{0.00},{0.00},{0.00},{0.00}};
    short int sample;
    int outer=0;
    for(outer;outer<4;outer++)
    {   int i = 0;
        while(i<total_samples){
            time = i/samples_ps;
            for(int j=0;j<3;j++)
            {angle[outer][j]=(2*M_PI*chord_freq[outer][j])*time;}
            sample = (sin(angle[outer][0])*max_encode + sin(angle[outer][1])*max_encode + sin(angle[outer][2])*max_encode)/3;
            fwrite(&sample,2,1,f);
            i=i+1;
        }
    }
    fclose(f);
    return 0;
}
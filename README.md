# Chord Generator 🎵

A small C project that generates **chords as raw PCM audio** using sine waves.

This project is primarily a learning exercise in C, digital audio, signal generation, and eventually **crossfading between chords**.

## Current Goal

The current implementation generates a sequence of four chords, where each chord is constructed by adding three sine waves corresponding to the chord's notes.

The next step is to introduce a **crossfade between consecutive chords**, so that the transition from one chord to the next is smooth rather than an abrupt change in waveform.

### Current transition

```text
Chord 1              Chord 2
████████████████████|████████████████████
                    ↑
               abrupt change
```

### Desired transition

```text
Chord 1              Chord 2
██████████████▓▓▒▒░░░▒▒▓▓██████████████
                  ↑
              crossfade
```

---

## How It Works

The program generates audio samples at a sampling rate of:

```text
44,100 samples/second
```

Each chord lasts:

```text
2 seconds
```

and consists of three sine waves.

For example, the first chord is:

```c
{261.63, 329.63, 392.00}
```

These correspond approximately to:

* C4 — 261.63 Hz
* E4 — 329.63 Hz
* G4 — 392.00 Hz

The three sine waves are averaged to produce a single PCM sample:

```c
sample = (
    sin(angle[outer][0]) * max_encode +
    sin(angle[outer][1]) * max_encode +
    sin(angle[outer][2]) * max_encode
) / 3;
```

This keeps the resulting signal within the approximate range of a signed 16-bit PCM sample.

---

## Chords

The current chord progression is:

```text
C major → F major → A minor → G major
```

represented by:

```c
float chord_freq[4][3] = {
    {261.63, 329.63, 392.00},
    {349.23, 440.00, 523.25},
    {440.00, 523.25, 659.25},
    {392.00, 493.88, 587.33}
};
```

Each row contains the three frequencies used to construct one chord.

---

## Audio Generation

For every sample, the program calculates the corresponding point on each sine wave.

The basic sine wave equation is:

```text
sin(2πft)
```

where:

* `f` = frequency of the note
* `t` = time
* `2π` = one complete revolution in radians

In the program:

```c
angle[outer][j] =
    (2 * M_PI * chord_freq[outer][j]) * time;
```

The resulting sine waves are then combined.

---

## Output Format

The generated audio is written directly to:

```text
sine_cfamg.raw
```

The file is written in:

```text
16-bit signed PCM
44.1 kHz
Mono
Raw format
```

Because this is a `.raw` file, it does not contain a WAV header describing the audio format.

A player that supports raw PCM data must therefore be told the correct parameters:

```text
Sample rate: 44100 Hz
Bit depth:   16-bit
Channels:    1
Encoding:    Signed PCM
Endianness:  Little-endian
```

---

## Crossfade

The intended crossfade region is currently defined as the final 10% of each chord:

```c
float fade_start = total_samples * 0.9;
float fade_end   = total_samples;
```

For a two-second chord at 44.1 kHz:

```text
Total samples = 88,200
Crossfade starts = 79,380
Crossfade length = 8,820 samples
```

The intended behavior is to gradually reduce the volume of the current chord while simultaneously increasing the volume of the next chord.

Conceptually:

```text
Current chord weight

1.0 ────────────────╲
                     ╲
                      ╲
                       ╲ 0.0


Next chord weight

0.0 ────────────────╱
                   ╱
                  ╱
                 ╱
1.0 ────────────╱
```

During the crossfade:

```c
output =
    current_chord * current_weight +
    next_chord    * next_weight;
```

with the weights changing over the crossfade region.

---

## Planned Architecture

The current implementation writes every generated sample directly to the output file:

```text
Generate sample
      ↓
fwrite()
      ↓
file
```

Implementing a crossfade requires access to samples from **both chords at the same time**.

The planned approach is therefore to generate/process samples in a small buffer rather than immediately writing every sample.

Conceptually:

```text
Generate current chord ───────┐
                              │
                              ├── Crossfade ──→ Output buffer
Generate next chord ──────────┘
                                      ↓
                                   fwrite()
```

Outside the crossfade region, the current chord can have a weight of `1.0`.

Inside the crossfade:

```text
current_weight ↓
next_weight    ↑
```

The resulting samples are then written to the output.

---

## Why This Project?

This project is being built as a hands-on exercise in:

* C programming
* Arrays and multidimensional arrays
* File I/O
* `fwrite()` and binary data
* Floating-point arithmetic
* Sine-wave generation
* Digital audio
* PCM encoding
* Sampling rates
* Audio buffers
* Signal mixing
* Crossfading

The goal is not just to generate a sound file, but to understand what is actually happening between a mathematical waveform and the bytes being written to disk.

---

## Building

The project requires a C compiler with support for the standard C library and the math library.

For GCC:

```bash
gcc main.c -o chord_generator -lm
```

On Windows with MinGW:

```powershell
gcc main.c -o chord_generator.exe -lm
```

Run:

```powershell
.\chord_generator.exe
```

The program will generate:

```text
sine_cfamg.raw
```

in the current working directory.

---

## Current Status

* [x] Generate sine waves
* [x] Generate three-note chords
* [x] Generate multiple chords sequentially
* [x] Write 16-bit PCM samples to a raw file
* [x] Define a crossfade region
* [ ] Implement crossfade
* [ ] Buffer samples during transitions
* [ ] Produce a WAV file with a proper header
* [ ] Experiment with different crossfade curves
* [ ] Experiment with different chord progressions

---

## Learning Notes

One of the main ideas being explored in this project is that an audio file is ultimately just a sequence of samples.

For this project:

```text
time
 ↓
sine waves
 ↓
mix frequencies
 ↓
PCM sample
 ↓
16-bit integer
 ↓
binary file
```

Crossfading is therefore not fundamentally a "file" operation. It is a **sample-level signal processing operation** performed before those samples are written to the file.

That distinction is one of the main concepts this project is intended to explore.

#pragma once

#include <QObject>
struct sonicStreamStruct;
typedef struct sonicStreamStruct *sonicStream;

class SonicCpp
{
public:
    /* Create a sonic stream.  Return NULL only if we are out of memory and cannot
    allocate the stream. Set numChannels to 1 for mono, and 2 for stereo. */
    static sonicStream sonicCreateStream(int sampleRate, int numChannels);
    /* Destroy the sonic stream. */
    static void sonicDestroyStream(sonicStream stream);
    /* Use this to write floating point data to be speed up or down into the stream.
    Values must be between -1 and 1.  Return 0 if memory realloc failed, otherwise 1 */
    static int sonicWriteFloatToStream(sonicStream stream, float *samples, int numSamples);
    /* Use this to write 16-bit data to be speed up or down into the stream.
    Return 0 if memory realloc failed, otherwise 1 */
    static int sonicWriteShortToStream(sonicStream stream, short *samples, int numSamples);
    /* Use this to write 8-bit unsigned data to be speed up or down into the stream.
    Return 0 if memory realloc failed, otherwise 1 */
    static int sonicWriteUnsignedCharToStream(sonicStream stream, unsigned char *samples, int numSamples);
    /* Use this to read floating point data out of the stream.  Sometimes no data
    will be available, and zero is returned, which is not an error condition. */
    static int sonicReadFloatFromStream(sonicStream stream, float *samples, int maxSamples);
    /* Use this to read 16-bit data out of the stream.  Sometimes no data will
    be available, and zero is returned, which is not an error condition. */
    static int sonicReadShortFromStream(sonicStream stream, short *samples, int maxSamples);
    /* Use this to read 8-bit unsigned data out of the stream.  Sometimes no data will
    be available, and zero is returned, which is not an error condition. */
    static int sonicReadUnsignedCharFromStream(sonicStream stream, unsigned char *samples, int maxSamples);
    /* Force the sonic stream to generate output using whatever data it currently
    has.  No extra delay will be added to the output, but flushing in the middle of
    words could introduce distortion. */
    static int sonicFlushStream(sonicStream stream);
    /* Return the number of samples in the output buffer */
    static int sonicSamplesAvailable(sonicStream stream);
    /* Get the speed of the stream. */
    static float sonicGetSpeed(sonicStream stream);
    /* Set the speed of the stream. */
    static void sonicSetSpeed(sonicStream stream, float speed);
    /* Get the pitch of the stream. */
    static float sonicGetPitch(sonicStream stream);
    /* Set the pitch of the stream. */
    static void sonicSetPitch(sonicStream stream, float pitch);
    /* Get the rate of the stream. */
    static float sonicGetRate(sonicStream stream);
    /* Set the rate of the stream. */
    static void sonicSetRate(sonicStream stream, float rate);
    /* Get the scaling factor of the stream. */
    static float sonicGetVolume(sonicStream stream);
    /* Set the scaling factor of the stream. */
    static void sonicSetVolume(sonicStream stream, float volume);
    /* Get the chord pitch setting. */
    static int sonicGetChordPitch(sonicStream stream);
    /* Set chord pitch mode on or off.  Default is off.  See the documentation
    page for a description of this feature. */
    static void sonicSetChordPitch(sonicStream stream, int useChordPitch);
    /* Get the quality setting. */
    static int sonicGetQuality(sonicStream stream);
    /* Set the "quality".  Default 0 is virtually as good as 1, but very much faster. */
    static void sonicSetQuality(sonicStream stream, int quality);
    /* Get the sample rate of the stream. */
    static int sonicGetSampleRate(sonicStream stream);
    /* Set the sample rate of the stream.  This will drop any samples that have not been read. */
    static void sonicSetSampleRate(sonicStream stream, int sampleRate);
    /* Get the number of channels. */
    static int sonicGetNumChannels(sonicStream stream);
    /* Set the number of channels.  This will drop any samples that have not been read. */
    static void sonicSetNumChannels(sonicStream stream, int numChannels);
    /* This is a non-stream oriented interface to just change the speed of a sound
    sample.  It works in-place on the sample array, so there must be at least
    speed*numSamples available space in the array. Returns the new number of samples. */
    static int sonicChangeFloatSpeed(float *samples, int numSamples, float speed, float pitch,
                                     float rate, float volume, int useChordPitch, int sampleRate, int numChannels);
    /* This is a non-stream oriented interface to just change the speed of a sound
    sample.  It works in-place on the sample array, so there must be at least
    speed*numSamples available space in the array. Returns the new number of samples. */
    static int sonicChangeShortSpeed(short *samples, int numSamples, float speed, float pitch,
                                     float rate, float volume, int useChordPitch, int sampleRate, int numChannels);
};


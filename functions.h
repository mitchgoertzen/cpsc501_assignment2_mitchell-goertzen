#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <complex>
#include <vector>

int getFileSize(FILE* inFile);

typedef struct HEADER
{
    uint8_t         ChunkID[4];
    uint32_t        ChunkSize;
    uint8_t         Format[4];
    uint8_t         Subchunk1ID[4];
    uint32_t        Subchunk1Size;
    uint16_t        AudioFormat;
    uint16_t        NumChannels;
    uint32_t        SampleRate;
    uint32_t        ByteRate;
    uint16_t        BlockAlign;
    uint16_t        BitsPerSample;
    uint8_t         Subchunk2ID[4];
    uint32_t        Subchunk2Size;
} myHeader;

void convolve(double* INPUT, double* IR, int inpSize, int irSize, int channels);

std::vector<double> readWavFile(int *arraySize, int *channels, char *filename);	
void readWavFileHeader(int *channels, int *numSamples, FILE *inputFile);

void writeWavFile(double *outputArray, int outputArraySize, int channels, char *filename);
void writeWavFileHeader(int channels, int numberSamples, double outputRate, FILE *outputFile);

size_t fwriteIntLSB(int data, FILE *stream);
int freadIntLSB(FILE *stream);
size_t fwriteShortLSB(short int data, FILE *stream);
short int freadShortLSB(FILE *stream);

#endif
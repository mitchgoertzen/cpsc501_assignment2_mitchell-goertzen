#ifndef FUNCTIONS_H
#define FUNCTIONS_H


double* readWavFile(int *arraySize, int *channels, char *filename);	
void readWavFileHeader(int *channels, int *numSamples, FILE *inputFile);

void writeWavFile(double *outputArray, int outputArraySize, int channels, char *filename);
void writeWavFileHeader(int channels, int numberSamples, double outputRate, FILE *outputFile);

size_t fwriteIntLSB(int data, FILE *stream);
int freadIntLSB(FILE *stream);
size_t fwriteShortLSB(short int data, FILE *stream);
short int freadShortLSB(FILE *stream);

#endif
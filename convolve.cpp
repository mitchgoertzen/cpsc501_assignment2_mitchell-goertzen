/*
	Produces a single tone, and writes the result in .wav file format to the filename given as user input
	Note that the specified filename should have a .wav ending
	
	Usage: ./testtone outputFile
	
	Contains functions for writing .wav files that can be used for the assignment
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>	// includes sin
#include <string>
#include <fstream>
#include "functions.h"
#include <iostream>

// CONSTANTS ******************************

#define PI              	3.14159265358979

// Frequency of tone to be created (A = 440 Hz)
#define TONE_FREQUENCY		440

// Duration of tone to be created (3 seconds for now)
#define SECONDS				3

//  Standard sample rate in Hz
#define SAMPLE_RATE     	44100.0

//  Standard sample size in bits
#define BITS_PER_SAMPLE		16

// Standard sample size in bytes		
#define BYTES_PER_SAMPLE	(BITS_PER_SAMPLE/8)

// Rescaling factor to convert between 16-bit shorts and doubles between -1 and 1
#define MAX_SHORT_VALUE		32768

// Number of channels
#define MONOPHONIC			1
#define STEREOPHONIC		2

// Offset of the fmt chunk in the WAV header
#define FMT_OFFSET			12

using namespace std;
char *outputFilename;
int main(int argc, char **argv) {
	

	if (argc < 4) {
		printf("Wrong input\n");
		exit(-1);
	}

    char *inputFilename;
	inputFilename = argv[1];

    char *irFilename;
	irFilename = argv[2];

	outputFilename = argv[3];

    FILE* inputFile = fopen(inputFilename, "r");
    if (inputFile == nullptr)
    {
        fprintf(stderr, "Unable to open wav file: %s\n", inputFilename);
        return 1;
    }

    FILE* irFile = fopen(irFilename, "r");
    if (irFile == nullptr)
    {
        fprintf(stderr, "Unable to open IR file: %s\n", irFilename);
        return 1;
    }

	int inputChannels;
    int inputSamples;

    int irChannels;
    int irSamples;

    printf("Reading wav file %s...\n", inputFilename);
    readWavFileHeader(&inputChannels, &inputSamples, inputFile);

    printf("Reading IR file %s...\n", irFilename);
    readWavFileHeader(&irChannels, &irSamples, irFile);

	double *inputArray = readWavFile(&inputSamples, &inputChannels, inputFilename);
    double *irArray = readWavFile(&irSamples, &irChannels, irFilename);

    convolve(inputArray, irArray, inputSamples, irSamples, inputChannels);
    
    printf("Finished");

}

void convolve(double* INPUT, double* irData, int inpSize, int irSize, int channels){

    int outputChannels = channels;
    int outputSize = inpSize + irSize - 1;
    double *outputArray = new double[outputSize];

    for(int i = 0;i < irSize;i++){
        for(int j = 0;j < inpSize;j++){
            outputArray[i+j] += irData[i] * INPUT[j];
        }
    }

    double largestDouble = 1;
    for (int i=0; i< outputSize; i++) {
		if (abs(outputArray[i]) > largestDouble) {
			largestDouble = abs(outputArray[i]);
		}
    }

    for (int i=0; i<outputSize; i++) {
		outputArray[i] = (outputArray[i]/largestDouble);
    }

    printf("Writing result to file %s...\n", outputFilename);
    writeWavFile(outputArray, outputSize, outputChannels, outputFilename);
}

void readWavFileHeader(int *channels, int *numSamples, FILE *inputFile){

    myHeader wavHeader;
    int headerSize = sizeof(myHeader);

    size_t bytesRead = fread(&wavHeader, 1, headerSize, inputFile);
    if (bytesRead > 0)
    {
        int numberSamples = (int) (wavHeader.ChunkSize / ( wavHeader.BitsPerSample / 8));
        int duration = numberSamples/ wavHeader.SampleRate;
        numberSamples = duration * wavHeader.SampleRate;
        *numSamples = numberSamples;
    }

    fclose(inputFile);
    return;
}

double* readWavFile(int *arraySize, int *channels, char *filename){
    
    int numChannels = *channels;
    int size = *arraySize;
    double *outputArray = new double[size];
    int16_t *buffer = new int16_t[size];

    FILE* inp = fopen(filename, "r");

    fread(buffer, 2, size, inp);

    for (int i = 0; i < size; i++) {
        outputArray[i] = buffer[i];
    }


    delete buffer;

    return outputArray;
}

/*
Writes the header for a WAV file with the given attributes to 
 the provided filestream
*/

void writeWavFileHeader(int channels, int numberSamples, double outputRate, FILE *outputFile) {
    // Note: channels is not currently used. You will need to add this functionality
	// yourself for the bonus part of the assignment
	
	/*  Calculate the total number of bytes for the data chunk  */
    int dataChunkSize = numberSamples * BYTES_PER_SAMPLE;
	
    /*  Calculate the total number of bytes for the form size  */
    int formSize = 36 + dataChunkSize;
	
    /*  Calculate the total number of bytes per frame  */
    short int frameSize = channels * BYTES_PER_SAMPLE;
	
    /*  Calculate the byte rate  */
    int bytesPerSecond = (int)ceil(outputRate * frameSize);

    /*  Write header to file  */
    /*  Form container identifier  */
    fputs("RIFF", outputFile);
      
    /*  Form size  */
    fwriteIntLSB(formSize, outputFile);
      
    /*  Form container type  */
    fputs("WAVE", outputFile);

    /*  Format chunk identifier (Note: space after 't' needed)  */
    fputs("fmt ", outputFile);
      
    /*  Format chunk size (fixed at 16 bytes)  */
    fwriteIntLSB(16, outputFile);

    /*  Compression code:  1 = PCM  */
    fwriteShortLSB(1, outputFile);

    /*  Number of channels  */
    fwriteShortLSB((short) MONOPHONIC, outputFile);

    /*  Output Sample Rate  */
    fwriteIntLSB((int)outputRate, outputFile);

    /*  Bytes per second  */
    fwriteIntLSB(bytesPerSecond, outputFile);

    /*  Block alignment (frame size)  */
    fwriteShortLSB(frameSize, outputFile);

    /*  Bits per sample  */
    fwriteShortLSB(BITS_PER_SAMPLE, outputFile);

    /*  Sound Data chunk identifier  */
    fputs("data", outputFile);

    /*  Chunk size  */
    fwriteIntLSB(dataChunkSize, outputFile);
}


/*
Creates a WAV file with the contents of the provided outputArray as the samples, and writes
it to the given filename
 */

void writeWavFile(double *outputArray, int outputArraySize, int channels, char *filename) {
    // Note: channels is not currently used. You will need to add this functionality
	// yourself for the bonus part of the assignment

  //open a binary output file stream for writing
    FILE *outputFileStream = fopen(filename, "wb");
    if (outputFileStream == NULL) {
      printf("File %s cannot be opened for writing\n", filename);
        return;
    }

    //create an 16-bit integer array to hold rescaled samples
    short *intArray = new short[outputArraySize];

    //find the largest entry and uses that to rescale all other
    // doubles to be in the range (-1, 1) to prevent 16-bit integer overflow
    double largestDouble = 1;
    for (int i=0; i< outputArraySize; i++) {
		if (abs(outputArray[i]) > largestDouble) {
			largestDouble = abs(outputArray[i]);
		}
    }

    for (int i=0; i<outputArraySize; i++) {
		intArray[i] = (short) ((outputArray[i]/largestDouble)*MAX_SHORT_VALUE);
    }
	
    int numSamples = outputArraySize;

	// actual file writing
    writeWavFileHeader(channels, numSamples, SAMPLE_RATE, outputFileStream);
    fwrite(intArray, sizeof(short), outputArraySize, outputFileStream);
    
    //clear memory from heap
    delete[] intArray;
}


//writes an integer to the provided stream in little-endian form
size_t fwriteIntLSB(int data, FILE *stream) {
    unsigned char array[4];

    array[3] = (unsigned char)((data >> 24) & 0xFF);
    array[2] = (unsigned char)((data >> 16) & 0xFF);
    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    return fwrite(array, sizeof(unsigned char), 4, stream);
}


//reads an integer from the provided stream in little-endian form
int freadIntLSB(FILE *stream) {
    unsigned char array[4];

    fread(array, sizeof(unsigned char), 4, stream);
    
    int data;
    data = array[0] | (array[1] << 8) | (array[2] << 16) | (array[3] << 24);

    return data;
}


//writes a short integer to the provided stream in little-endian form
size_t fwriteShortLSB(short int data, FILE *stream) {
    unsigned char array[2];

    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    return fwrite(array, sizeof(unsigned char), 2, stream);
}


//reads a short integer from the provided stream in little-endian form
short int freadShortLSB(FILE *stream) {
    unsigned char array[2];

    fread(array, sizeof(unsigned char), 2, stream);
    
    int data;
    data = array[0] | (array[1] << 8);

    return data;
}
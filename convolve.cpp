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



int main(int argc, char **argv) {
	char *outputFilename;

	if (argc != 2) {
		printf("Please enter a filename\n");
		exit(-1);
	}

	outputFilename = argv[1];
	
	// a good exercise would be to convert this to stereophonic
	int outputChannels = MONOPHONIC;
	
	int outputArraySize = (int) SECONDS * SAMPLE_RATE;	// creates a 3-second tone
	double *outputArray = new double[outputArraySize];

	// create a pure-tone A, which is a sine wave with period of 440 Hz
	// and input samples to the array
	for (int i = 0; i < outputArraySize; i++) {
		outputArray[i] = sin(2 * PI * TONE_FREQUENCY * i / SAMPLE_RATE);
	}
	
	printf("Writing result to file %s...\n", outputFilename);
    writeWavFile(outputArray, outputArraySize, outputChannels, outputFilename);
    
    printf("Finished");

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
		if (outputArray[i] > largestDouble) {
			largestDouble = outputArray[i];
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
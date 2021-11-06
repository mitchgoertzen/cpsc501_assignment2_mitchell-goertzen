/*
FFT source: https://cp-algorithms.com/algebra/fft.html
        replaced use of C++ built-in complex class, because
        I wasn't sure if I was allowed to use it
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>	// includes sin
#include <string>
#include <fstream>
#include "complex_functions.h"
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

using cl = std::vector<std::pair<double, double>>;
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

	std::vector<double> inputVector = readWavFile(&inputSamples, &inputChannels, inputFilename);
    std::vector<double> irVector = readWavFile(&irSamples, &irChannels, irFilename);

    cl complexInput = realToComplex(inputVector);
    cl complexIR = realToComplex(irVector);

    int maxSize = max(complexInput.size(), complexIR.size());
    int n = 1;
    while(n < maxSize){
        n*=2;
    }

    complexInput.resize(n);
    complexIR.resize(n);

    cl outputVector = convolveWithFFT(complexInput, complexIR);
    double* outputArray = new double[n];

    for(int i = 0; i < n;i++){
        outputArray[i] = outputVector[i].first;
    } 

    writeWavFile(outputArray, n, inputChannels, outputFilename);
    
    printf("Finished");

}

cl realToComplex(std::vector<double> const& a){

    int n = a.size();
    cl complexOut;
    //Optimization 1: Partial Unroll Loop
    int i;
    for(i = 0; i < n-2;i+=3){
        complexOut.push_back(make_pair(a[i], 0));
        complexOut.push_back(make_pair(a[i+1], 0));
        complexOut.push_back(make_pair(a[i+2], 0));
    }
    if(i == n - 1){
        complexOut.push_back(make_pair(a[i], 0));
    } else if(i == n - 2){
        complexOut.push_back(make_pair(a[i], 0));
        complexOut.push_back(make_pair(a[i+1], 0));
    }
    return complexOut;
}

std::pair<double, double> multiply(std::pair<double, double> const& a, std::pair<double, double> const& b) {

    std::pair<double, double> C;

        C.first = (a.first * b.first) - (a.second * b.second);
        C.second = (a.second * b.first) + (a.first * b.second);
 
    return C;
}

cl convolveWithFFT(cl const& a, cl const& b) {

    cl A = a;
    cl B = b;

    int n = A.size();
    cl C(n);

    fft(A, 1);
    fft(B, 1);

    for(int i = 0; i < n;i++){
        C[i] = multiply(A[i], B[i]);
    }

    fft(C, -1);
 
    return C;
}


void fft(cl & A, int direction) {

    int n = A.size();
    if (n == 1)
        return;

    cl even(n / 2);
    cl odd(n / 2);


    for (int i = 0; 2 * i < n; i++) {
        even[i] = A[2*i];
        odd[i] = A[2*i+1];
    }
    
    fft(even, direction);
    fft(odd, direction);

    double theta = 2 * PI / n * direction;
    std::pair<double, double> omega(1, 0);
    std::pair<double, double> omegan(cos(theta), sin(theta));
    for (int i = 0; 2 * i < n; i++) {

        std::pair<double, double> temp = multiply(omega, odd[i]);
        A[i].first = even[i].first + temp.first;
        A[i].second = even[i].second + temp.second;

        A[i + n/2].first = even[i].first - temp.first;
        A[i + n/2].second = even[i].second - temp.second;

        if (direction == -1) {
            A[i].first /= 2;
            A[i].second /= 2;
            A[i + n/2].first /= 2;
            A[i + n/2].second /= 2;
        }
        omega = multiply(omega, omegan);
    }
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

std::vector<double> readWavFile(int *arraySize, int *channels, char *filename){
    
    int numChannels = *channels;
    int size = *arraySize;
    std::vector<double> outputArray;
    int16_t *buffer = new int16_t[size];

    FILE* inp = fopen(filename, "r");

    fread(buffer, 2, size, inp);

    for (int i = 0; i < size; i++) {
        outputArray.push_back(buffer[i]);
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
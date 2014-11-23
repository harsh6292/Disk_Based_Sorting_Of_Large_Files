#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>

int performBasicMergesort(char* indexFileName, char* sortedFileName, int* inputBuffer, int* outputBuffer);
int compareKeys(const int* key1, const int* key2);
int createSortedRuns(int *inputBuffer, char *indexFileName, int *lastFileOffsetUntil, int *noOfRuns);
void mergeSortedRuns(int noOfRunsToBeRead, int *inputBuffer, int *outputBuffer, char *inputFileName, char *sortedFileName, int additionToFileName);
int performMultistepMergesort(char* indexFileName, char* sortedFileName, int* inputBuffer, int* outputBuffer);
int performReplacementMergesort(char* inputFileName, char* sortedFileName, int* inputBuffer, int* outputBuffer);
double getAccurateTime(struct timeval startTime, struct timeval endTime);
void heapify(int *inputBuffer, int noOfElements, int startPos);
void siftElement(int *inputBuffer, int index, int heapSize);

struct timeval startTime, endTime;



int main(int argc, char** argv)
{
	FILE *indexFile, *sortedFile;

	char readRecord[200],*indexFileName, *sortedFileName;
	int *inputBuffer, *outputBuffer;
	double accTime;

	if(argc!=4)
	{
		printf("\nToo few arguments. Expected 3 arguments. Found only %d.\n", (argc-1));
		return 0;
	}
	
	indexFileName = argv[2];
	sortedFileName = argv[3];
	
	inputBuffer = (int*)malloc(sizeof(int)*1000);
	outputBuffer = (int*)malloc(sizeof(int)*1000);


	if( strcmp(argv[1], "--basic") == 0 )
	{

		gettimeofday(&startTime, NULL);
		performBasicMergesort(indexFileName, sortedFileName, inputBuffer, outputBuffer);
		gettimeofday( &endTime, NULL );


		accTime = getAccurateTime(startTime, endTime);
		printf( "Time: %0.6f\n", accTime );

	}
	else if( strcmp(argv[1], "--multistep") == 0 )
	{

		gettimeofday(&startTime, NULL);
		performMultistepMergesort(indexFileName, sortedFileName, inputBuffer, outputBuffer);
		gettimeofday( &endTime, NULL );


		accTime = getAccurateTime(startTime, endTime);
		printf( "Time: %0.6f\n", accTime );
	}
	else if( strcmp(argv[1], "--replacement") == 0 )
	{

		gettimeofday(&startTime, NULL);
		performReplacementMergesort(indexFileName, sortedFileName, inputBuffer, outputBuffer);
		gettimeofday( &endTime, NULL );


		accTime = getAccurateTime(startTime, endTime);
		printf( "Time: %0.6f\n", accTime );
	}

	return(0);
}



int performBasicMergesort(char* indexFileName, char* sortedFileName, int* inputBuffer, int* outputBuffer)
{
	FILE *indexFile, *sortedRun, *outputSortedFile;
	char tempIndexFile[1000], runID[10];
	char positionToBeConsidered[1000];

	int noOfRuns=0, runBufferSize = 0, i=0, j=0, min = 0, outputBufferCount = 0, positionOfMin = 0;
	int areAllRunsExhausted = 0, secCount = 0, bufferSize = 1000, noOfValuesRead = 0, minValueToWrite = -1;
	int lastFileOffsetUntil = 0, countValidEntriesOfLastFile = 0, outputbufferWriteCount = 0, getValue = 0;



	//Open Sorted file for writing
	if( (outputSortedFile = fopen(sortedFileName, "w+b")) == NULL )
	{
		printf("\nCannot Open Sorted File!");
		return(0);
	}



	//read into buffer values from input.bin and create runs of 1000 keys each
	getValue = createSortedRuns(inputBuffer, indexFileName, &lastFileOffsetUntil, &noOfRuns);
	if(getValue == 0)
	{
		printf("\nIndex File cannot be opened");
		return(0);
	}


	if(noOfRuns == 0)
	{
		return(0);
	}

	FILE *inputRuns[noOfRuns];
	int eachRunBufferPC[noOfRuns], eachRunOffset[noOfRuns];
	runBufferSize = (1000/noOfRuns);

	mergeSortedRuns(noOfRuns, inputBuffer, outputBuffer, indexFileName, sortedFileName, 0);

	return(0);
}




int compareKeys(const int* key1, const int* key2)
{
	if( *key1 > *key2)
		return(1);
	else if(*key1 == *key2)
		return(0);
	else if(*key1 < *key2)
		return(-1);

}




double getAccurateTime(struct timeval startTime, struct timeval endTime)
{
	double accTime = ( ((double)endTime.tv_sec + 1.0e-6*endTime.tv_usec) - ((double)startTime.tv_sec + 1.0e-6*startTime.tv_usec) );

	return(accTime);
}



int createSortedRuns(int *inputBuffer, char *indexFileName, int *lastFileOffsetUntil, int *noOfRuns)
{
	FILE *indexFile = NULL, *sortedRun = NULL;
	int bufferSize = 1000, noOfValuesRead = 0, count = 0;
	char tempIndexFile[1000], runID[10];
	

	//Open the Unsorted Index File
	if( (indexFile = fopen(indexFileName, "r+b")) == NULL )
	{
		printf("\nCannot Open Index File!");
		return(0);
	}


	//read into buffer values from input.bin and create runs of 1000 keys each
	while( ( noOfValuesRead = fread(inputBuffer, sizeof(int), bufferSize, indexFile) )!=0 )
	{
		snprintf(tempIndexFile, (2*bufferSize), "%s.%03d",indexFileName,count);
		sortedRun = fopen(tempIndexFile, "w+b");
	

		//if last file contains values less than buffersize(1000), then sort accordingly
		//and make note of offset upto last file is to be read
		if( noOfValuesRead < bufferSize )
		{
			qsort(inputBuffer, noOfValuesRead, sizeof(int), compareKeys);

			fwrite(inputBuffer, sizeof(int), noOfValuesRead, sortedRun);
			*lastFileOffsetUntil = noOfValuesRead;
		}
		else
		{
			//else write whole file accordingly
			qsort(inputBuffer, bufferSize, sizeof(int), compareKeys);
			fwrite(inputBuffer, sizeof(int), bufferSize, sortedRun);
			*lastFileOffsetUntil = bufferSize;
		}
		

		fclose(sortedRun);
		
		count++;
	}

	*noOfRuns = count;
	fclose(indexFile);
	return(1);
}





void mergeSortedRuns(int noOfRunsToBeRead, int *inputBuffer, int *outputBuffer, char *inputFileName, char *sortedFileName, int additionToFileName)
{

	FILE 	*outputSortedFile, *inputRuns[noOfRunsToBeRead];

	int 	runBufferSize = 0, i = 0, outputBufferCount = 0, countValidEntriesOfLastFile = 0;
	int 	areAllRunsExhausted = 0, bufferSize = 1000;
	int 	actualBufferReadByEachRun[noOfRunsToBeRead], min = 0, positionOfMin = 0;
	int 	eachRunBufferPC[noOfRunsToBeRead], eachRunOffset[noOfRunsToBeRead];
	
	char 	tempFileName[2000], runID[10];


	runBufferSize = (1000/noOfRunsToBeRead);
	
	//now open each super sorted run file, buffer them and sort them
	//Open Sorted file for writing
	if( (outputSortedFile = fopen(sortedFileName, "w+b")) == NULL )
	{
		printf("\nCannot Open Sorted File!");
		return(0);
	}

	for( i = 0; i < noOfRunsToBeRead; i++)
	{
		snprintf(tempFileName, (2*bufferSize), "%s.%03d",inputFileName, (i+additionToFileName));
		inputRuns[i] = fopen(tempFileName, "r+b");
		actualBufferReadByEachRun[i] = fread( (inputBuffer+(i*runBufferSize) ), sizeof(int), runBufferSize, inputRuns[i] );
		eachRunBufferPC[i] = 0;
	}


	while(1)
	{
		min = INT_MAX;

		//for each run buffer, get the minimum value
		for( i = 0; i<noOfRunsToBeRead; i++)
		{
			
			if( eachRunBufferPC[i]!= -1 )
			{
				if( inputBuffer[(eachRunBufferPC[i]) +(runBufferSize*i)] <= min )
				{
					min = inputBuffer[(eachRunBufferPC[i]) +(runBufferSize*i)];
					positionOfMin = i;
				}
			} 
	
		}


		//got the min value, update output buffer
		outputBuffer[outputBufferCount] = min;
		outputBufferCount++;
		eachRunBufferPC[positionOfMin] += 1;


		if( eachRunBufferPC[positionOfMin] == actualBufferReadByEachRun[positionOfMin] && actualBufferReadByEachRun[positionOfMin]>0 )
		{
			if( ( actualBufferReadByEachRun[positionOfMin] = fread( (inputBuffer+(positionOfMin*runBufferSize) ), sizeof(int), runBufferSize, inputRuns[positionOfMin] ) ) != 0)
			{
				eachRunBufferPC[positionOfMin] = 0;
			}	
			else
			{
				eachRunBufferPC[positionOfMin] = -1;
			}
		}


		//check if outputbuffer is full(=buffersize)
		if( outputBufferCount == bufferSize )
		{
			fwrite(outputBuffer, sizeof(int), outputBufferCount, outputSortedFile);
			outputBufferCount = 0;
		}


		//check if all runs exhausted
		for( i=0; i<noOfRunsToBeRead; i++)
		{
			if( eachRunBufferPC[i] != -1 )
			{
				//Runs are not exhausted yet
				areAllRunsExhausted = 1;
				break;
			}
			else
				areAllRunsExhausted = 0;
		}


		//if all runs are exhausted, then its time to write last buffer to file
		//secondly exit the program
		if( areAllRunsExhausted == 0)
		{
			if(outputBufferCount > 0)
			{
				fwrite(outputBuffer, sizeof(int), outputBufferCount, outputSortedFile);
			}

			break;
		}
	}

	for( i = 0; i < noOfRunsToBeRead; i++)
	{
		fclose(inputRuns[i]);
	}

	fclose(outputSortedFile);

}



int performMultistepMergesort(char* indexFileName, char* sortedFileName, int* inputBuffer, int* outputBuffer)
{
	FILE *indexFile, *sortedRun, *outputSortedFile;
	char tempIndexFile[2000], runID[10], tempSuperRunFile[2000];
	

	int noOfRuns=0, runBufferSize = 0, i=0, j=0, min = 0, outputBufferCount = 0, positionOfMin = 0;
	int areAllRunsExhausted = 0, secCount = 0, bufferSize = 1000, noOfValuesRead = 0, minValueToWrite = -1;
	int lastFileOffsetUntil = 0, countValidEntriesOfLastFile = 0, outputbufferWriteCount = 0, getValue = 0;
	int noOfRunsInSuperRuns = 15, noOfSuperRuns = 0;;


	//read into buffer values from input.bin and create runs of 1000 keys each
	getValue = createSortedRuns(inputBuffer, indexFileName, &lastFileOffsetUntil, &noOfRuns);
	if(getValue == 0)
	{
		printf("\nIndex File cannot be opened");
		return(0);
	}


	if(noOfRuns == 0)
	{
		return(0);
	}
	

	if( noOfRuns <= noOfRunsInSuperRuns )
		noOfRunsInSuperRuns = noOfRuns;

	noOfSuperRuns = (noOfRuns/noOfRunsInSuperRuns);


	if( (noOfRuns % noOfRunsInSuperRuns ) !=0 )
	{
		noOfSuperRuns += 1;
	}


	FILE *inputRuns[noOfRunsInSuperRuns], *inputSuperRuns[noOfSuperRuns];	
	int eachRunBufferPC[noOfRunsInSuperRuns], eachRunOffset[noOfRunsInSuperRuns];
	int actualBufferReadByEachRun[noOfRunsInSuperRuns];



	//open each sorted runs and buffer values for each run
	for( i = 0; i< noOfSuperRuns; i++ )
	{
		snprintf(tempIndexFile, (2*bufferSize), "%s.super.%03d",indexFileName,i);

	    if( (noOfRunsInSuperRuns*(i+1)) < noOfRuns)
	    {
	    	mergeSortedRuns( noOfRunsInSuperRuns, inputBuffer, outputBuffer, indexFileName, tempIndexFile, (noOfRunsInSuperRuns*i));
	    }
	    else
	    {
	    	mergeSortedRuns( (noOfRuns- (noOfRunsInSuperRuns*i) ), inputBuffer, outputBuffer, indexFileName, tempIndexFile, (noOfRunsInSuperRuns*i));
	    }

	}


	snprintf(tempSuperRunFile, (2*bufferSize), "%s.super",indexFileName);
	mergeSortedRuns( noOfSuperRuns, inputBuffer, outputBuffer, tempSuperRunFile, sortedFileName, 0);

	return(0);

}




int performReplacementMergesort(char* inputFileName, char* sortedFileName, int* inputBuffer, int* outputBuffer)
{

	FILE *inputFile, *inputSortedRuns, *outputSortedFile;

	int primaryHeapIndex = 0, secondaryHeapIndex = 749, readInputIndex = 750, bufferSize = 1000;	

	int sizeOfHeap = 0, actualInputBufferRead = 0, sizeofSecondaryHeap = 0;
	int outputBufferCount = 0, outputbufferWriteCount = 0, noOfRunsCreated = 0;
	int tempHeapIndex = 0, valuesRead = 0, opBufferWrote = 0, inBufferRead = 0;
	int inputFileFullyRead = 0, k=0, j=0;
	int tempPrimaryIndexForSwitching = 0, tempSecondaryIndexForSwitching = 0;

	char tempSuperRunFile[1000], runID[10];

	if( (inputFile = fopen(inputFileName, "r+b")) == NULL )
	{
		printf("\nCannot Open Input file");
		return(0);
	}


	
	sizeOfHeap = fread(inputBuffer, sizeof(int), readInputIndex, inputFile);

	if(sizeOfHeap == 0)
	{
		
		if( (outputSortedFile = fopen(sortedFileName, "w+b")) == NULL )
		{
			printf("\nCannot Open Sorted File!");
			return(0);
		}
		fclose(outputSortedFile);
		return(0);
	}

	actualInputBufferRead = fread(inputBuffer+readInputIndex, sizeof(int), (bufferSize-readInputIndex), inputFile);
	inBufferRead += actualInputBufferRead;
	inBufferRead += sizeOfHeap;

	heapify(inputBuffer, sizeOfHeap, 0);
	

	snprintf(tempSuperRunFile, (2*bufferSize), "%s.%03d",inputFileName, noOfRunsCreated);
	inputSortedRuns = fopen(tempSuperRunFile, "w+b");


	while(1)
	{


		if(sizeOfHeap > 0)
		{
			

			if(actualInputBufferRead > 0)
			{

				if(primaryHeapIndex == 0)
				{
					outputBuffer[outputBufferCount] = inputBuffer[primaryHeapIndex];

				}
				else
				{
					outputBuffer[outputBufferCount] = inputBuffer[tempPrimaryIndexForSwitching]; //(primaryHeapIndex - sizeOfHeap+1)]; 
				}
				

				valuesRead++;	

				if( primaryHeapIndex == 0 && inputBuffer[primaryHeapIndex] <= inputBuffer[readInputIndex] )
				{

					inputBuffer[primaryHeapIndex] = inputBuffer[readInputIndex];

					readInputIndex++;

					siftElement(inputBuffer, primaryHeapIndex, (sizeOfHeap));
					
				}
				else if( primaryHeapIndex == 749 && inputBuffer[tempPrimaryIndexForSwitching] <= inputBuffer[readInputIndex] )
				{

					inputBuffer[tempPrimaryIndexForSwitching] = inputBuffer[readInputIndex];

					readInputIndex++;

					siftElement(inputBuffer, tempPrimaryIndexForSwitching, (sizeOfHeap) );					
				}
				else
				{
					if(sizeOfHeap>0)
					{
						if(primaryHeapIndex == 0)
						{
	
							inputBuffer[primaryHeapIndex] = inputBuffer[(primaryHeapIndex+sizeOfHeap-1)];
						}
						else
						{
							inputBuffer[tempPrimaryIndexForSwitching] = inputBuffer[(tempPrimaryIndexForSwitching + sizeOfHeap-1)]; //primaryHeapIndex];
						}
					}


					sizeOfHeap = (sizeOfHeap - 1);


					if( secondaryHeapIndex != 0)
					{
						inputBuffer[(secondaryHeapIndex - sizeofSecondaryHeap)] = inputBuffer[readInputIndex];
					}
					else
					{
						inputBuffer[(tempSecondaryIndexForSwitching - sizeofSecondaryHeap)] = inputBuffer[readInputIndex];
					}


					sizeofSecondaryHeap++;
					readInputIndex++;

					if( primaryHeapIndex == 0)
					{
						siftElement(inputBuffer, primaryHeapIndex, (sizeOfHeap));

					}
					else
					{
						
						siftElement(inputBuffer, tempPrimaryIndexForSwitching, (sizeOfHeap) );
					}


				}
				outputBufferCount++;

			}


			if( outputBufferCount == bufferSize )
			{
				fwrite(outputBuffer, sizeof(int), outputBufferCount, inputSortedRuns);

				//count to keep track of buffers, so that last buffer can be write out in end
				outputbufferWriteCount+=1;
				opBufferWrote += bufferSize;

				outputBufferCount = 0;
			}		


			if( readInputIndex == (actualInputBufferRead+750))
			{
				readInputIndex = 750;
				actualInputBufferRead = fread(inputBuffer+readInputIndex, sizeof(int), (bufferSize-readInputIndex), inputFile);
				inBufferRead += actualInputBufferRead;
			}


			if(actualInputBufferRead <= 0)
			{
								
				if(outputBufferCount > 0)
				{
					fwrite(outputBuffer, sizeof(int), outputBufferCount, inputSortedRuns);
					opBufferWrote += outputBufferCount;
					outputbufferWriteCount+=1;

					fclose(inputSortedRuns);
					
					outputBufferCount = 0;
					noOfRunsCreated++;


					snprintf(tempSuperRunFile, (2*bufferSize), "%s.%03d",inputFileName, noOfRunsCreated);
					inputSortedRuns = fopen(tempSuperRunFile, "w+b");

				}



				if( sizeOfHeap>0)
				{
					if(primaryHeapIndex == 0)
					{
						qsort(inputBuffer, sizeOfHeap, sizeof(int), compareKeys);
					}
					else
					{
						qsort( (inputBuffer+(tempPrimaryIndexForSwitching)), sizeOfHeap, sizeof(int), compareKeys);
					}
				}



				if( sizeofSecondaryHeap>0)
				{
					if(secondaryHeapIndex == 0)
					{
						qsort( (inputBuffer+(tempSecondaryIndexForSwitching- sizeofSecondaryHeap+1)), sizeofSecondaryHeap, sizeof(int), compareKeys);
					}
					else
					{
						qsort( (inputBuffer+(secondaryHeapIndex- sizeofSecondaryHeap+1)), sizeofSecondaryHeap, sizeof(int), compareKeys);
					}
				}


				int tempPrimaryIndex = 0, tempSecondaryIndex = 0;

				if (primaryHeapIndex == 0)
				{
					tempPrimaryIndex = 0;
					tempSecondaryIndex = 749;
				}
				else
				{
					tempPrimaryIndex = 749;
					tempSecondaryIndex = 0;
				}


				while(sizeOfHeap>0 || sizeofSecondaryHeap >0)
				{

					if(sizeOfHeap>0 && sizeofSecondaryHeap>0)
					{

						if(tempPrimaryIndex == 0)
						{
							if(inputBuffer[primaryHeapIndex] < inputBuffer[(secondaryHeapIndex- sizeofSecondaryHeap+1)])
							{
								outputBuffer[outputBufferCount] = inputBuffer[primaryHeapIndex];
								outputBufferCount++;
								primaryHeapIndex++;
								sizeOfHeap--;
							}
							else
							{
								outputBuffer[outputBufferCount] = inputBuffer[(secondaryHeapIndex- sizeofSecondaryHeap+1)];
								outputBufferCount++;
								sizeofSecondaryHeap--;
							}
						}
						else
						{
							if(inputBuffer[(tempSecondaryIndexForSwitching- sizeofSecondaryHeap+1)] < inputBuffer[tempPrimaryIndexForSwitching])
							{
								outputBuffer[outputBufferCount] = inputBuffer[(tempSecondaryIndexForSwitching- sizeofSecondaryHeap+1)];
								outputBufferCount++;
								sizeofSecondaryHeap--;
							}
							else
							{
								outputBuffer[outputBufferCount] = inputBuffer[tempPrimaryIndexForSwitching];
								outputBufferCount++;
								tempPrimaryIndexForSwitching++;
								sizeOfHeap--;
							}
						}

					}
					else if(sizeOfHeap>0)
					{

						if(tempPrimaryIndex == 0)
						{
							outputBuffer[outputBufferCount] = inputBuffer[primaryHeapIndex];
							outputBufferCount++;
							primaryHeapIndex++;
							sizeOfHeap--;
						}
						else
						{
							outputBuffer[outputBufferCount] = inputBuffer[tempPrimaryIndexForSwitching];
							outputBufferCount++;
							tempPrimaryIndexForSwitching++;
							sizeOfHeap--;
						}


					}
					else if(sizeofSecondaryHeap>0)
					{
						if(tempSecondaryIndex != 0)
						{
							outputBuffer[outputBufferCount] = inputBuffer[(secondaryHeapIndex- sizeofSecondaryHeap+1)];
							outputBufferCount++;
							sizeofSecondaryHeap--;
						}
						else
						{
							outputBuffer[outputBufferCount] = inputBuffer[(tempSecondaryIndexForSwitching- sizeofSecondaryHeap+1)];
							outputBufferCount++;
							//secondaryHeapIndex++;
							sizeofSecondaryHeap--;
						}
					}
				}




				if( outputBufferCount > 0 )
				{
					fwrite(outputBuffer, sizeof(int), outputBufferCount, inputSortedRuns);

					opBufferWrote += outputBufferCount;

					outputBufferCount = 0;
					//count to keep track of buffers, so that last buffer can be write out in end
					outputbufferWriteCount+=1;
				}

				
				fclose(inputSortedRuns);
		    	break;
			}
		}
		else
		{
			//current run completed
			//create new run
			noOfRunsCreated++;

			if( outputBufferCount > 0 )
			{
				fwrite(outputBuffer, sizeof(int), outputBufferCount, inputSortedRuns);

				opBufferWrote += outputBufferCount;

				outputBufferCount = 0;
				//count to keep track of buffers, so that last buffer can be write out in end
				outputbufferWriteCount+=1;
			}

			fclose(inputSortedRuns);

			snprintf(tempSuperRunFile, (2*bufferSize), "%s.%03d",inputFileName, noOfRunsCreated);
			inputSortedRuns = fopen(tempSuperRunFile, "w+b");


		    if(secondaryHeapIndex == 749 )
		    	tempPrimaryIndexForSwitching = (secondaryHeapIndex - sizeofSecondaryHeap +1);
		    else
		    	tempPrimaryIndexForSwitching = 0;


		    if(primaryHeapIndex == 0)
		    	tempSecondaryIndexForSwitching = 749;
		    else
		    	tempSecondaryIndexForSwitching = 0;



			tempHeapIndex = sizeOfHeap;
			sizeOfHeap = sizeofSecondaryHeap;
			sizeofSecondaryHeap = tempHeapIndex;


		    tempHeapIndex =  primaryHeapIndex;
		    primaryHeapIndex = secondaryHeapIndex;

		    secondaryHeapIndex = tempHeapIndex;

		    if(primaryHeapIndex == 749)
		    	heapify(inputBuffer, sizeOfHeap, (primaryHeapIndex-sizeOfHeap+1));
		    else
		    	heapify(inputBuffer, sizeOfHeap, 0);

		}
		

	}


	noOfRunsCreated+=1;
	mergeSortedRuns(noOfRunsCreated, inputBuffer, outputBuffer, inputFileName, sortedFileName, 0);

	return(0);
}




void heapify(int *inputBuffer, int noOfElements, int startPos)
{
	int i = 0;

	i = (noOfElements/2);

	while( i >= startPos)
	{
		siftElement(inputBuffer+startPos, i, (noOfElements) );
		i--;
	}
}


void siftElement(int *inputBuffer, int index, int heapSize)
{
	int smallerElementIndex = 0;
	int leftIndex = 2*index+1;
	int rightIndex = leftIndex+1;
		
	int top = inputBuffer[index];
	
	while(index < (heapSize/2) )
	{
		leftIndex = 2*index+1;
		rightIndex = leftIndex+1;
		

		if(rightIndex < heapSize && inputBuffer[leftIndex] > inputBuffer[rightIndex] )
			smallerElementIndex = rightIndex;
		else
			smallerElementIndex = leftIndex;
		

		if(top <= inputBuffer[smallerElementIndex] )
			break;
		

		inputBuffer[index] = inputBuffer[smallerElementIndex];
		index = smallerElementIndex;
	}
	
	inputBuffer[index] = top;
}
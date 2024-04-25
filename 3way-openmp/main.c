#include <time.h>   
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <unistd.h> 
#include <string.h>
#include <locale.h>
#include <omp.h> 

#pragma region DEFINES
#if RUNNING_LOCAL
    #define TEXT_FILE "../../wiki_dump.txt"
#else
    #define TEXT_FILE "/homes/dan/625/wiki_dump.txt"
    //#define TEXT_FILE "test.txt"
#endif
#define NUMBER_OF_LINES 1000000
//#define NUMBER_OF_LINES 18

#pragma endregion

#pragma region GLOBAL_VARIABLES

#pragma endregion


uint8_t find_max_ascii(char* line){
    uint8_t max_char = 0;
    uint16_t i = 0;
    while(line[i] != '\0'){         // Go until the end of line is reached
        if(max_char < line[i])      // If max char is smaller than the next char
            max_char = line[i];     // Replace the max char with the next char
        i++;
    }
    return max_char;
}

void processLines(int start_lines, int linesPerThread, int thread_id, int** biggest_char){
    FILE *file = fopen(TEXT_FILE, "r");  // Open file to read
    if(file == NULL)                                // If the file is unable to be opened
    {
        printf("Fail");
    }
    else{
        char line[2001];                                        // longest line is 2000 characters + '\0'
        for(uint64_t i = 0; i < start_lines; i++)    // Iterate until we reach the threads start line
            fscanf(file, "%[^\n]\n", line);                     // Advance the file pointer to the next line
    
        for(uint64_t i = 0; i < linesPerThread; i++){               // Iterate over each line for this thread
            fscanf(file, "%[^\n]\n", line);                                     // Grab the next associated line from the file
            biggest_char[thread_id][i] = find_max_ascii(line);      // Store the max ascii valu efrom the current line into to global buffer
        }
        fclose(file);           // Close the file 
    }

}


int main()
{
    int thread_id; //The thread id
    int start_lines; //The line that the thread starts at
    int numThreads; //The total number of threads
    int** biggest_char; //The 2d array holding the biggest character
    int linesPerThread;  //The number of lines a thread is responsible for

    #pragma omp parallel private(thread_id) //Start of the parallel computing
    {
        if(thread_id == 0){ //If master thread
            numThreads = omp_get_num_threads(); //Get the total number of threads
            biggest_char = malloc(sizeof(int*)*numThreads); //allocate 2d array with numThread rows
            linesPerThread = NUMBER_OF_LINES/numThreads; //Calculate the number of lines per thread
            for(int i = 0; i< numThreads; ++i){ 
                biggest_char[i] = malloc(linesPerThread*sizeof(int)); //2d array wiht linesPerThread columns
            }
        }
        #pragma omp barrier //Wait for all threads(Mainly the master thread) to reach this point

        thread_id = omp_get_thread_num(); //get the thread number
        start_lines = thread_id*linesPerThread; //Calculate the start line for the thread

        processLines(start_lines, linesPerThread, thread_id, biggest_char); //Process each line
    }

    for(int i = 0; i < numThreads; i++){
        for(int j = 0; j < linesPerThread; j++){    // threadArr[i] has finished, so we can print its data
            uint32_t index = j + i*linesPerThread;  // Get the overall index
            printf("%d: %d\n", index, biggest_char[i][j]);      // Print its index and its ascii value
        }
    }

    //Free the memory
    for(int i = 0; i < numThreads; ++i){
        free(biggest_char[i]);
    }
    free(biggest_char);
}






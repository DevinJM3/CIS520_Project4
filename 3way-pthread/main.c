#include <time.h>   
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <unistd.h> 
#include <string.h>
#include <pthread.h> 

#pragma region DEFINES
#if RUNNING_LOCAL
    #define TEXT_FILE "../../wiki_dump.txt"
#else
    #define TEXT_FILE "~dan/625/wiki_dump.txt"
#endif
#define NUMBER_OF_LINES 1000000
#pragma endregion

#pragma region GLOBAL_VARIABLES
//pthread_mutex_t mutex; // mutex for 
 
char biggest_char[NUMBER_OF_LINES];
#pragma endregion

typedef struct{
    char *filename;
    uint32_t start_line;
    uint32_t end_line;
    uint32_t lines_per_thread;
} ThreadArgs;


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

void *processLines(void *args){
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    FILE *file = fopen(threadArgs->filename, "r");  // Open file to read
    if(file == NULL)                                // If the file is unable to be opened
        pthread_exit(-1);                           // Exit the thread with error code -1

    char line[2001];                                        // longest line is 2000 characters + '\0'
    for(uint64_t i = 0; i < threadArgs->start_line; i++)    // Iterate until we reach the threads start line
        fscanf(file, "%[^\n]\n", line);                     // Advance the file pointer to the next line
    
    char *maxAsciiBuffer = malloc(threadArgs->lines_per_thread);
    for(uint64_t i = 0; i < threadArgs->lines_per_thread; i++){     // Iterate over each line for this thread
        fscanf(file, "%[^\n]\n", line);                             // Grab the next associated line from the file
        maxAsciiBuffer[i] = find_max_ascii(line);                   // Calculate the maximum ascii value
    }

    fclose(file);
    pthread_exit(NULL);
}

int main()
{
    printf("Starting...\n");
    char *line = (char*) malloc(2001); // No lines longer than 2000 chars + null termination
    FILE *file = fopen(TEXT_FILE, "r");
    if(file == NULL){
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    char discard[2001];
    for(int i = 0; i < 2; i++)
        fscanf(file, "%[^\n]\n", discard);
    fscanf(file, "%[^\n]\n", line);
    printf("%s\n", line);

    // uint32_t num_lines = 0;
    // while(!feof(file)){
    //     fscanf(file, "%[^\n]\n", line);
    //     uint8_t character = find_max_ascii(line);
    //     printf("%d: (%c,%d)\n", num_lines++, character, character);
    // }

    fclose(file);
//     int nlines = 0, nchars = 0, err;
//     char *line = (char*) malloc( 2001 ); // no lines larger than 2000 chars

//     // Read in the lines from the data file
//     FILE *file = fopen( TEXT_FILE, "r" );
//     if(file == NULL){
//         perror("Unable to open file");
//         exit(EXIT_FAILURE);
//     }

//     while(!feof(file)){
//         err = fscanf(file, "%[^\n]\n", line);
//         uint8_t character = find_max_ascii(line);
//         printf("%d: (%c,%d)\n", nlines, character, character);
//         nlines++;
//     }

//    fclose(file);

    // for(int i = 0; i < 1000000; i++){

    // }

}



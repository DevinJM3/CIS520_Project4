#include <time.h>   
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <unistd.h> 
#include <string.h>
#include <locale.h>
#include <pthread.h> 

#pragma region DEFINES
#if RUNNING_LOCAL
    #define TEXT_FILE "../../wiki_dump.txt"
#else
    #define TEXT_FILE "~dan/625/wiki_dump.txt"
#endif
#define NUMBER_OF_LINES 1000000
#define NUMBER_OF_THREADS 2
#define NUMBER_OF_LINES_PER_THREAD (NUMBER_OF_LINES / NUMBER_OF_THREADS)
#pragma endregion

#pragma region GLOBAL_VARIABLES
pthread_mutex_t mutex;    // mutex for the array

char biggest_char[NUMBER_OF_THREADS][NUMBER_OF_LINES/NUMBER_OF_THREADS];
#pragma endregion

typedef struct{
    char *filename;
    uint32_t thread_id;
    uint32_t start_line;
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
        pthread_exit(NULL);                         // Exit the thread with error code 1

    char line[2001];                                        // longest line is 2000 characters + '\0'
    for(uint64_t i = 0; i < threadArgs->start_line; i++)    // Iterate until we reach the threads start line
        fscanf(file, "%[^\n]\n", line);                     // Advance the file pointer to the next line
    
    for(uint64_t i = 0; i < NUMBER_OF_LINES_PER_THREAD; i++){               // Iterate over each line for this thread
        fscanf(file, "%[^\n]\n", line);                                     // Grab the next associated line from the file

        pthread_mutex_lock(&mutex);                                         // Wait to secure the mutex
        biggest_char[threadArgs->thread_id][i] = find_max_ascii(line);      // Store the max ascii valu efrom the current line into to global buffer
        pthread_mutex_unlock(&mutex);                                       // Safely release the mutex lock
    }

    free(args);             // Free the alocated memory
    fclose(file);           // Close the file 
    pthread_exit(NULL);     // Return a success message
}

int main()
{
    if(pthread_mutex_init(&mutex, NULL) != 0){ // If the mutex fails to initialize
        perror("Mutex initilization has failed!");
        exit(EXIT_FAILURE);
    }

    pthread_t *threadArr = malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);   // Initialize the threads

    for(int i = 0; i < NUMBER_OF_THREADS; i++){                 
        ThreadArgs *thread = malloc(sizeof(ThreadArgs));        // Arguemnt passed into each thread
        thread->start_line = NUMBER_OF_LINES_PER_THREAD * i;
        thread->filename = TEXT_FILE;
        thread->thread_id = i;

        if(pthread_create(&threadArr[i], NULL, processLines, thread) == 0)  // If thread creation is successful
            printf("Thread %d created\n", threadArr[i]);                    // Show that the thread has successfully been created  
    }

    for(int i = 0; i < NUMBER_OF_THREADS; i++){
        pthread_join(threadArr[i], NULL);   // Wait until threadArr[i] joins the main thread (finishes its process)

        for(int j = 0; j < NUMBER_OF_LINES_PER_THREAD; j++){    // threadArr[i] has finished, so we can print its data
            uint32_t index = j + i*NUMBER_OF_LINES_PER_THREAD;  // Get the overall index
            printf("%d: %c\n", index, biggest_char[i][j]);      // Print its index and its ascii value
        }
    }
    
    free(threadArr);
}



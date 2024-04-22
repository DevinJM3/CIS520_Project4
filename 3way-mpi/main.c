#include <time.h>   
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <unistd.h> 
#include <string.h>
#include <locale.h>
#include <mpi.h> 

#pragma region DEFINES
#if RUNNING_LOCAL
    #define TEXT_FILE "../../wiki_dump.txt"
#else
    #define TEXT_FILE "~dan/625/wiki_dump.txt"
    //#define TEXT_FILE "test.txt"
#endif
#define NUMBER_OF_LINES 1000000
//#define NUMBER_OF_LINES 4

#pragma endregion

#pragma region GLOBAL_VARIABLES

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

int* processLines(uint32_t start_lines, int linesPerProcess){
    int taskID;
    int* biggest_char = malloc(sizeof(char)*linesPerProcess);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskID);

    FILE *file = fopen(TEXT_FILE, "r");  // Open file to read
    if(file == NULL)                                // If the file is unable to be opened
    {
        printf("Fail");
        return NULL;
    }
    else{
        char line[2001];                                        // longest line is 2000 characters + '\0'
        for(uint64_t i = 0; i < start_lines; i++)    // Iterate until we reach the threads start line
            fscanf(file, "%[^\n]\n", line);                     // Advance the file pointer to the next line
    
        for(uint64_t i = 0; i < linesPerProcess; i++){               // Iterate over each line for this thread
            fscanf(file, "%[^\n]\n", line);                                     // Grab the next associated line from the file
            biggest_char[i] = find_max_ascii(line);      // Store the max ascii valu efrom the current line into to global buffer
        }
    }
    fclose(file);           // Close the file 
    return biggest_char;
}

int main()
{
    MPI_Init(NULL, NULL); //Initialize MPI

    int numProcesses;
    int taskID;

    MPI_Comm_rank(MPI_COMM_WORLD, &taskID); //Get the task ID
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses); //Get the number of processes

    int linesPerProcess = NUMBER_OF_LINES / numProcesses; //Calculate the number of lines per process
    int* biggest_char_single_process = malloc(sizeof(char)*linesPerProcess); //An array to keep track of the biggest char for each line


    if (taskID == 0){ //For the main process
        for(int i = 1; i < numProcesses; ++i){
            uint32_t start_line = linesPerProcess * i; //Calculates the start line for each process
            MPI_Send(&start_line, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //Send the start line to each process.
        }

        uint32_t start_line = 0; //start line for the main process
        biggest_char_single_process = processLines(start_line, linesPerProcess); //process the lines starting at the start line

        for(int j = 0; j < linesPerProcess; j++){  //Loop for the first process
            uint32_t index = j;  // Get the overall index
            printf("%d: %d\n", 0, biggest_char_single_process[j]);      // Print its index and its ascii value
        }

        for(int i = 1; i < numProcesses; i++){ //Loop to print each process's work
            MPI_Recv(biggest_char_single_process, linesPerProcess, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //Receive each array
            for(int j = 0; j < linesPerProcess; j++){  
                uint32_t index = j + i*linesPerProcess;  // Get the overall index
                printf("%d: %d\n", index, biggest_char_single_process[j]);      // Print its index and its ascii value
            }
        }
    }
    else{ //For all other processes
        uint32_t start_line;
        MPI_Recv(&start_line, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //Receives the start line
        biggest_char_single_process = processLines(start_line, linesPerProcess); //process the lines starting at the start line
        MPI_Send(biggest_char_single_process, linesPerProcess, MPI_INT, 0, 0, MPI_COMM_WORLD); //Sends the array back to the main process

    } 

    MPI_Finalize(); //End of MPI


    

}



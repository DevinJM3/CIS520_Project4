#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define NUM_THREADS 4
#define FILE_LINES 1000000

//A 2d char array meant to hold the highest ascii value on each line of the wiki file
char output[FILE_LINES][50];

pthread_mutex_t lock;

///
/// Finds the highest ascii value in a char array
/// \param line The array being evaluated
/// \param nchars The number of chars in the line
/// \return Int of the highest ascii value in the line
///
int find_highest_ascii(char* line, int nchars)
{
  //Checks if the parameters are valid and returns 0 if they are not
  if(line == NULL || nchars < 0) return 0;
  //An int meant to keep track of the highest ascii value in the line
  int max = 0;
  //Traverses the line looking for the highest ascii value
  for (int i = 0; i < nchars; i++ ) {
    if((int)line[i] > max)
    {
      max = (int)line[i];
    }
  }
  //The highest ascii value found in the string
  return max;
}

///
/// Traverses the wiki_dump file and finds the highest ascii character on each line
/// \param thread_id The id of the thread that is traversing the wiki_dump
/// the id of the thread determines what part of the file the thread is 
/// evaluating
///
void *read_wiki_file(void* thread_id)
{
  //Opens the wiki_dump file in read mode
  FILE *file = fopen( "../../wiki_dump.txt", "r" );
  //Creates a pointer to a char array that can hold up to 2000 chars
  char *line = (char*) malloc( 2001 ); // no lines larger than 2000 chars
  //A value to be used to determine the number of chars on a line
  int nchars = 0;
  //The maxium number of lines checked for each thread
  int maxlines = 0;
  int offset = 0;
  if(FILE_LINES%NUM_THREADS != 0 && (long)thread_id == NUM_THREADS-1) 
  {
    offset = (FILE_LINES%NUM_THREADS);
  }
  maxlines = FILE_LINES/NUM_THREADS;
  
  //Used to check if there is an error
  int err;
  //A char array that is used to store elements for the output array. Has the form "line_num: ascii_value"
  char result[20];    
  //Traverses the threads section of the array, finds the highest value, and copies the memory to the output array
  for(int i = maxlines*(long)thread_id; i < (maxlines*((long)thread_id+1))+offset; i++)
  {
    err = fscanf( file, "%[^\n]\n", line);
    if( err == EOF ) break;
    nchars = strlen( line );
    int ascii = find_highest_ascii(line, nchars);
    snprintf(result, 20 , "%d: %d", i, ascii);
    
    
    pthread_mutex_lock(&lock);
    memcpy(output[i], result, 20);  
    pthread_mutex_unlock(&lock);
  }
  //Closes the file
  fclose(file);
  //Tells the thread to exit
  pthread_exit(NULL);
}

void print_output()
{
    printf("Start");
  //Traverses the output array and prints every line of it
  for(int i = 0; i < FILE_LINES; i++)
  {
    printf("%d %s\n",i, output[i]);
  }
}

int main()
{
  //Sets the memory of the array to 0s
  memset(output,0,FILE_LINES);
  //The pthreads of the program
  pthread_t threads[NUM_THREADS];
  pthread_attr_t attr;
  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  void *status;
  int rc;

  if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
  } 

  //Creates the threads and has them execute read_wki_file
  for(long t=0; t < NUM_THREADS; t++){
    printf("In main: creating thread %ld\n", t);
    rc = pthread_create(&threads[t], &attr, read_wiki_file, (void *)t);
    //Checks if the rc is valid
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }
  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);
  //Has the threads join back  together
  for(int i=0; i<NUM_THREADS; i++) {
      rc = pthread_join(threads[i], &status);
      if (rc) {
          printf("ERROR; return code from pthread_join() is %d\n", rc);
          exit(-1);
      }
  }
  //Prints the output array
  print_output();
  /* Last thing that main() should do */
  pthread_mutex_destroy(&lock); 
  pthread_exit(NULL);
}
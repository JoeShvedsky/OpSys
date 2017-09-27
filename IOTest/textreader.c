//Joe Shvedsky
//Reads in from text file then searches for a string and its(their) frequency

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//SICK PRE-PROCESSOR DUDE
#define MAX_LEN 80


void read_file(FILE*, char**, int*, int, long);

int word_exists(char*, char**, int);

int main(int argc, char* argv[])
{
    //simple command line error check   
    if(argc < 2 || argc > 3)
    {    
        fprintf(stderr, "ERROR: invalid command line arguments.\n");
        return EXIT_FAILURE;
    }
    
    FILE* fp;
    
    fp = fopen(argv[1], "rt");
    if(fp == NULL)
    {
        fprintf(stderr, "ERROR: could not open file to read.\n");
        return EXIT_FAILURE;
    }
    
    //this bit of code checks for the optional third argument and if it is valid
    long size_to_read; //will be used as a parameter
    
    if(argc == 2) //read the whole file
    {
        size_to_read = -1;
    }
    else //if optional third command-line argument is present
    {
        char* end_ptr; 
        size_to_read = strtol(argv[2], &end_ptr, 10);
        if(size_to_read < 0 || strlen(end_ptr) > 0)
        {
            fprintf(stderr, "ERROR: optional number of words to read must be a non-negative integer.\n");
            return EXIT_FAILURE;
        }
    }
    
    //allocate memory
    char** words;
    int* word_sizes;
    
    int array_size = 8;
    
    words = (char**)calloc(array_size, sizeof(char*));
    word_sizes = (int*)calloc(array_size, sizeof(int));
    
    printf("Allocated initial parallel arrays of size %d.\n", array_size);
    fflush(stdout);
    
    //do the thing
    read_file(fp, words, word_sizes, array_size, size_to_read);
    
  
    //success!
    return EXIT_SUCCESS;
}

void read_file(FILE* fp, char** words, int* word_sizes, int array_size, long num_to_read)
{

    //keep track of the word sizes
    int cur_total_size = 0;
    int cur_unique_size = 0;
   
    
    //run through the whole file until it reads the final word
    while(1)
    {
        char word[MAX_LEN] = "";
        int c;
        int char_pos = 0;
        //continually read in chars to make the words
        do 
        {
            c = fgetc(fp);
            if(isalnum(c))
            {    
                word[char_pos++] = c;
            }
            
        } while(c != EOF && isalnum(c));
        
        //we're done
        if(char_pos == 0 && c == EOF)
        {
            break;
        }
        
        if(char_pos > 0)
        {
            word[char_pos + 1] = '\0';
            
            int word_pos = word_exists(word, words, cur_unique_size);
            //only increment word counter if the word was already stored
            if(word_pos >= 0)
            {

                ++word_sizes[word_pos];

                cur_total_size++;
            }
            
            else //if new word
            {
                if(cur_unique_size == array_size) //need to re-size arrays
                {
            
                    array_size *= 2;
                    words = (char**)realloc(words, array_size*sizeof(char*));
                    word_sizes = (int*)realloc(word_sizes, array_size*sizeof(int));
                    if(words==NULL || word_sizes == NULL)
                    {
                        fprintf(stderr, "ERROR: Could not re-allocate memory.\n");
                        exit(1);
                    }
                    printf("Re-allocated parallel arrays to be size %d.\n", array_size);
                    fflush(stdout);
                }
                //add the new word in, making sure to make space for null terminator
                words[cur_unique_size] = (char*)calloc(strlen(word)+1, sizeof(char));
                strncat(words[cur_unique_size], word, MAX_LEN);
                
                word_sizes[cur_unique_size] = 1;
                
                cur_unique_size++;
                cur_total_size++;
            }
            //another ending condition
            //(for non-alphanumeric characters, 
            //meaning word is non-zero length but useless)
            if(c == EOF)
            {
                break;
            }
        }
    }

    
    
    fclose(fp);
    
    printf("All done (successfully read %d words; %d unique words).\n", \
    cur_total_size, cur_unique_size);
    fflush(stdout);
    
    //print the whole thing
    if(num_to_read == -1 || num_to_read >= cur_unique_size)
    {
        printf("All words (and corresponding counts) are:\n");
        fflush(stdout);
        
        int pos;
        for(pos = 0; pos < cur_unique_size; pos++)
        {
            printf("%s -- %d\n", words[pos], word_sizes[pos]);
            fflush(stdout);
        }
        fflush(stdout);
    }
    
    else //print only specified number of words
    {

        printf("First %ld words (and their corresponding counts) are:\n", num_to_read);
        int pos;
        for(pos = 0; pos < num_to_read; pos++)
        {
            printf("%s -- %d\n", words[pos], word_sizes[pos]);
            fflush(stdout);
        }
        fflush(stdout);
    }
    

    //free memory
    free(word_sizes);
    int i;
    for(i = 0; i < cur_unique_size; i++)
        free(words[i]);
    free(words);

}

//pretty simple, compare word to all of words[]
int word_exists(char* word, char** words, int size)
{
    int pos;
    for(pos = 0; pos < size; pos++)
    {
        if(strncmp(word, words[pos], MAX_LEN) == 0)
        {
            return pos;
        }
    }
    return -1;
}


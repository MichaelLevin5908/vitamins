/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright (C) 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "word_count.h"
#include "word_helpers.h"

typedef struct {
    word_count_list_t *word_counts;
    char *filename;
} thread_args_t;

void *thread_count_words(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    FILE *file = fopen(args->filename, "r");
    
    if (file != NULL) {
        count_words(args->word_counts, file);
        fclose(file);
    } else {
        perror("Error opening file");
    }
    
    free(args);
    return NULL;
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts);

    if (argc <= 1) {
        /* Process stdin in a single thread. */
        count_words(&word_counts, stdin);
    } else {
        int num_threads = argc - 1;
        pthread_t threads[num_threads];

        for (int i = 0; i < num_threads; i++) {
            thread_args_t *args = malloc(sizeof(thread_args_t));
            if (args == NULL) { 
                perror("Error allocating memory");
                return EXIT_FAILURE;
            }
            
            args->word_counts = &word_counts;
            args->filename = argv[i + 1];

            if (pthread_create(&threads[i], NULL, thread_count_words, args) != 0) {
                perror("Error creating thread");
                free(args);
                return EXIT_FAILURE;
            }
        }
        
        for(int i = 0; i < num_threads; i++) {
            if (pthread_join(threads[i], NULL) != 0) {
                perror("Error joining thread");
                return EXIT_FAILURE;
            }
        }
    }

    /* Output final result of all threads' work. */
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}

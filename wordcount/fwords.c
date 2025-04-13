/*
 * Word count application with one process per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2019 University of California, Berkeley
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
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "word_count.h"
#include "word_helpers.h"

/*
 * Read stream of counts and accumulate globally.
 */
void merge_counts(word_count_list_t *wclist, FILE *count_stream) {
    char *word;
    int count;
    int rv;
    while ((rv = fscanf(count_stream, "%8d\t%ms\n", &count, &word)) == 2) {
        add_word_with_count(wclist, word, count);
    }
    if ((rv == EOF) && (feof(count_stream) == 0)) {
        perror("could not read counts");
    } else if (rv != EOF) {
        fprintf(stderr, "read ill-formed count (matched %d)\n", rv);
    }
}

/*
 * main - handle command line, spawning one process per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts);

    if (argc <= 1) {
        /* Process stdin in a single process. */
        count_words(&word_counts, stdin);
    } else {
        /* TODO */
        for (int i = 1; i < argc; i++) {
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe creation failed");
                exit(EXIT_FAILURE);
            }
            
            /* Fork a child process */
            pid_t pid = fork();
            
            if (pid == -1) {
                /* Fork failed */
                perror("fork failed");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                /* Child process */
                
                close(pipefd[0]);
                
                FILE *file = fopen(argv[i], "r");
                if (file == NULL) {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }
                
                word_count_list_t file_counts;
                init_words(&file_counts);
                
                count_words(&file_counts, file);
                fclose(file);
                
                FILE *out_stream = fdopen(pipefd[1], "w");
                if (out_stream == NULL) {
                    perror("fdopen failed");
                    exit(EXIT_FAILURE);
                }
            
                fprint_words(&file_counts, out_stream);
                fclose(out_stream);
                
                exit(EXIT_SUCCESS);
            } else {
                /* Parent process */
                
                close(pipefd[1]);
                
                FILE *in_stream = fdopen(pipefd[0], "r");
                if (in_stream == NULL) {
                    perror("fdopen failed");
                    exit(EXIT_FAILURE);
                }
                
                merge_counts(&word_counts, in_stream);
                fclose(in_stream); 
                
                waitpid(pid, NULL, 0);
            }
        }
    }

    /* Output final result of all process' work. */
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}

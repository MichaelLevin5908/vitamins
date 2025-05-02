/* Tests writing to standard output. */

#include <stdio.h>
#include <string.h>
#include <syscall.h>

int main(void) {
    const char *message = "CS 111: Operating Systems Principles\n";
    write(STDOUT_FILENO, message, strlen(message));
    return 0;
}

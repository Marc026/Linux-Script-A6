#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

// Function to apply Caesar cipher
void caesarCipher(char *buffer, int shift, int reverse) {
    int i = 0;
    while (buffer[i]) {
        if (buffer[i] >= 'A' && buffer[i] <= 'Z') {
            if (reverse)
                buffer[i] = ((buffer[i] - 'A' - shift + 26) % 26) + 'A';
            else
                buffer[i] = ((buffer[i] - 'A' + shift) % 26) + 'A';
        } else if (buffer[i] >= 'a' && buffer[i] <= 'z') {
            if (reverse)
                buffer[i] = ((buffer[i] - 'a' - shift + 26) % 26) + 'a';
            else
                buffer[i] = ((buffer[i] - 'a' + shift) % 26) + 'a';
        }
        i++;
    }
}

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    printf("Process ID (PID): %d\n", pid);

    struct rusage usage;
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    int shift = 0;
    int reverse = 0;
    int count_lines = 0;
    int opt;
    char buffer[BUFFER_SIZE];
    
    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "s:rn")) != -1) {
        switch (opt) {
            case 's':
                shift = atoi(optarg);
                break;
            case 'r':
                reverse = 1;
                break;
            case 'n':
                count_lines = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-s shift] [-r] [-n] [FILE]...\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Open and read from each file
    for (int i = optind; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            perror("open");
            continue;
        }
        
        ssize_t bytes_read;
        int lines = 0;
        while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
            // Apply Caesar cipher
            caesarCipher(buffer, shift, reverse);
            // Count lines if option -n is specified
            if (count_lines) {
                for (int j = 0; j < bytes_read; j++) {
                    if (buffer[j] == '\n') {
                        lines++;
                    }
                }
            }
            // Write to stdout
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
                perror("write");
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
        
        close(fd);

        if (count_lines) {
            printf("Number of lines: %d\n", lines);
        }
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("CPU%%: %.2f\n", cpu_time_used / ((double)usage.ru_utime.tv_sec + (double)usage.ru_stime.tv_sec));
    printf("Memory usage: %ld KB\n", usage.ru_maxrss);
    printf("Run time: %.2f seconds\n", cpu_time_used);

    // Truncate the output file if reverse is not specified
    if (!reverse) {
        int output_fd = open("output_file.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (output_fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        
        // Redirect stdout to the output file
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(output_fd);
            exit(EXIT_FAILURE);
        }
        
        close(output_fd);
    }

    return 0;
}


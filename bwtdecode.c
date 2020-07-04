//
// Created by Shawn Jin on 30/6/20.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4000000 // buffer size for read
#define OUTPUT_BUFFER_SIZE 1000000

char buffer[BUFFER_SIZE];
int buffer_count[BUFFER_SIZE];
int first_array[5] = {0};
int total_size = 0;


int index_of_values(char c) {
    switch (c) {
        case '\n':
            return 0;
        case 'A':
            return 1;
        case 'C':
            return 2;
        case 'G':
            return 3;
        case 'T':
            return 4;
        default:
            return -1;
    }
}

void read_from_stream(int next_index, int current_index, FILE *str, FILE *out) {
    if (next_index / BUFFER_SIZE == current_index / BUFFER_SIZE) {
        // in same buffer no need to read
        return;
    }
    int start_pos = (next_index / BUFFER_SIZE) * BUFFER_SIZE;
    fseek(str, start_pos, SEEK_SET);
    memset(buffer, 0, BUFFER_SIZE);

    fread(buffer, 1, BUFFER_SIZE, str);

    memset(buffer_count, 0, BUFFER_SIZE * 4);
    int start_pos_out = (next_index / BUFFER_SIZE) * BUFFER_SIZE * 4 + total_size + 1;
    fseek(out, start_pos_out, SEEK_SET);
    fread(buffer_count, 4, BUFFER_SIZE, out);
}

void construct_first_row(FILE *str, FILE *out) {
    // write count to output as tmp file
    fseek(str, 0, SEEK_SET);
    fseek(out, total_size + 1, SEEK_SET);
    int val;
    while (!feof(str)) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(buffer_count, 0, BUFFER_SIZE * 4);
        fread(buffer, 1, BUFFER_SIZE, str);
        for (int i = 0; i < BUFFER_SIZE; i++) {
            val = first_array[index_of_values(buffer[i])];
            buffer_count[i] = val;
            first_array[index_of_values(buffer[i])]++;
        }
        fwrite(buffer_count, 4, BUFFER_SIZE, out);
    }

    for (int i = 1; i < 5; i++) {
        first_array[i] += first_array[i - 1];
    }
}

void clean_output(FILE *fp) {
    ftruncate(fileno(fp), total_size);
}

void reverse(char *str) {
    int len = strlen(str);
    char tmp;
    for (int i = 0; i < len / 2; i++) {
        tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}

void decode(FILE *input, FILE *output) {
    char *output_val = malloc(BUFFER_SIZE);
    int active_index = 0;
    int count = 0;
    // reversed output
    read_from_stream(0, BUFFER_SIZE, input, output);
    int index_of_buffer, index, p;
    char current_val;
    while (count < total_size) {
        index_of_buffer = active_index % BUFFER_SIZE;

        current_val = buffer[index_of_buffer];

        if ((count % OUTPUT_BUFFER_SIZE == 0 && count != 0) || count == total_size - 1) {
            fseek(output, total_size - count - 1, SEEK_SET);
            reverse(output_val);
            fwrite(output_val, 1, strlen(output_val), output);
            memset(output_val, 0, OUTPUT_BUFFER_SIZE);
        }

        output_val[count % OUTPUT_BUFFER_SIZE] = current_val;

        index = index_of_values(current_val); // index of ATCG values
        p = active_index;
        active_index = first_array[index - 1] + buffer_count[index_of_buffer];
        read_from_stream(active_index, p, input, output);

        count++;
    }
    fseek(output, total_size - 1, SEEK_SET);
    fputc('\n', output);
    free(output_val);
}


int main(int argc, char *argv[]) {
    FILE *encodedFile = fopen(argv[1], "rb");
    FILE *outputFile = fopen(argv[2], "w+b");

//    setbuf(encodedFile, NULL);
//    setbuf(outputFile, NULL);

    fseek(encodedFile, 0, SEEK_END);
    total_size = ftell(encodedFile);

    construct_first_row(encodedFile, outputFile);
    decode(encodedFile, outputFile);
    clean_output(outputFile);

    fclose(encodedFile);
    fclose(outputFile);

    return 0;
}
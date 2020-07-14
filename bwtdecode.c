//
// Created by Shawn Jin on 30/6/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>        /* for CHAR_BIT */

#define BUFFER_SIZE (128)
#define OUTPUT_BUFFER_SIZE (4096)

/* implement bitset */
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

struct bits_of_val {
    _Bool _1st;
    _Bool _2nd;
    _Bool is_end;
};

int total_size = 0;
int **first_array;
int total_number[5];
// refer to A: 00 C: 01 T: 10 G:11
//bitset<30 * 1024 * 1024 + 2> last_T;
char last_T[BITNSLOTS(30 * 1024 * 1024 + 8)];
//bitset<30 * 1024 * 1024 + 2> raw;
unsigned int pos_of_end = -1;

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

char convert(_Bool _1st, _Bool _2nd, int _1st_pos) {
    if (_1st_pos == pos_of_end) {
        return '\n';
    }
    if (_1st) {
        if (_2nd) return 'G';
        else return 'T';
    } else {
        if (_2nd) return 'C';
        else return 'A';
    }
}

struct bits_of_val convert1(char c) {
    struct bits_of_val bov;
    switch (c) {
        case 'A':
            bov._1st = 0;
            bov._2nd = 0;
            bov.is_end = 0;
            return bov;
        case 'C':
            bov._1st = 0;
            bov._2nd = 1;
            bov.is_end = 0;
            return bov;
        case 'T':
            bov._1st = 1;
            bov._2nd = 0;
            bov.is_end = 0;
            return bov;
        case 'G':
            bov._1st = 1;
            bov._2nd = 1;
            bov.is_end = 0;
            return bov;
        default:
            bov._1st = 0;
            bov._2nd = 0;
            bov.is_end = 1;
            return bov;
    }
}

char get_from_bit_set(int pos) {
    if (pos == pos_of_end)return '\n';
//    return convert(last_T[pos * 2], last_T[pos * 2 + 1], pos * 2);
    return convert(BITTEST(last_T, pos * 2), BITTEST(last_T, pos * 2 + 1), pos * 2);
}

void construct_first_row(FILE *input) {
    int buffer_size = BUFFER_SIZE;
    char *buffer = malloc(sizeof(char) * BUFFER_SIZE);
    rewind(input);
    int count = 0, idx, t_count = 0;
//    first_array.resize(total_size / buffer_size + 2, vector<int>(5));
//    total_number.resize(5);
    struct bits_of_val c;
    while (!feof(input)) {
        memset(buffer, 0, buffer_size);
        fread(buffer, 1, buffer_size, input);
        for (int i = 0; i < buffer_size; i++) {
            idx = index_of_values(buffer[i]);
            if (idx == -1) {
                break;
            }
            c = convert1(buffer[i]);
            if (c.is_end) {
                pos_of_end = t_count;
            }
            if (c._1st) BITSET(last_T, t_count * 2);
            if (c._2nd) BITSET(last_T, t_count * 2 + 1);
//            last_T[t_count * 2] = c._1st;
//            last_T[t_count * 2 + 1] = c._2nd;
            first_array[count + 1][idx]++;
            total_number[idx]++;
            t_count++;
        }
        count++;
    }

    for (int i = 1; i < total_size / BUFFER_SIZE + 2; i++) {
        for (int j = 0; j < 5; j++) {
            first_array[i][j] += first_array[i - 1][j];
        }
    }

    for (int i = 1; i < 5; i++) {
        total_number[i] += total_number[i - 1];
    }
}

int read_buffer(int index) {
    char c = get_from_bit_set(index);
    _Bool reverse = (index % BUFFER_SIZE) > (BUFFER_SIZE - 1 / 2);
    if (reverse) {
        int count = first_array[index / BUFFER_SIZE + 1][index_of_values(c)];
        for (int i = (index / BUFFER_SIZE + 1) * BUFFER_SIZE - 1; i < index / BUFFER_SIZE * BUFFER_SIZE; i--) {
            int val = get_from_bit_set(i);
            if (val != c) continue;
            count--;
            if (i == index)return count - 1;
        }
    } else {
        int count = first_array[index / BUFFER_SIZE][index_of_values(c)];
        for (int i = index / BUFFER_SIZE * BUFFER_SIZE; i < (index / BUFFER_SIZE + 1) * BUFFER_SIZE; i++) {
            int val = get_from_bit_set(i);
            if (val != c) continue;
            count++;
            if (i == index)return count;
        }
    }
    return -1;
}

void reverse(char *str, int len) {
    char tmp;
    for (int i = 0; i < len / 2; i++) {
        tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}

void decode(FILE *output) {
    char *output_val = malloc(sizeof(char) * (OUTPUT_BUFFER_SIZE + 1));
    memset(output_val, 0, sizeof(char) * (OUTPUT_BUFFER_SIZE + 1));
    int active_index = 0;
    int count = 0;
    // reversed output
    char current_val;
    while (count < total_size) {
        current_val = get_from_bit_set(active_index);
        if ((count % OUTPUT_BUFFER_SIZE == 0 && count != 0) || count == total_size - 1) {
            if (count % OUTPUT_BUFFER_SIZE != 0) {
                output_val[count % OUTPUT_BUFFER_SIZE] = '\0';
            }
            fseek(output, total_size - count - 1, SEEK_SET);
            reverse(output_val, strlen(output_val));
            fwrite(output_val, sizeof(char), strlen(output_val), output);
            memset(output_val, 0, BUFFER_SIZE);
        }


        output_val[count % OUTPUT_BUFFER_SIZE] = current_val;

        active_index = read_buffer(active_index) + total_number[index_of_values(current_val) - 1] - 1;
        count++;
    }
//    for (int i = total_size - 2; i >= 0; i--) {
//        auto c = convert(raw[i * 2], raw[i * 2 + 1], i * 2);
//        putc(c, output);
//    }
    fseek(output, total_size - 1, SEEK_SET);
    putc('\n', output);
    free(output_val);
}

int main(int argc, char *argv[]) {
    FILE *encodedFile = fopen(argv[1], "rb");
    FILE *outputFile = fopen(argv[2], "w+b");

    fseek(encodedFile, 0, SEEK_END);
    total_size = ftell(encodedFile);
    first_array = (int **) malloc((total_size / BUFFER_SIZE + 2) * sizeof(int *));
    for (int i = 0; i < total_size / BUFFER_SIZE + 2; i++) {
        first_array[i] = (int *) malloc(sizeof(int) * 5);
        memset(first_array[i], 0, sizeof(int) * 5);
    }

    construct_first_row(encodedFile);
    decode(outputFile);


    fclose(encodedFile);
    fclose(outputFile);

    for (int i = 0; i < total_size / BUFFER_SIZE + 2; i++) free(first_array[i]);
    free(first_array);

    return 0;
}

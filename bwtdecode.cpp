//
// Created by Shawn Jin on 12/7/20.
//

#include <iostream>
#include <cstdio>
#include <bitset>
#include <cstring>
#include <vector>

using namespace std;

#define BUFFER_SIZE (512)

struct bits_of_val {
    bool _1st;
    bool _2nd;
    bool is_end;
};

int total_size = 0;
vector<vector<int> > first_array;
vector<int> total_number;
// refer to A: 00 C: 01 T: 10 G:11
bitset<30 * 1024 * 1024 + 2> last_T;
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

char convert(bool _1st, bool _2nd, int _1st_pos) {
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

bits_of_val convert(char c) {
    switch (c) {
        case 'A':
            return bits_of_val{false, false, false};
        case 'C':
            return bits_of_val{false, true, false};
        case 'T':
            return bits_of_val{true, false, false};
        case 'G':
            return bits_of_val{true, true, false};
        default:
            return bits_of_val{false, false, true};
    }
}

char get_from_bit_set(int pos) {
    if (pos == pos_of_end)return '\n';
    return convert(last_T[pos * 2], last_T[pos * 2 + 1], pos * 2);
}

void construct_first_row(FILE *input) {
    int buffer_size = BUFFER_SIZE;
    char *buffer = new char[buffer_size];
    fseek(input, 0, SEEK_SET);
    int count = 0, idx, t_count = 0;
    first_array.resize(total_size / buffer_size + 2, vector<int>(5));
    total_number.resize(5);
    while (!feof(input)) {
        memset(buffer, 0, buffer_size);
        fread(buffer, 1, buffer_size, input);
        for (int i = 0; i < buffer_size; i++) {
            idx = index_of_values(buffer[i]);
            if (idx == -1) {
                break;
            }
            auto c = convert(buffer[i]);
            if (c.is_end) {
                pos_of_end = t_count;
            }
            last_T[t_count * 2] = c._1st;
            last_T[t_count * 2 + 1] = c._2nd;
            first_array[count + 1][idx]++;
            total_number[idx]++;
            t_count++;
        }
        count++;
    }

    for (int i = 1; i < first_array.size(); i++) {
        for (int j = 0; j < 5; j++) {
            first_array[i][j] += first_array[i - 1][j];
        }
    }

    for (int i = 1; i < 5; i++) {
        total_number[i] += total_number[i - 1];
    }
}

int read_buffer(int index) {
    auto c = get_from_bit_set(index);
    bool reverse = (index % BUFFER_SIZE) > (BUFFER_SIZE - 1 / 2);
    if (reverse) {
        auto count = first_array[index / BUFFER_SIZE + 1][index_of_values(c)];
        for (int i = (index / BUFFER_SIZE + 1) * BUFFER_SIZE - 1; i < index / BUFFER_SIZE * BUFFER_SIZE; i--) {
            auto val = get_from_bit_set(i);
            if (val != c) continue;
            count--;
            if (i == index)return count - 1;
        }
    } else {
        auto count = first_array[index / BUFFER_SIZE][index_of_values(c)];
        for (int i = index / BUFFER_SIZE * BUFFER_SIZE; i < (index / BUFFER_SIZE + 1) * BUFFER_SIZE; i++) {
            auto val = get_from_bit_set(i);
            if (val != c) continue;
            count++;
            if (i == index)return count;
        }
    }
    return -1;
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

void decode(FILE *output) {
    char *output_val = new char[BUFFER_SIZE];
    int active_index = 0;
    int count = 0;
    // reversed output
    char current_val;
    while (count < total_size) {
        current_val = get_from_bit_set(active_index);

        if ((count % BUFFER_SIZE == 0 && count != 0) || count == total_size - 1) {
            fseek(output, total_size - count - 1, SEEK_SET);
            reverse(output_val);
            fwrite(output_val, 1, strlen(output_val), output);
            memset(output_val, 0, BUFFER_SIZE);
        }

        output_val[count % BUFFER_SIZE] = current_val;

        auto ct = read_buffer(active_index);
        if (ct == -1) {
            exit(-1);
        }

        active_index = ct + total_number[index_of_values(current_val) - 1] - 1;
        count++;
    }
//    for (int i = total_size - 2; i >= 0; i--) {
//        auto c = convert(raw[i * 2], raw[i * 2 + 1], i * 2);
//        putc(c, output);
//    }
    fseek(output, total_size - 1, SEEK_SET);
    putc('\n', output);
    delete[] output_val;
}

int main(int argc, char *argv[]) {
    FILE *encodedFile = fopen(argv[1], "rb");
    FILE *outputFile = fopen(argv[2], "w+b");

    fseek(encodedFile, 0, SEEK_END);
    total_size = ftell(encodedFile);

    construct_first_row(encodedFile);
    decode(outputFile);


    fclose(encodedFile);
    fclose(outputFile);

    return 0;
}

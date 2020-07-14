//
// Created by Shawn Jin on 12/7/20.
//

#include <iostream>
#include <cstdio>
#include <bitset>
#include <cstring>
#include <vector>

using namespace std;

#define BUFFER_SIZE (256)
#define BITSET_SIZE (15 * 1024 * 1024 + 2)

int total_size = 0;
vector<vector<int> > first_array;
vector<int> total_number;
bitset<BITSET_SIZE> last_T;
vector<char> last_T_c;
bitset<BITSET_SIZE> raw;
vector<char> raw_c;

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

char get_from_bit_set(int pos) {
    auto tmp = last_T << (BITSET_SIZE - pos - 1);
    return last_T_c[tmp.count() - 1];
}

/**
 * use bitset and a vector to store the whole L
 * @param input
 */
void construct_first_row(FILE *input) {
    int buffer_size = BUFFER_SIZE;
    char *buffer = new char[buffer_size];
    fseek(input, 0, SEEK_SET);
    int count = 0, idx, t_count = 0;
    first_array.resize(total_size / buffer_size + 2, vector<int>(5));
    total_number.resize(5);
    char last_char = '\0';
    while (!feof(input)) {
        memset(buffer, 0, buffer_size);
        fread(buffer, 1, buffer_size, input);
        for (int i = 0; i < buffer_size; i++) {
            idx = index_of_values(buffer[i]);
            if (idx == -1) {
                break;
            }
            if (buffer[i] != last_char) {
                last_T[t_count] = true;
                last_T_c.push_back(buffer[i]);
            }
            last_char = buffer[i];
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
        for (int i = (index / BUFFER_SIZE + 1) * BUFFER_SIZE - 1; i >= (index / BUFFER_SIZE) * BUFFER_SIZE; i--) {
            auto val = get_from_bit_set(i);
            if (val == c) {
                count--;
                if (i == index) return count - 1;
            }
        }
        return -1;
    } else {
        auto count = first_array[index / BUFFER_SIZE][index_of_values(c)];
        for (int i = index / BUFFER_SIZE * BUFFER_SIZE; i < (index / BUFFER_SIZE + 1) * BUFFER_SIZE; i++) {
            auto val = get_from_bit_set(i);
            if (val != c) continue;
            count++;
            if (i == index)return count;
        }
        return count;
    }
}

void decode(FILE *output) {
    int active_index = 0;
    int count = 0;
    // reversed output
    char current_val, last_val;
    while (count < total_size) {
        current_val = get_from_bit_set(active_index);

        if (current_val != last_val) {
            raw[count] = true;
            raw_c.push_back(current_val);
        }
        last_val = current_val;
        active_index = read_buffer(active_index) + total_number[index_of_values(current_val) - 1] - 1;
        count++;
    }

    auto pos = raw_c.rbegin();
    for (int i = total_size - 2; i >= 0; i--) {
        if (raw[i] == true) {
            pos++;
            putc(*pos, output);
        } else {
            putc(*(pos + 1), output);
        }
//        cout << *pos;

    }
    putc('\n', output);
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

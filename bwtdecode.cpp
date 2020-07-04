//
// Created by Shawn Jin on 30/6/20.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <filesystem>

using namespace std;

#define BUFFER_SIZE 10000 // buffer size for read
#define OUTPUT_BUFFER_SIZE 10000

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

void read_from_stream(int next_index, int current_index, ifstream &str, fstream &out) {
    if (next_index / BUFFER_SIZE == current_index / BUFFER_SIZE) {
        // in same buffer no need to read
        return;
    }
    str.clear();
    int start_pos = (next_index / BUFFER_SIZE) * BUFFER_SIZE;
    str.seekg(start_pos);
    memset(buffer, 0, BUFFER_SIZE);
    str.read(buffer, BUFFER_SIZE);

    memset(buffer_count, 0, BUFFER_SIZE * 4);
    out.clear();
    int start_pos_out = (next_index / BUFFER_SIZE) * BUFFER_SIZE * 4 + total_size + 1;
    out.seekg(start_pos_out);
    out.read((char *) &buffer_count, BUFFER_SIZE * 4);
}

void construct_first_row(ifstream &str, fstream &out) {
    // write count to output as tmp file
    out.clear();
    out.seekp(total_size + 1);
    str.clear();
    str.seekg(0);
    int val;
    while (!str.eof()) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(buffer_count, 0, BUFFER_SIZE * 4);
        str.read(buffer, BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE; i++) {
            val = first_array[index_of_values(buffer[i])];
            buffer_count[i] = val;
            first_array[index_of_values(buffer[i])]++;
        }
        out.write(reinterpret_cast<const char *>(&buffer_count), sizeof(int) * BUFFER_SIZE);
    }

    for (int i = 1; i < 5; i++) {
        first_array[i] += first_array[i - 1];
    }
}

void clean_output(char *output) {
    filesystem::resize_file(output, total_size);
}

void reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        swap(str[i], str[len - 1 - i]);
    }
}

void decode(ifstream &input, fstream &output) {
    char *output_val = new char[OUTPUT_BUFFER_SIZE];
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
            output.clear();
            output.seekp(total_size - count - 1);
            reverse(output_val);
            output.write(output_val, strlen(output_val));
            memset(output_val, 0, OUTPUT_BUFFER_SIZE);
        }

        output_val[count % OUTPUT_BUFFER_SIZE] = current_val;

        index = index_of_values(current_val); // index of ATCG values
        p = active_index;
        active_index = first_array[index - 1] + buffer_count[index_of_buffer];
        read_from_stream(active_index, p, input, output);
        count++;
    }
    output.clear();
    output.seekp(total_size - 1);
    output.put('\n');
    delete[] output_val;
}


int main(int argc, char *argv[]) {
    ifstream encodedFile(argv[1]);
    fstream outputFile(argv[2], ios::out | ios::in | ios::trunc);

    encodedFile.clear();
    encodedFile.seekg(0, ios::end);
    total_size = encodedFile.tellg();

    construct_first_row(encodedFile, outputFile);
    decode(encodedFile, outputFile);
    clean_output(argv[2]);

    encodedFile.clear();
    encodedFile.close();

    outputFile.clear();
    outputFile.close();
    return 0;
}
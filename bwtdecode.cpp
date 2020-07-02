//
// Created by Shawn Jin on 30/6/20.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>

using namespace std;

#define BUFFER_SIZE 10000000 // buffer size for read

char buffer[BUFFER_SIZE];
int buffer_count[BUFFER_SIZE] = {0};
vector<vector<int> > first_row;
vector<int> first_array;


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

void read_from_stream(int next_index, int current_index, ifstream &str) {
    if (next_index / BUFFER_SIZE == current_index / BUFFER_SIZE) {
        // in same buffer no need to read
        return;
    }
    str.clear();
    int start_pos = (next_index / BUFFER_SIZE) * BUFFER_SIZE;
    str.seekg(start_pos);
    memset(buffer, 0, sizeof(buffer));
    str.read(buffer, sizeof(buffer));

    int sum[] = {0, 0, 0, 0, 0};
    memset(buffer_count, 0, sizeof(buffer_count));
    for (int i = 0; i < BUFFER_SIZE; i++) {
        char val = buffer[i];
        if (val == '\0') {
            break;
        }
        sum[index_of_values(val)]++;
        buffer_count[i] = sum[index_of_values(val)];
    }
}

void construct_first_row(ifstream &str) {
    str.clear();
    str.seekg(0, ios::end);
    int total_size = str.tellg();
    int m = total_size / BUFFER_SIZE + 1;
    str.seekg(0);

    first_row.resize(m, vector<int>(5));

    int count = 0;
    while (!str.eof()) {
        memset(buffer, 0, sizeof(buffer));
        str.read(buffer, sizeof(buffer));
        for (char &i : buffer) {
            first_row[count][index_of_values(i)]++;
        }
        count++;
        if (count < m) first_row[count].assign(first_row[count - 1].begin(), first_row[count - 1].end());
    }

    for (int i = 1; i < m; i++) {

    }
    first_row.insert(first_row.begin(), vector({0, 0, 0, 0, 0}));
}


void get_first_array() {
    first_array = vector(*first_row.rbegin());
    for (int i = 1; i < 5; i++) {
        // sum previous
        first_array[i] += first_array[i - 1];
    }
}


void decode(int total_size, ifstream &input, ofstream &output) {
    int active_index = 0;
    int count = 1;
    // reversed output
    output.seekp(total_size - 1);
    output.put('\n');
    read_from_stream(0, BUFFER_SIZE, input);
    while (true) {
        int m = active_index / BUFFER_SIZE; // m of vector first_row
        int index_of_buffer = active_index % BUFFER_SIZE;

        char current_val = buffer[index_of_buffer];

        if (current_val == '\n') {
            break;
        }

        output.clear();
        output.seekp(total_size - count - 1);
        output.put(current_val);

        int index = index_of_values(current_val); // index of ATCG values
        int p = active_index;
        active_index = first_array[index - 1] + buffer_count[index_of_buffer] + first_row[m][index] - 1;
        read_from_stream(active_index, p, input);
        count++;
    }
}


int main(int argc, char *argv[]) {
    ifstream encodedFile(argv[1]);
    ofstream outputFile(argv[2]);

    encodedFile.clear();
    encodedFile.seekg(0, ios::end);
    int total_size = encodedFile.tellg();

    construct_first_row(encodedFile);
    get_first_array();
    decode(total_size, encodedFile, outputFile);

    encodedFile.clear();
    encodedFile.close();

    outputFile.clear();
    outputFile.close();
    return 0;
}
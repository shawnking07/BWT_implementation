//
// Created by Shawn Jin on 30/6/20.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>

using namespace std;

#define BUFFER_SIZE 12000000 // buffer size for input

char values[5] = {'\n', 'A', 'C', 'G', 'T'};
char buffer[BUFFER_SIZE];
vector<vector<int> > first_row;
vector<int> first_array;


int index_of_values(char c){
    for(int i =0;i<5;i++){
        if (c == values[i]){
            return i;
        }
    }
    return -1;
}

void read_from_stream(int next_index, int current_index, ifstream &str) {
    str.clear();
    if (next_index / BUFFER_SIZE == current_index / BUFFER_SIZE) {
        // in same buffer no need to read
        return;
    }
    int start_pos = (next_index / BUFFER_SIZE) * BUFFER_SIZE;
    str.seekg(start_pos);
    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
    str.read(buffer, sizeof(char) * BUFFER_SIZE);
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
        memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
        str.read(buffer, sizeof(char) * BUFFER_SIZE);
        for (char &i : buffer) {
            auto itr = find(values, values + 5, i);
            if (itr == values + 5) {
                break;
            }
            int index = distance(values, itr);
            first_row[count][index]++;

        }
//        for (int i = 1; i < 5; i++) {
//            // sum previous
//            first_row[count][i] += first_row[count][i - 1];
//        }
        count++;
    }

    for (int i = 1; i < m; i++) {
        for (int j = 0; j < 5; j++) {
            first_row[i][j] += first_row[i - 1][j];
        }
    }
    first_row.insert(first_row.begin(), vector({0, 0, 0, 0, 0}));
}

/**
 * count shown times of buffer[index_in_buffer] in buffer
 * @param index_in_buffer
 * @return
 */
int number_of_s_in_buffer(int index_in_buffer) {
    int num = 0;
    char val = buffer[index_in_buffer];
    for (int i = 0; i <= index_in_buffer; i++) {
        if (buffer[i] == val) {
            num++;
        }
    }
    return num;
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
        active_index = first_array[index - 1] + number_of_s_in_buffer(index_of_buffer) + first_row[m][index] - 1;
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
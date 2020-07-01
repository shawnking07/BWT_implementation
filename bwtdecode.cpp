//
// Created by Shawn Jin on 30/6/20.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

using namespace std;

#define BUFFER_SIZE 10000000 // buffer size for input

struct val_index {
    char val;
    int index;
};

char values[5] = {'\n', 'A', 'C', 'G', 'T'};
char buffer[BUFFER_SIZE];

void construct_first_row(int first_row[], ifstream &str) {
    while (!str.eof()) {
        memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
        str.read(buffer, sizeof(char) * BUFFER_SIZE);
        for (char &i : buffer) {
            auto itr = find(values, values + 5, i);
            if (itr == values + 5) {
                break;
            }
            int index = distance(values, itr);
            first_row[index]++;

        }
    }

    for (int i = 1; i < 5; i++) {
        first_row[i] += first_row[i - 1];
    }
}

val_index index_val_in_encoded(int s, ifstream &str) {
    val_index res{};
    res.index = -1;
    str.clear();
    str.seekg(s);
    str.get(res.val);

    int count = 0;
    while (count <= s) {
        memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
        str.clear();
        str.seekg(0);
        str.read(buffer, sizeof(char) * BUFFER_SIZE);
        for (char &i : buffer) {
            if (i == res.val) {
                res.index++;
            }
            if (++count > s) {
                break;
            }
        }
    }
    return res;
}

void decode(const int first_row[], ifstream &str, ofstream &output) {
    int active_index = 0;
    int count = 0;
    // reversed output
    output.seekp(first_row[4] - 1);
    output.put('\n');
    while (true) {
        auto res = index_val_in_encoded(active_index, str);
        if (res.val == '\n') {
            break;
        }
        output.clear();
        output.seekp(first_row[4] - count - 2);
        output.put(res.val);
        int index = distance(values, find(values, values + 5, res.val)); // index of ATCG values

        active_index = first_row[index - 1] + res.index;
        count++;
    }
}

int main(int argc, char *argv[]) {
    ifstream encodedFile(argv[1]);
    ofstream outputFile(argv[2]);
    int first_row[5] = {0};

    construct_first_row(first_row, encodedFile);
    decode(first_row, encodedFile, outputFile);

    encodedFile.clear();
    encodedFile.close();

    outputFile.clear();
    outputFile.close();
    return 0;
}
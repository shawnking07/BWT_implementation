//
// Created by Shawn Jin on 30/6/20.
//

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

#define BUFFER_SIZE (4096*10)

using namespace std;

char buffer[BUFFER_SIZE];
int buffer_count[BUFFER_SIZE];
int current_start_index = -BUFFER_SIZE;
vector<vector<int> > first_array;
vector<int> total_number;
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

void construct_first_row(FILE *str) {
    // write count to output as tmp file
    rewind(str);
    int idx, count = 1;
    total_number.resize(5);

    first_array = vector(total_size / BUFFER_SIZE + 2, vector<int>(5));
    while (!feof(str)) {
        memset(buffer, 0, BUFFER_SIZE);
        fread(buffer, 1, BUFFER_SIZE, str);
        for (char i : buffer) {
            idx = index_of_values(i);
            if (idx == -1) {
                break;
            }
            first_array[count][idx]++;
            total_number[idx]++;
        }
        count++;
    }

    for (int i = 1; i < 5; i++) {
        total_number[i] += total_number[i - 1];
    }
}

void read_buffer(int start_index, FILE *fp) {
    if (start_index / BUFFER_SIZE == current_start_index / BUFFER_SIZE) {
        return;
    }
    memset(buffer, 0, sizeof(buffer));
    fseek(fp, start_index / BUFFER_SIZE * BUFFER_SIZE, SEEK_SET);
    fread(buffer, 1, BUFFER_SIZE, fp);

    // count number for each char
    auto count = first_array[start_index / BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int idx = index_of_values(buffer[i]);
        if (idx == -1) {
            break;
        }
        buffer_count[i] = count[idx]++;
    }

    current_start_index = start_index;
}

vector<int> generate_list(char c) {
    vector<int> res;
    int idx = index_of_values(c);
    if (idx == -1) {
        return res;
    }
    for (int i = total_number[idx - 1]; i < total_number[idx]; i++) {
        res.push_back(i);
    }
    return res;
}

int search(const string &str, FILE *fp) {
    vector<int> next;
    vector<int> tmp;
    for (auto it = str.rbegin(); it != str.rend(); it++) {
        if (it == str.rbegin()) {
            // last char
            next = generate_list(*it);
        } else {
            for (auto it_next: next) {
                read_buffer(it_next, fp);
                if (buffer[it_next % BUFFER_SIZE] == *it) {
                    tmp.push_back(buffer_count[it_next % BUFFER_SIZE] + total_number[index_of_values(*it) - 1]);
                }
            }
            if (tmp.empty()) {
                return 0;
            }
            next = tmp;
            tmp.clear();
        }
    }
    return next.size();
}

int main(int argc, char *argv[]) {
    FILE *encodedFile = fopen(argv[1], "rb");
    fseek(encodedFile, 0, SEEK_END);
    total_size = ftell(encodedFile);
    construct_first_row(encodedFile);
    string line;
    while (cin >> line) {
        cout << search(line, encodedFile) << endl;
    }
    return 0;
}
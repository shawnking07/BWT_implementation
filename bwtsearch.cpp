//
// Created by Shawn Jin on 30/6/20.
//

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

int b_size = 1440;
#define BUFFER_SIZE (b_size)

#define MAGIC_VALUE 250000  /* for dynamic allocating buffer size */

using namespace std;

struct RANGE {
    int start;
    int end;
};

char *buffer;
// occ table
vector<vector<int> > occ_table;
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

void construct_occ_table(FILE *str) {
    rewind(str);
    int idx, count = 1;
    total_number.resize(5);

    occ_table = vector(total_size / BUFFER_SIZE + 2, vector<int>(5));
    while (!feof(str)) {
        memset(buffer, 0, BUFFER_SIZE);
        fread(buffer, 1, BUFFER_SIZE, str);
        for (int i = 0; i < BUFFER_SIZE; i++) {
            idx = index_of_values(buffer[i]);
            if (idx == -1) {
                break;
            }
            occ_table[count][idx]++;
            total_number[idx]++;
        }
        count++;
    }

    for (int i = 1; i < occ_table.size(); i++) {
        for (int j = 0; j < 5; j++) {
            occ_table[i][j] += occ_table[i - 1][j];
        }
    }

    for (int i = 1; i < 5; i++) {
        total_number[i] += total_number[i - 1];
    }
}

/**
 * count number of c
 * @param index : start index
 * @param fp
 * @return
 */
int read_buffer(int index, FILE *fp) {
    int start_pos = index - index % BUFFER_SIZE;
    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
    fseek(fp, start_pos, SEEK_SET);
    fread(buffer, 1, BUFFER_SIZE, fp);

    char c = buffer[index % BUFFER_SIZE];
    bool reverse = (index % BUFFER_SIZE) > (BUFFER_SIZE - 1 / 2);
    // binary-count ??
    if (reverse) {
        // start from tail
        int count = occ_table[index / BUFFER_SIZE + 1][index_of_values(c)];
        for (int i = min(index - index % BUFFER_SIZE + BUFFER_SIZE, total_size) - 1;
             i >= index - index % BUFFER_SIZE; i--) {
            char val = buffer[i % BUFFER_SIZE];
            if (val != c) continue;
            if (i == index)return count;
            count--;
        }
    } else {
        // start from head
        int count = occ_table[index / BUFFER_SIZE][index_of_values(c)];
        for (int i = index - index % BUFFER_SIZE; i < min(index - index % BUFFER_SIZE + BUFFER_SIZE, total_size); i++) {
            char val = buffer[i % BUFFER_SIZE];
            if (val != c) continue;
            count++;
            if (i == index)return count;
        }
    }
    return -1;
}

RANGE generate_range(char c) {
    RANGE res{};
    int idx = index_of_values(c);
    if (idx == -1) {
        return res;
    }
    res.start = total_number[idx - 1];
    res.end = total_number[idx] - 1;
    return res;
}

int search(const string &str, FILE *fp) {
    int count = 0, i;
    auto r = generate_range(*str.rbegin());
    RANGE tmp{-1, -2};
    if (str.size() == 1) {
        count += r.end - r.start + 1;
        return count;
    }
    for (auto it = str.rbegin() + 1; it != str.rend(); it++) {
        fseek(fp, r.start, SEEK_SET);
        for (i = 0; i <= r.end - r.start; i++) {
            char c = getc(fp);
            if (c == *it) {
                tmp.start = read_buffer(r.start + i, fp) + total_number[index_of_values(*it) - 1] - 1;
                break;
            }
        }

        for (i = 0; i <= r.end - r.start; i++) {
            fseek(fp, r.end - i, SEEK_SET);
            char c = getc(fp);
            if (c == *it) {
                tmp.end = read_buffer(r.end - i, fp) + total_number[index_of_values(*it) - 1] - 1;
                break;
            }
        }

        r = tmp;
        tmp = {-1, -2};
    }
    count += r.end - r.start + 1;

    return count;
}

int main(int argc, char *argv[]) {
    FILE *encodedFile = fopen(argv[1], "rb");
    fseek(encodedFile, 0, SEEK_END);
    total_size = ftell(encodedFile);
    b_size = max(total_size / MAGIC_VALUE, 64);
    buffer = new char[b_size];
    construct_occ_table(encodedFile);
    string line;
    while (cin >> line) {
        cout << search(line, encodedFile) << endl;
    }
    return 0;
}
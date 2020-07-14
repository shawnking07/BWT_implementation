//
// Created by Shawn Jin on 30/6/20.
//

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

#define BUFFER_SIZE (4096*10)

using namespace std;

struct RANGE {
    int start;
    int end;
};

char buffer[BUFFER_SIZE];
int buffer_count[BUFFER_SIZE];
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
    // write count to output as tmp file
    rewind(str);
    int idx, count = 1;
    total_number.resize(5);

    occ_table = vector(total_size / BUFFER_SIZE + 2, vector<int>(5));
    while (!feof(str)) {
        memset(buffer, 0, BUFFER_SIZE);
        fread(buffer, 1, BUFFER_SIZE, str);
        for (char i : buffer) {
            idx = index_of_values(i);
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

int read_buffer(int index, FILE *fp) {
    bool reverse = index % BUFFER_SIZE > (BUFFER_SIZE / 2);
    int start_pos = index - index % BUFFER_SIZE;
    memset(buffer, 0, sizeof(buffer));
    fseek(fp, start_pos, SEEK_SET);
    fread(buffer, 1, BUFFER_SIZE, fp);
    auto count = occ_table[index / BUFFER_SIZE];
    if (reverse) {
        count = occ_table[index / BUFFER_SIZE + 1];
    }

    if (reverse) {
        for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
            int idx = index_of_values(buffer[i]);
            if (idx == -1) {
                continue;
            }
            buffer_count[i] = count[idx]--;
            if (i == index % BUFFER_SIZE)return buffer_count[i] - 1;
        }
    } else {

        for (int i = 0; i < BUFFER_SIZE; i++) {
            int idx = index_of_values(buffer[i]);
            if (idx == -1) {
                break;
            }
            buffer_count[i] = count[idx]++;
            if (i == index % BUFFER_SIZE)return buffer_count[i];
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
    res.end = total_number[idx];
    return res;
}

char get_c_pos(FILE *input, int pos) {
    fseek(input, pos, SEEK_SET);
    return getc(input);
}

int search(const string &str, FILE *fp) {
    int count = 0, pos, pos2;
    auto r = generate_range(*str.rbegin());
    RANGE next{.start=-1, .end=-2};
    RANGE tmp{.start=-1, .end=-2};
    for (int i = r.start / BUFFER_SIZE; i < r.end / BUFFER_SIZE + 1; i++) {
        next.start = max(r.start, i * BUFFER_SIZE);
        next.end = min((i + 1) * BUFFER_SIZE, r.end) - 1;
        if (str.size() == 1) {
            count += next.end - next.start + 1;
            continue;
        }
        for (auto it = str.rbegin() + 1; it != str.rend(); it++) {
            tmp.start = -1;
            tmp.end = -2;
            for (pos = next.start; pos <= next.end; pos++) {
                if (get_c_pos(fp, pos) == *it) {
                    tmp.start = read_buffer(pos, fp) + total_number[index_of_values(*it) - 1];
                    break;
                }
            }
            for (pos2 = next.end; pos2 >= pos; pos2--) {
                if (get_c_pos(fp, pos2) == *it) {
                    tmp.end = read_buffer(pos2, fp) + total_number[index_of_values(*it) - 1];
                    break;
                }
            }
            next = tmp;
            if (tmp.end - tmp.start < 0 || tmp.start == -1 || tmp.end == -2) {
                break;
            }
        }
        count += next.end - next.start + 1;
    }
    return count;
}

int main(int argc, char *argv[]) {
    FILE *encodedFile = fopen(argv[1], "rb");
    fseek(encodedFile, 0, SEEK_END);
    total_size = ftell(encodedFile);
    construct_occ_table(encodedFile);
    string line;
    while (cin >> line) {
        cout << search(line, encodedFile) << endl;
    }
    return 0;
}
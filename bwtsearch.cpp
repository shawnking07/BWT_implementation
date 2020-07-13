//
// Created by Shawn Jin on 30/6/20.
//

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <bitset>

#define BUFFER_SIZE (4096)

using namespace std;

struct RANGE {
    int start;
    int end;
};

char buffer[BUFFER_SIZE];
int buffer_count[BUFFER_SIZE];
int current_start_index = -BUFFER_SIZE;
vector<vector<int> > first_array;
vector<int> total_number;
int total_size = 0;
bitset<30 * 1024 * 1024> last_T;
unsigned int pos_of_end = -1;
int last_n = -1;

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

    for (int i = 1; i < first_array.size(); i++) {
        for (int j = 0; j < 5; j++) {
            first_array[i][j] += first_array[i - 1][j];
        }
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
    fseek(fp, start_index - start_index % BUFFER_SIZE, SEEK_SET);
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
                read_buffer(pos, fp);
                if (buffer[pos % BUFFER_SIZE] == *it) {
                    tmp.start = buffer_count[pos % BUFFER_SIZE] + total_number[index_of_values(*it) - 1];
                    break;
                }
            }
            for (pos2 = next.end; pos2 >= pos; pos2--) {
                read_buffer(pos2, fp);
                if (buffer[pos2 % BUFFER_SIZE] == *it) {
                    tmp.end = buffer_count[pos2 % BUFFER_SIZE] + total_number[index_of_values(*it) - 1];
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
    construct_first_row(encodedFile);
    string line;
    while (cin >> line) {
        cout << search(line, encodedFile) << endl;
    }
    return 0;
}
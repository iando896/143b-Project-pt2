#include <string.h>
#include <iostream>
#include <deque>
#include <vector>
#include <fstream>
#include <string>
using namespace std;

#define PM_SIZE 524288
#define PAGE_SIZE 512
#define BLOCK_SIZE 1024

int PM[PM_SIZE];
int D[BLOCK_SIZE][PAGE_SIZE];
deque<int> free_frames;

void printPM() {
    cout << "-----Printing PM-----" << endl;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < PAGE_SIZE; j++) {
            int index = i * 512 + j;
            if (PM[index] != 0) {
                cout << "Frame " << i << " at index " << index << " = " << PM[index] << endl;
            }
        }
    }
}

void printDisk() {
    cout << "-----Printing Disk-----" << endl;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < PAGE_SIZE; j++) {
            if (D[i][j] != 0) {
                cout << "Disk block " << i << " at page " << j <<  " = " << D[i][j] << endl;
            }
        }
    }
}

int allocate_first_free() {
    int alloc = free_frames.front();
    free_frames.pop_front();
    return alloc;
}

void allocate(int frame) {
    for (auto it = free_frames.begin(); it != free_frames.end(); it++) {
        if (*it == frame) {
            free_frames.erase(it);
            return;
        }
    }
}

void init() {
    for (int i = 0; i < PM_SIZE; i++) {
        PM[i] = 0;
    }

    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < PAGE_SIZE; j++) {
            D[i][j] = 0;
        }
    }

    //Init free_frames list
    free_frames = deque<int>(PAGE_SIZE, 0);
    for (int i = 0; i < PAGE_SIZE; i++) {
        free_frames[i] = i;
    }
    //Allocate first two frames for segment table
    allocate_first_free();
    allocate_first_free();
}

void read_block(int b, int m) {
    memcpy(&D[b], &PM[m], PM_SIZE * 4);
}

vector<int> split_three(string s, char delim, int start) {
    int start_index = start;
    int end_index = 0;
    vector<int> ans;
    for (int i = 0; i < 3; i++) {
    //loop through string
        string num;
        int j = start_index;
        for (; j <= s.size(); j++) {
            //cout << segmentLine.size() << endl;    
            if (j == s.size() or s[j] == delim) {
                end_index = j;
                // cout << "Start: " << start_index << endl;
                // cout << "End: " << end_index << endl;
                num = s.substr(start_index, end_index - start_index);
                start_index = j + 1;
                break;
            }
        }
        // cout << "Num: " << num << endl;
        ans.push_back(stoi(num));
    }
    ans.push_back(start_index);
    return ans;
}

vector<int> split(string s, char delim) {
    int start_index = 0;
    int end_index = 0;
    vector<int> ans;

    while (start_index < s.size()) {
        string num;
        int j = start_index;
        for (; j <= s.size(); j++) {
            //cout << segmentLine.size() << endl;    
            if (j == s.size() or s[j] == delim) {
                end_index = j;
                // cout << "Start: " << start_index << endl;
                // cout << "End: " << end_index << endl;
                num = s.substr(start_index, end_index - start_index);
                start_index = j + 1;
                break;
            }
        }
        // cout << "Num: " << num << endl;
        ans.push_back(stoi(num));
    }
    return ans;
}

void init_segments() {
    string file_name;
    fstream init_fs;
    while(true) {
        cout <<"Provide init file name: "; cin >> file_name;
        init_fs.open(file_name);
        if (!init_fs.is_open()) {
            init_fs.close();
            cout << "Failed to open init file" << endl;
        } else {
            break;
        }
    }

    string segmentLine;
    getline(init_fs, segmentLine);
    cout << segmentLine << endl;

    int start = 0;
    while (start < segmentLine.size()) {
        vector<int> segment_data = split_three(segmentLine, ' ', start);
        PM[segment_data[0] * 2] = segment_data[1];
        PM[segment_data[0] * 2 + 1] = segment_data[2];
        if (segment_data[2] > 0) {
            allocate(segment_data[2]);
        }
        start = segment_data[3];
    }
    

    string pageLine;
    getline(init_fs, pageLine);
    cout << pageLine << endl;

    start = 0;
    while (start < pageLine.size()) {
        vector<int> page_data = split_three(pageLine, ' ', start);
        // cout << "Segment: " << page_data[0] << endl;
        // cout << "Page num: " << page_data[1] << endl;
        // cout << "Frame num: " << page_data[2] << endl;
        if (PM[page_data[0] * 2 + 1] > 0) {
            //cout << "Entered" << endl;
            PM[PM[page_data[0] * 2 + 1] * 512 + page_data[1]] = page_data[2];
            
            allocate(page_data[2]);
        } else { //add to disk
            D[abs(PM[page_data[0] * 2 + 1])][page_data[1]] = page_data[2];
        }
        start = page_data[3];
    }

    init_fs.close();    
}

void translate_va() {
    string file_name;
    fstream va_fs;
    while (true) {
        cout <<"Provide VA file name: "; cin >> file_name;
        va_fs.open(file_name);
        if (!va_fs.is_open()) {
            va_fs.close();
            cout << "Failed to open VA file" << endl;
        } else {
            break;
        }
    }
    //open output file
    fstream output;
    output.open("output.txt", fstream::out);
    if (!output.is_open()) {
        output.close();
        cout << "Failed to open Output file" << endl;
    }

    string address_line;
    getline(va_fs, address_line);

    vector<int> addresses = split(address_line, ' ');

    for (int i = 0; i < addresses.size(); i++) {
        cout << "Address: " << addresses[i] << endl;
        int s = addresses[i] >> 18;
        int w = addresses[i] & 0x1ff;
        int p = addresses[i] >> 9 & 0x1ff;
        int pw = addresses[i] & 0x3ffff;

        if (pw >= PM[2 * s]) {
            //Error
            output << "-1 ";
        } else {
            if (PM[2 * s + 1] < 0) { //page in disk

            } else if (PM[PM[2 * s + 1] * 512 + p] < 0) { //page in other part of disk

            }
            //cout << PM[PM[2* s + 1] * 512 + p] * 512 + w << endl;;
            output << PM[PM[2* s + 1] * 512 + p] * 512 + w << " ";
        }

    }
    output.close();
    va_fs.close();
}

int main() {
    init();
    //open init file
    init_segments();
    printPM();
    printDisk();
    translate_va();
    return 0;
}
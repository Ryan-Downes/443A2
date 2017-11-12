#include <algorithm>

#include <stdlib.h>

#include <iostream>

#include <fstream>

#include <string>

#include <sys/timeb.h>

#include "library.cc"

int main(int argc, char** argv) {
    char * buffer = (char*) malloc (2);
    std::ifstream csv_file;
    csv_file.open(argv[1], std::ifstream::in);
    FILE *page_file = fopen(argv[2], "w+b");
    int page_size = atoi(argv[3]);
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    Page *page = new Page();
    init_fixed_len_page(page, page_size, record_size);
    int total_records = 0;
    int total_pages = 0;
    char* buf;
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;
    std::string line;
    while (std::getline(csv_file, line)) {
        std::string line;
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());
        Record r;
        fixed_len_read((char*)line.c_str(), record_size, &r);
        int slot_index = add_fixed_len_page(page, &r);
        if (slot_index == -1) {  // page is full
            total_pages++;
            fwrite(page->data, 1, page->page_size, page_file);
            fflush(page_file);
            init_fixed_len_page(page, page_size, record_size);
        }
        total_records++;
    }
    ftime(&t);
    int total_run_time = (int) ((t.time * 1000) + t.millitm) - start_time_in_ms;
    csv_file.close();
    fclose(page_file);
    fprintf(stdout, "NUMBER OF RECORDS: %d\n", total_records);
    fprintf(stdout, "NUMBER OF PAGES: %d\n", total_pages);
    fprintf(stdout, "TIME: %d milliseconds\n", total_run_time);
    return 0;
}
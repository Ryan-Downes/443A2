#include "library.cc"

#include <algorithm>

#include <stdlib.h>

#include <iostream>

#include <fstream>

#include <string>

#include <sys/timeb.h>

#include <string.h>

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Argument is not correct");
        exit(1);
    }
    char * buffer = (char*) malloc (2);
    std::ifstream csv_file;
    csv_file.open(argv[1], std::ifstream::in);
    FILE *page_file = fopen(argv[2], "w+b");
    int page_size = atoi(argv[3]);
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    Page *page;
    page = (Page *)malloc(sizeof(Page) + (page_size/record_size));
    page->data = (char *)malloc(page_size);
    init_fixed_len_page(page, page_size, record_size);
    int total_records = 0;
    int total_pages = 0;
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;
    std::string line;
    while (std::getline(csv_file, line)) {
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());
        Record r;
        fixed_len_read((char*)line.c_str(), record_size, &r);
        int index = add_fixed_len_page(page, &r);
        if (index == -1) {
            total_pages++;
            fwrite(page->data, 1, page->page_size, page_file);
            fflush(page_file);
            bzero(page->data, page_size);
            init_fixed_len_page(page, page_size, record_size);
            add_fixed_len_page(page, &r);
        }
        total_records++;
    }
    int free_slot = fixed_len_page_freeslots(page);
    if(free_slot != fixed_len_page_capacity(page)){
            total_pages++;
            for(int k =0;k<fixed_len_page_capacity(page);k++){
                if(page->mapping[k]=='1'){
                    fwrite(page->data + k*record_size, 1, record_size, page_file);
                }
            }
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
#include "library.cc"

#include <algorithm>

#include <stdlib.h>

#include <iostream>

#include <fstream>

#include <string>

#include <sys/timeb.h>

#include <string.h>

#include <vector>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Argument is not correct");
        exit(1);
    }
    int page_size = atoi(argv[2]);
    std::ofstream myfile;
    myfile.open ("result.csv");
    FILE *pageFile = fopen(argv[1], "r");
    int total_records = 0;
    int total_pages = 0;
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;
    Page *page;
    page = (Page *)malloc(sizeof(Page) + (page_size/record_size));
    page->data = (char *)malloc(page_size);
    init_fixed_len_page(page, page_size, record_size);
    fread(page->data, 1, page->page_size, pageFile);
    while (!feof(pageFile)) {
        for (int i = 0; i < fixed_len_page_capacity(page); ++i) {
                page->mapping[i]='1';
        }
        total_pages++;
        for (int k = 0; k < fixed_len_page_capacity(page); ++k) {
                Record r;
                read_fixed_len_page(page,k,&r);
                for (int i=0; i<r.size();i++){
                    myfile << ("%s",r[i]);
                    if(i != r.size()-1){
                        myfile << ",";
                    }
                    else{
                        myfile << "\n";
                    }
                }
                total_records++;
        }
        bzero(page->data, page_size);
        fread(page->data, 1, page->page_size, pageFile);
    }
    std::string line = page->data;
    if(line.length() > 0){
        total_pages++;
        for (int i = 0; i < line.length()/record_size; ++i) {
                page->mapping[i]='1';
        }
        for (int k = 0; k < line.length()/record_size; ++k) {
            Record r;
            read_fixed_len_page(page,k,&r);
            for (int i=0; i<r.size();i++){
                myfile << ("%s",r[i]);
                if(i != r.size()-1){
                        myfile << ",";
                    }
                    else{
                        myfile << "\n";
                    }
            }
            total_records++;
        }
    }
    ftime(&t);
    int total_run_time = (int) ((t.time * 1000) + t.millitm) - start_time_in_ms;
    
    fprintf(stdout, "NUMBER OF RECORDS: %d\n", total_records);
    fprintf(stdout, "NUMBER OF PAGES: %d\n", total_pages);
    fprintf(stdout, "TIME: %d milliseconds\n", total_run_time);
    fclose(pageFile);
    myfile.close();
    return 0;
}

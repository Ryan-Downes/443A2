#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <vector>
#include "library.cc"
#include <sstream>
#include <string>


namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}


int main(int argc, char** argv) {
    bool output = true;
    char *col_store_directory = argv[1];
    int attribute_id = atoi(argv[2]);
    const char* path = (col_store_directory + (std::string) "/" + patch::to_string(attribute_id)).c_str();

    FILE *col_heapfile = fopen(path, "r+");
    int page_size = atoi(argv[6]);
    Heapfile *heap = new Heapfile();
    init_heapfile(heap, page_size, col_heapfile);

    int return_id = atoi(argv[3]);
    int select2_flag = 0;
    FILE *return_col_heapfile;
    Heapfile *return_heap;

    if (return_id == attribute_id){
        FILE *return_col_heapfile = col_heapfile;
        Heapfile *return_heap = heap;
        select2_flag = 1;
    }else{
        const char* return_path = (col_store_directory + (std::string) "/" + patch::to_string(return_id)).c_str();
        FILE *return_col_heapfile = fopen(return_path, "r");
        Heapfile *return_heap = new Heapfile();
        init_heapfile(return_heap, page_size, return_col_heapfile);
    }

    char *start = argv[4];
    char *end = argv[5];

    FILE *p_file = NULL;
    p_file = fopen(path,"rb");
    fseek(p_file,0,SEEK_END);
    int dir_char = ftell(p_file);
    fclose(p_file);


    int pages_in_directory = dir_char /(page_size);

    int dir_offset = 0 ;
    fread(&dir_offset, sizeof(int), 1, col_heapfile);

    int page_offset = 0 ;
    int freespace = 0;
    int pos = 0;
    int num_records = 0;

    Page page;
    Page return_page;

    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;
    int last_itter = 0;
    while ((dir_offset != 0) &(last_itter != 1)) {
        // check if they are things on the last page to be checked
        if (dir_offset = 0 ){
            last_itter = 1;
            pos = ftell(col_heapfile);
            fseek(col_heapfile, 0, SEEK_END);
            if (ftell(col_heapfile) - (pos + page_size) < 0) {
                break;
            }
            fseek(col_heapfile, pos, SEEK_SET);
        }
        for (int i = 0; i < pages_in_directory; ++i) {
            fread(&page_offset, sizeof(int), 1, col_heapfile);
            fread(&freespace, sizeof(int), 1, col_heapfile);
            if (freespace != page_size) {
                pos = ftell(col_heapfile);

                read_page(heap, page_offset, &page);
                if(select2_flag == 0){
                    read_page(return_heap, page_offset, &return_page);
                }
                for (int j = 0; j < fixed_len_page_capacity(&page); ++j) {
                    Record r;
                    read_fixed_len_page(&page,j, &r);
                    Record r_to_return;
                    if(select2_flag == 0){
                        read_fixed_len_page(&page, j, &r_to_return);
                    }else{
                        Record r_to_return = r;
                    }
                    if (r.empty()) {
                        continue;
                    }

                    for (Record::iterator it = r.begin(); it != r.end(); ++it) {
                        if (strncmp(*it, start, 5) >= 0 && strncmp(*it, end, 5) <= 0) {
            sizeof                num_records++;
                            int index = std::distance(r.begin(), it);

                            char *output_rec = new char[6];
                            strncpy(output_rec, r_to_return.at(index), 5);
                            output_rec[5] = '\0';
                            fprintf(stdout, "%c\n", output_rec);
                        }
                    }
                }
                fseek(col_heapfile, pos, SEEK_SET);
            }
        }
    }

    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    fclose(col_heapfile);
    if(select2_flag == 0){
        fclose(return_col_heapfile);
    }
    fprintf(stderr, "TOTAL TIME: %d milliseconds\n", (int) total_run_time);
    std::cout << "NUMBER OF MATCHING RECORDS: %d\n", (int) num_records;

    return 0;
}

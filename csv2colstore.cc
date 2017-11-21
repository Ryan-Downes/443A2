#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include "library.cc"

#include <sstream>

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
    std::ifstream csv_file;
    csv_file.open(argv[1]);
    char *colstore_directory = argv[2];
    int page_size = atoi(argv[3]);
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    std::vector<Record> *records = new std::vector<Record>();
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;
    int num_records = 0;
    while (csv_file) {
        std::string line;
        csv_file >> line;

        if (line.size() == 0) {
            continue;
        }
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());
        Record *rec = new Record;
        fixed_len_read((char*)line.c_str(), record_size, rec);
        records->push_back(*rec);
        num_records++;
    }

    csv_file.close();

    for (int i = 0; i < NUM_ATTRIBUTE; ++i) {
        const char* path = (colstore_directory + (std::string) "/" + patch::to_string(i)).c_str();
        //const char* path = pa.c_str();

        FILE *column_heapfile = fopen(path, "w+");

        Heapfile *heap = new Heapfile();
        init_heapfile(heap, page_size, column_heapfile);
        Page page;
        init_fixed_len_page(&page, page_size, record_size);
        Record *col_record = new Record();
        for (size_t j = 0; j < records->size(); ++j){
            col_record->push_back(records->at(j).at(i));
            if (col_record->size() == NUM_ATTRIBUTE) {
                int slot_index = add_fixed_len_page(&page, col_record);
                if (slot_index == -1) {
                    PageID pid = alloc_page(heap);

                    fseek(heap->page_ptr, pid * heap->page_size, SEEK_SET);
                    fwrite((&page)->data, 1, heap->page_size, heap->page_ptr);
                    fflush(heap->page_ptr);
                    fseek(heap->page_ptr, 0, SEEK_SET);

                    init_fixed_len_page(&page, page_size, record_size);
                    slot_index = add_fixed_len_page(&page, col_record);
                }
                write_fixed_len_page(&page, slot_index, col_record);
                col_record->clear();
            }
        }
        if (fixed_len_page_freeslots(&page) > 0) {
            PageID pid = alloc_page(heap);
            fseek(heap->page_ptr, pid * heap->page_size, SEEK_SET);
            fwrite((&page)->data, 1, heap->page_size, heap->page_ptr);
            fflush(heap->page_ptr);
            fseek(heap->page_ptr, 0, SEEK_SET);
        }
        delete col_record;
        delete heap;
        fclose(column_heapfile);
    }
    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;
    fprintf(stdout, "TOTAL TIME: %d milliseconds\n", (int) total_run_time);
    fprintf(stdout, "NUMBER OF RECORDS: %d\n",  num_records);
    return 0;
}

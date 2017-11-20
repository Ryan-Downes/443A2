#include "library.cc"

#include <algorithm>

#include <stdlib.h>

#include <iostream>

#include <fstream>

#include <string>

#include <sys/timeb.h>
#include <string.h>
int main(int argc, char** argv) {
    if (argc != 4){

     printf("Argument is not correct");
        exit(1);
    }
    FILE *page_file_ptr = fopen(argv[1], "r+");
    Heapfile *heapfile;
        std::string recordString = argv[2];
        if (recordString.find(":")==-1) {
  printf("Record ID parameter not in proper format.");
        exit(1);
    }

        RecordID *rid ;

        std::vector <char *> v;
        char * dup = strdup(recordString.c_str());
    char * token = strtok(dup, ":");
     v.push_back(token);
        token = strtok(NULL, ":");
  v.push_back(token);


        rid->page_id = atoi(v.at(0));
        rid->slot = atoi(v.at(1));
        free(dup);
        printf("%d\n",rid->page_id);
        printf("%d\n",rid->slot);
	int page_size = atoi(argv[3]);
    init_heapfile(heapfile, page_size, page_file_ptr);
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
        Page *p;
        read_page(heapfile, rid->page_id, p);
        Record * recordToEdit;
	read_fixed_len_page(p, rid->slot, recordToEdit);
	recordToEdit->clear();
	p->mapping[rid->slot]='0';
        write_page(p, heapfile, rid->page_id);
}


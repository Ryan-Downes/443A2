#include <sys/timeb.h>

#include "library.cc"

#include <algorithm>

#include <stdlib.h>

#include <iostream>

#include <fstream>

#include <string>

#include <sys/timeb.h>

#include <string.h>


#include <string.h>
int main(int argc, char** argv) {
    if (argc != 6){
        printf("Argument is not correct");
        exit(1);
    }
    char * buffer = (char*) malloc (2);
	int attribute_id = atoi(argv[2]);
	char * start = argv[3];
	char * end = argv[4];
    FILE *page_file_ptr = fopen(argv[1], "r");
    Heapfile *heapfile = (Heapfile*) malloc(sizeof(Heapfile));
    int page_size = atoi(argv[5]);
    init_heapfile(heapfile, page_size, page_file_ptr);
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    RecordIterator i = RecordIterator(heapfile);
     while (i.hasNext()){
     	Record r = i.next();
        if (strncmp(r.at(attribute_id), start, 10) >= 0 && strncmp(r.at(attribute_id), end, 10) <= 0){
			printf("%s | %s | %s", r.at(attribute_id),r.at(1),r.at(5));
		}		
 
    }
}


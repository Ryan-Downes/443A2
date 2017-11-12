#include "library.h"

#include <stdlib.h>

#include <string.h>
/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record) {
	int total_length = 0;
	for( int i =0; i < record->size(); i++){
		int str_len = strlen(record->at(i));
		total_length = total_length + str_len;
	}
	//This should be the same as NUM_ATTRIBUTE*ATTRIBUTE_SIZE
	return total_length;
}

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf) {
	for(int i =0; i < record->size();i++){
		memcpy(((char*)buf + i*ATTRIBUTE_SIZE), record->at(i), ATTRIBUTE_SIZE);
	}
}

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record){
     for (int i = 0; i < size/ATTRIBUTE_SIZE; ++i) {
        char *temp;
        temp = (char *) malloc (11);
        memset(temp, '\0', ATTRIBUTE_SIZE + 1);
        strncpy(temp, (char *)buf, ATTRIBUTE_SIZE);
        record->push_back(temp);
        buf = ((char*)buf) + ATTRIBUTE_SIZE;
    }
}

/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size){
	page->page_size = page_size;
	page->slot_size = slot_size;
	page->data = malloc(page_size);
	//Create a bitmap in the page corresponding to the slots, if the corresponding bit is 0, then it means that
	// slot is empty, if it is 1 then it means the slot has been occupied.
	for(int i =0; i < slot_size; i++){
		page->mapping[i] = 0;
	}
}
/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page){
	return page->page_size / page->slot_size;
}
/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page){
	int count = 0;
	for(int i=0; i < fixed_len_page_capacity(page);i++){
		if (page->mapping[i]==0){
			count = count + 1;
		}
	}
	return count;
}
/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r){
	int free_slot = fixed_len_page_freeslots(page);
	if(free_slot < 0){
		return -1;
	}
	else{
		for(int i=0; i < fixed_len_page_capacity(page);i++){
			if (page->mapping[i]==0){
				write_fixed_len_page(page,i,r);
				page->mapping[i]=1;
				return i;
			}
		}
	}
}
/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r){
	char *buf = ((char *)page->data + (slot * page->slot_size));
    fixed_len_write(r, buf);
}
/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
	if(page->mapping[slot] == 1){
		char *buf = ((char * )page->data + (slot * (page->slot_size)));
		fixed_len_read(buf, (page->slot_size), r);
	}
}

///**
// * Initalize a heapfile to use the file and page size given.
// */
//void init_heapfile(Heapfile *heapfile, int page_size, std:: ifstream file){
//	heapfile->file_ptr = file;
//	heapfile->page_size = page_size;
//}
//
///**
// * Allocate another page in the heapfile.  This grows the file by a page.
// */
//PageID alloc_page(Heapfile *heapfile){
//    Page *page = new Page();
//    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
//    init_fixed_len_page(page, heapfile->page_size, record_size);
//
//    //Page *heap_page = heapfile->file_ptr;
//    return 0;
//}
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
    page->data = (char*) malloc(page_size);
    //Create a bitmap in the page corresponding to the slots, if the corresponding bit is 0, then it means that
    // slot is empty, if it is 1 then it means the slot has been occupied.
    for(int i =0; i < page_size/slot_size; i++){
        page->mapping[i] = '0';
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
        if (page->mapping[i]=='0'){
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
    if(free_slot == 0){
        return -1;
    }
    else{
        for(int i=0; i < fixed_len_page_capacity(page);i++){
            if (page->mapping[i]=='0'){
                write_fixed_len_page(page,i,r);
                page->mapping[i]='1';
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
    if(page->mapping[slot] == '1'){
        char *buf = ((char * )page->data + (slot * (page->slot_size)));
        fixed_len_read(buf, (page->slot_size), r);
    }
}
char * tostr (int x)
{
    char string[10];


	sprintf(string,"%i",x);
    return string;
}

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE* file){
    heapfile->page_ptr = file;
    heapfile->page_size = page_size;
    heapfile->directorySlotSize =  ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    heapfile->offset=0;
    Page *directoryPage = new Page();
    heapfile->directoryLL = new PageNode();
    heapfile->directoryLL->offset = heapfile->offset;
    heapfile->directoryLL->next = NULL;
    heapfile->tail = heapfile->directoryLL;
    heapfile->offset=heapfile->offset+1;
    Page *dataPage = new Page();
    
    Record *r = new Record();
    r->push_back(tostr(heapfile->offset));//adds offset and free space to record;
    r->push_back(tostr(heapfile->page_size));
    add_fixed_len_page(directoryPage,r);
    
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
//update directory
    Page *page = new Page();
    int record_size = ATTRIBUTE_SIZE * NUM_ATTRIBUTE;
    init_fixed_len_page(page, heapfile->page_size, record_size);
    FILE* heap_page = (heapfile->page_ptr);
    heapfile->offset = heapfile->offset+1;
    writePageAt(heapfile,page,heapfile->offset);

    Page *directoryPage = getPageAt(heapfile,heapfile->tail->offset);
    Record *r = new Record();///new record for heap directory
    r->push_back(tostr(heapfile->offset));//adds offset and free space to record;
    r->push_back(tostr(heapfile->page_size));
    PageID offsetToReturn = heapfile->offset;
    if (add_fixed_len_page(directoryPage ,r)){//if record add fails, need to add new page to the directory
        heapfile->offset = heapfile->offset+1;
        offsetToReturn = heapfile->offset;
        PageNode *temp=new PageNode();//consider turning into a function whenever adding a new page to the direcctory
        temp->offset=heapfile->offset;
        temp->next=NULL;
        heapfile->tail->next=temp;
        heapfile->tail=temp;
		Page *tempPage = new Page();
        add_fixed_len_page(tempPage, r);
        writePageAt(heapfile, tempPage,heapfile->offset);
    }
    return offsetToReturn;
}
/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){
    int numRecords = heapfile->page_size/heapfile->directorySlotSize;
    int currentRecordsPast = 0; 
    PageNode * temp = heapfile->directoryLL;
    while (currentRecordsPast+numRecords < pid){
        temp = temp->next;
        if (temp==NULL){
            fputs ("PID out of range",stderr); exit (2);
        }
        currentRecordsPast=currentRecordsPast+numRecords;
    }
    currentRecordsPast = currentRecordsPast - numRecords;
    Page * directoryPage = getPageAt(heapfile,temp->offset);
    Record * r;
    read_fixed_len_page(directoryPage, pid-currentRecordsPast, r);
    page  = getPageAt(heapfile, atoi(r->front()));
    
}
/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){
     int numRecords = heapfile->page_size/heapfile->directorySlotSize;
    int currentRecordsPast = 0;
    PageNode * temp = heapfile->directoryLL;
    while (currentRecordsPast+numRecords < pid){
        temp = temp->next;
        if (temp==NULL){
            fputs ("PID out of range",stderr); exit (2);
        }
        currentRecordsPast=currentRecordsPast+numRecords;
    }
    currentRecordsPast = currentRecordsPast - numRecords;
    Page * directoryPage = getPageAt(heapfile, temp->offset);
    Record * r;
    read_fixed_len_page(directoryPage, pid-currentRecordsPast, r);
    writePageAt(heapfile, page, atoi(r->front()));



}
Page * getPageAt(Heapfile *heapfile, int offset){
    
    fseek(heapfile->page_ptr, offset, SEEK_SET);
    Page *page = new Page();
    int numSlots = heapfile->page_size/heapfile->directorySlotSize;
    char *buffer = (char*) malloc (sizeof(char)*heapfile->directorySlotSize+1);
    int count = 0;
//while loop to read each data record in a row
    while (count!=numSlots){
        Record *r;
        if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}
        fread (buffer,1,heapfile->directorySlotSize,heapfile->page_ptr);

        fixed_len_read(buffer, heapfile->directorySlotSize, r);
        count=count+1;
    }
    char *mappingbuffer = (char*) malloc (sizeof(char)*numSlots);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}
    fread (mappingbuffer,1,numSlots,heapfile->page_ptr);
	for(int i=0; i < fixed_len_page_capacity(page);i++){
            fread(mappingbuffer,1,numSlots,heapfile->page_ptr);
		page->mapping[i]=mappingbuffer[0];
        }

    return page;
}


int  writePageAt(Heapfile *heapfile, Page * page, int offset){

    fseek(heapfile->page_ptr, offset, SEEK_SET);
    fwrite(page->data, 1, page->page_size,heapfile->page_ptr );
    fwrite(page->mapping,1,page->page_size/page->slot_size, heapfile->page_ptr);
    return 0;
}
/**std::string tostr (int x)
{
    std::stringstream str;
    istr << x;
    return str.str();
}**/


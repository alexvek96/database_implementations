#ifndef SHT_TABLE_H
#define SHT_TABLE_H
#define S_MAX_NUM_OF_RECORDS ((BF_BLOCK_SIZE - sizeof(SHT_block_info))/sizeof(SHT_pair)) // per block

#define S_NUM_OF_BUCKETS 15
#include <record.h>
#include <ht_table.h>


//-----------------------------------------------------------------------------

typedef struct {
    int sfileDesc;                               // File Descriptor of SHT file
    long int snumBuckets;                        // number of hash buckets
    char stype;
    int snum_of_blocks;
    int sHashTable[S_NUM_OF_BUCKETS];              // Hash Table keeps the id of first block of the hash chain
    int sRecords_per_Bucket[S_NUM_OF_BUCKETS];     // stores total records (pairs) inserted in every hash bucket
    int sBlocks_per_Bucket[S_NUM_OF_BUCKETS];      // stores total blocks created as chain in every hash bucket
    int sBlocks_with_overflow[S_NUM_OF_BUCKETS];   // number of overflown blocks per hash bucket
} SHT_info;

typedef struct {
    int snum_of_records;     // number of records (pairs) in the Block
    int snext_block;         // index to the next Block
} SHT_block_info;

typedef struct {
    char name[15];          // Record.name
    int sblock_id;          // block id of SHT file where this Record.name is stored   
} SHT_pair;

int serror;
void* sdata;
SHT_block_info* sp_block_info;
SHT_info* sp_file_info;
SHT_info sfile_info;
SHT_block_info sblock_info;
BF_Block* sblock;
BF_Block* sblock_0;                   // the first block of the file storing all the file info
Record* p_record;
SHT_pair pair;
SHT_pair* p_pair;

//-----------------------------------------------------------------------------

/*Η συνάρτηση SHT_CreateSecondaryIndex χρησιμοποιείται για τη δημιουργία
και κατάλληλη αρχικοποίηση ενός αρχείου δευτερεύοντος κατακερματισμού με
όνομα sfileName για το αρχείο πρωτεύοντος κατακερματισμού fileName. Σε
περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται 0, ενώ σε διαφορετική
περίπτωση -1.*/
int SHT_CreateSecondaryIndex(
    char *sfileName, /* όνομα αρχείου δευτερεύοντος ευρετηρίου*/
    int buckets, /* αριθμός κάδων κατακερματισμού*/
    char* fileName /* όνομα αρχείου πρωτεύοντος ευρετηρίου*/);



/* Η συνάρτηση SHT_OpenSecondaryIndex ανοίγει το αρχείο με όνομα sfileName
και διαβάζει από το πρώτο μπλοκ την πληροφορία που αφορά το δευτερεύον
ευρετήριο κατακερματισμού.*/
SHT_info* SHT_OpenSecondaryIndex(
    char *sfileName /* όνομα αρχείου δευτερεύοντος ευρετηρίου */);

/*Η συνάρτηση SHT_CloseSecondaryIndex κλείνει το αρχείο που προσδιορίζεται
μέσα στη δομή header_info. Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται
0, ενώ σε διαφορετική περίπτωση -1. Η συνάρτηση είναι υπεύθυνη και για την
αποδέσμευση της μνήμης που καταλαμβάνει η δομή που περάστηκε ως παράμετρος,
στην περίπτωση που το κλείσιμο πραγματοποιήθηκε επιτυχώς.*/
int SHT_CloseSecondaryIndex( SHT_info* header_info );

/*Η συνάρτηση SHT_SecondaryInsertEntry χρησιμοποιείται για την εισαγωγή μιας
εγγραφής στο αρχείο κατακερματισμού. Οι πληροφορίες που αφορούν το αρχείο
βρίσκονται στη δομή header_info, ενώ η εγγραφή προς εισαγωγή προσδιορίζεται
από τη δομή record και το block του πρωτεύοντος ευρετηρίου που υπάρχει η εγγραφή
προς εισαγωγή. Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται 0, ενώ σε
διαφορετική περίπτωση -1.*/
int SHT_SecondaryInsertEntry(
    SHT_info* header_info, /* επικεφαλίδα του δευτερεύοντος ευρετηρίου*/
    Record record, /* η εγγραφή για την οποία έχουμε εισαγωγή στο δευτερεύον ευρετήριο*/
    int block_id /* το μπλοκ του αρχείου κατακερματισμού στο οποίο έγινε η εισαγωγή */);

/*Η συνάρτηση αυτή χρησιμοποιείται για την εκτύπωση όλων των εγγραφών που
υπάρχουν στο αρχείο κατακερματισμού οι οποίες έχουν τιμή στο πεδίο-κλειδί
του δευτερεύοντος ευρετηρίου ίση με name. Η πρώτη δομή περιέχει πληροφορίες
για το αρχείο κατακερματισμού, όπως αυτές είχαν επιστραφεί κατά το άνοιγμά
του. Η δεύτερη δομή περιέχει πληροφορίες για το δευτερεύον ευρετήριο όπως
αυτές είχαν επιστραφεί από την SHT_OpenIndex. Για κάθε εγγραφή που υπάρχει
στο αρχείο και έχει όνομα ίσο με value, εκτυπώνονται τα περιεχόμενά της
(συμπεριλαμβανομένου και του πεδίου-κλειδιού). Να επιστρέφεται επίσης το
πλήθος των blocks που διαβάστηκαν μέχρι να βρεθούν όλες οι εγγραφές. Σε
περίπτωση λάθους επιστρέφει -1.*/
int SHT_SecondaryGetAllEntries(
    HT_info* ht_info, /* επικεφαλίδα του αρχείου πρωτεύοντος ευρετηρίου*/
    SHT_info* header_info, /* επικεφαλίδα του αρχείου δευτερεύοντος ευρετηρίου*/
    char* name /* το όνομα στο οποίο γίνεται αναζήτηση */);

int sHashStatistics(char* sfilename);
/* function prints:
1) number of blocks in the file
2) minimum, average and maximum number of records the buckets contain
3) average number of blocks that exist in a bucket
4) number of buckets that have overflown, and how many of them exist in every bucket
function returns '0' for success, '-1' for failure
*/
#endif // SHT_FILE_H

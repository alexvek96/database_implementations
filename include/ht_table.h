#ifndef HT_TABLE_H
#define HT_TABLE_H
#define MAX_NUM_OF_RECORDS ((BF_BLOCK_SIZE - sizeof(HT_block_info))/sizeof(Record)) // per block
#define NUM_OF_BUCKETS 15
#include <record.h>

//-----------------------------------------------------------------------------

typedef struct {                                // File meta-data
    int fileDesc;                               // File Descriptor
    long int numBuckets;                        // number of hash buckets
    int num_of_blocks;
    char type;                                  // file type (HT)
    int HashTable[NUM_OF_BUCKETS];              // Hash Table keeps the id of first block of the hash chain
    int Records_per_Bucket[NUM_OF_BUCKETS];     // stores total records inserted in every hash bucket
    int Blocks_per_Bucket[NUM_OF_BUCKETS];      // stores total blocks created as chain in every hash bucket
    int Blocks_with_overflow[NUM_OF_BUCKETS];   // number of overflown blocks per hash bucket
} HT_info;

typedef struct {
    int num_of_records;             // number of records in the Block
    int next_block;                 // index to the next Block    
} HT_block_info;

int error, i;
void* data;
HT_block_info* p_block_info;
HT_info* p_file_info;
HT_info file_info;
HT_block_info block_info;
BF_Block* block;
BF_Block* block_0;                  // the first block of the file storing all the file info
Record* p_record;

//-----------------------------------------------------------------------------

/*Η συνάρτηση HT_CreateFile χρησιμοποιείται για τη δημιουργία
και κατάλληλη αρχικοποίηση ενός άδειου αρχείου κατακερματισμού
με όνομα fileName. Έχει σαν παραμέτρους εισόδου το όνομα του
αρχείου στο οποίο θα κτιστεί ο σωρός και των αριθμό των κάδων
της συνάρτησης κατακερματισμού. Σε περίπτωση που εκτελεστεί
επιτυχώς, επιστρέφεται 0, ενώ σε διαφορετική περίπτωση -1.*/
int HT_CreateFile(char *fileName, int buckets);

/*Η συνάρτηση HT_OpenFile ανοίγει το αρχείο με όνομα filename
και διαβάζει από το πρώτο μπλοκ την πληροφορία που αφορά το
αρχείο κατακερματισμού. Κατόπιν, ενημερώνεται μια δομή που κρατάτε
όσες πληροφορίες κρίνονται αναγκαίες για το αρχείο αυτό προκειμένου
να μπορείτε να επεξεργαστείτε στη συνέχεια τις εγγραφές του. Αφού
ενημερωθεί κατάλληλα η δομή πληροφοριών του αρχείου, την επιστρέφετε.
Σε περίπτωση που συμβεί οποιοδήποτε σφάλμα, επιστρέφεται τιμή NULL.
Αν το αρχείο που δόθηκε για άνοιγμα δεν αφορά αρχείο κατακερματισμού,
τότε αυτό επίσης θεωρείται σφάλμα. */
HT_info* HT_OpenFile(char *fileName);   /*όνομα αρχείου*/

/*Η συνάρτηση HT_CloseFile κλείνει το αρχείο που προσδιορίζεται μέσα
στη δομή header_info. Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται
0, ενώ σε διαφορετική περίπτωση -1. Η συνάρτηση είναι υπεύθυνη και για την
αποδέσμευση της μνήμης που καταλαμβάνει η δομή που περάστηκε ως παράμετρος,
στην περίπτωση που το κλείσιμο πραγματοποιήθηκε επιτυχώς.*/
int HT_CloseFile(HT_info* header_info );

/*Η συνάρτηση HT_InsertEntry χρησιμοποιείται για την εισαγωγή μιας εγγραφής
στο αρχείο κατακερματισμού. Οι πληροφορίες που αφορούν το αρχείο βρίσκονται στη
δομή header_info, ενώ η εγγραφή προς εισαγωγή προσδιορίζεται από τη δομή record.
Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφετε τον αριθμό του block στο οποίο
έγινε η εισαγωγή (blockId) , ενώ σε διαφορετική περίπτωση -1.*/
int HT_InsertEntry(HT_info* header_info, /*επικεφαλίδα του αρχείου*/ Record record /*δομή που προσδιορίζει την εγγραφή*/);

/* Η συνάρτηση αυτή χρησιμοποιείται για την εκτύπωση όλων των εγγραφών που υπάρχουν
στο αρχείο κατακερματισμού οι οποίες έχουν τιμή στο πεδίο-κλειδί ίση με value.
Η πρώτη δομή δίνει πληροφορία για το αρχείο κατακερματισμού, όπως αυτή είχε επιστραφεί
από την HT_OpenIndex. Για κάθε εγγραφή που υπάρχει στο αρχείο και έχει τιμή στο πεδίο-κλειδί
(όπως αυτό ορίζεται στην HT_info) ίση με value, εκτυπώνονται τα περιεχόμενά της
(συμπεριλαμβανομένου και του πεδίου-κλειδιού). Να επιστρέφεται επίσης το πλήθος των blocks που
διαβάστηκαν μέχρι να βρεθούν όλες οι εγγραφές. Σε περίπτωση επιτυχίας επιστρέφει το πλήθος των 
blocks που διαβάστηκαν, ενώ σε περίπτωση λάθους επιστρέφει -1.*/
int HT_GetAllEntries(HT_info* header_info, /*επικεφαλίδα του αρχείου*/ int value /*τιμή του πεδίου-κλειδιού προς αναζήτηση*/);

int HashStatistics(char* filename);
/* function prints:
1) number of blocks in the file
2) minimum, average and maximum number of records the buckets contain
3) average number of blocks that exist in a bucket
4) number of buckets that have overflown, and how many of them exist in every bucket
function returns '0' for success, '-1' for failure
*/
#endif // HT_FILE_H

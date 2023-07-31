#ifndef RECORD_H
#define RECORD_H
#define RECORDS_NUM 200       // you can change it if you want
#define FILE_NAME "data.db"

typedef enum Record_Attribute {
  ID,
  NAME,
  SURNAME,
  CITY
} Record_Attribute;

typedef struct Record {
  char record[15];
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

Record randomRecord();

void printRecord(Record record);

#endif

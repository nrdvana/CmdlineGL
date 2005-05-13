/*****************************************************************************\
* WARNING: Ugly C code follows.  You are hereby forbidden to look at it, so   *
*   I don't want to hear about how bad it is or possible improvements.        *
* Do not reuse this code, as it is bug-ridden.                                *
* In particular, I don't free any of of my allocated memory on the assumption *
* that the OS will free it on completion of the program.  This program is a   *
* quick-and-dirty conversion from the earlier one I wrote up in Java, and     *
* intended solely for the purpose of generating static hash tables for        *
* CmdlineGL as part of the 'make' sequence.                                   *
*   If someone really wants to rewrite it, please do so in perl.  I would     *
* have, except I'm not terribly fluent with perl, and one of my goals in      *
* writing CmdlineGL was to abuse technology.  Another one of my goals was to  *
* not spend much time on the project, since it was just a joke anyway.        *
*                           YOU HAVE BEEN WARNED                              *
\*****************************************************************************/

#include <stdio.h>

#include "HashFunc.c"

void die(const char *msg) {
	printf("%s\n", msg);
	exit(-1);
}

typedef struct TableEntry_t {
	int count;
	char **names;
	char **values;
} TableEntry;

const char *DELIM=" \t\n\r";
char buffer[1024];
int main(int argc, char**argv) {
	TableEntry *table, *bucket;
	int tableSize, i, ent, red, lineNum;
	char *line, *TableName, *BucketName, *EntryStruct, *name, *value;

	if (argc != 5)
		die("Usage: HashTableGenUtil BucketCount TableName BucketName EntryStruct < table_data > TableData.c");

	tableSize= strtol(argv[1], (char*) NULL, 10);
	if (tableSize <= 0) die("Invalid table size specified\n");

	table= (TableEntry*) malloc(tableSize * sizeof(TableEntry));
	if (!table) die("Out of Mem");
	for (i=0; i<tableSize; i++) {
		table[i].count= 0;
		table[i].names= (char**) malloc(0); // Yeah, this is pretty cheesy, hehe
		table[i].values= (char**) malloc(0);
	}
	TableName= argv[2];
	BucketName= argv[3];
	EntryStruct= argv[4];

	line= NULL;
	lineNum= 0;
	while (line= fgets(buffer, sizeof(buffer), stdin)) {
		lineNum++;

		name= (char*) strtok(line, DELIM);
		while (name && *name=='\0')
			name= (char*) strtok(NULL, DELIM);
		if (!name) // empty line
			continue;
		if (!(name= (char*) strdup(name))) die("Out of Mem");

		value= (char*) strtok(NULL, DELIM);
		while (value && *value=='\0')
			value= (char*) strtok(NULL, DELIM);
		if (!value) { printf("Key without value on line %d\n", lineNum); exit(-1); }
		if (!(value= (char*) strdup(value))) die("Out of Mem");

		bucket= &table[CalcHash(name) % tableSize];
		bucket->count++;
		if (!(bucket->names= (char**) realloc(bucket->names, sizeof(char*) * bucket->count))) die("Out of Mem");
		if (!(bucket->values= (char**) realloc(bucket->values, sizeof(char*) * bucket->count))) die("Out of Mem");
		bucket->names[bucket->count-1]= name;
		bucket->values[bucket->count-1]= value;
	}
	for (i=0; i<tableSize; i++) {
		if (table[i].count) {
			printf("%s %s%d[%d] = {", EntryStruct, BucketName, i, table[i].count);
			for (ent= 0; ent < table[i].count; ent++) {
				if (ent != 0) printf(", ");
				printf("{\"%s\", %s}", table[i].names[ent], table[i].values[ent]);
			}
			printf("};\n");
		}
	}

//	printf("struct %sBucket { int EntryCount; %s *Entries; };\n", TableName, EntryStruct);
//	printf("#define %sSize %d\n", TableName, tableSize);
	printf("#define EMPTY {0, 0}\n");

	printf("const struct %sBucket %s[%d]= {\n", TableName, TableName, tableSize);
	for (i=0; i<tableSize; i++) {
		if (!(i&0x07)) printf("\t");
		if (table[i].count)
			printf("{%d, %s%d}", table[i].count, BucketName, i);
		else
			printf("EMPTY");
		if (i != tableSize-1)
			printf(", ");
		if ((i&0x07) == 7)
			printf("\n");
	}
	printf("};\n");
	printf("#undef EMPTY\n");
	return 0;
}

#include <libraries/dos.h>
#include "tree.h"

struct FileHandler* filehandler;
int bytes_written;
int loop;
int arraysize;
UWORD* gpointer;
char fp[5];
char tobewritten[4];
UWORD result;


void main() {
	printf("Size of UWORD: %d\n", sizeof(UWORD));
	// 52 rows, 58 colums. 52*58 = 3016 bits
	// 377 bytes no padding
	// 52 rows, 64 colums with padding. = 3328 bits = 416 bytes

	filehandler = Open("tree.dat", MODE_NEWFILE);
	if (filehandler == NULL)
		printf("Could not open the file!\n");
	else {
		sprintf( tobewritten, "%d", tree.Width);
		bytes_written = Write( filehandler, tobewritten, sizeof(char)*2);
		printf("%d bytes written.\n", bytes_written);

		sprintf( tobewritten, "%d", tree.Height);
		bytes_written = Write( filehandler, tobewritten, sizeof(char)*2);
		printf("%d bytes written.\n", bytes_written);
		
		sprintf( tobewritten, "%d", tree.Depth);
		bytes_written = Write( filehandler, tobewritten, sizeof(char));
		printf("%d bytes written.\n", bytes_written);

		sprintf( tobewritten, "%d", sizeof(treeData)/sizeof(UWORD));
		bytes_written = Write( filehandler, tobewritten, sizeof(char)*3);
		printf("%d bytes written.\n", bytes_written);

		gpointer = treeData;
		for(loop = 0; loop < (sizeof(treeData)/sizeof(UWORD)); loop++)
		{
			sprintf(tobewritten, "%04X", *gpointer);
			bytes_written = Write(filehandler, tobewritten, sizeof(char)*4);
			gpointer++;
		}
		printf("A lot of bytes written. Size of tobewritten: %d\n", sizeof(tobewritten));

		printf("Time to read them back out.\n");
		Seek(filehandler, 0, OFFSET_BEGINNING);

		bytes_written = Read(filehandler, fp, sizeof(char)*2);
		result = strtol( fp, NULL, 10);
		printf("Read %d bytes. Width: %d\n", bytes_written, result);

		bytes_written = Read(filehandler, fp, sizeof(char)*2);
		result = strtol( fp, NULL, 10);
		printf("Read %d bytes. Width: %d\n", bytes_written, result);

		bytes_written = Read(filehandler, fp, sizeof(char));
		result = strtol( fp, NULL, 10);
		printf("Read %d bytes. Depth: %d\n", bytes_written, result);

		bytes_written = Read(filehandler, fp, sizeof(char)*3);
		result = strtol( fp, NULL, 10);
		printf("Read %d bytes. Array Size: %d\n", bytes_written, result);

		arraysize = result;

		for(loop = 0; loop < arraysize; loop++) {
			bytes_written = Read(filehandler, fp, sizeof(char)*4);
			result = strtol( fp, NULL, 16);
			printf("%04X,", result);
		}
		printf("\n");

		Close(filehandler);
	}
}

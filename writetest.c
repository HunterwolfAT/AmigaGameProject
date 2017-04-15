#include <libraries/dos.h>

struct FileHandler* filehandler;
char string[2];
char* imagedata;
long bytes_written;
int width, height = 0;
int loop;

void main() {
	/*
	filehandler = Open("test.txt", MODE_NEWFILE);
	bytes_written = Write(filehandler, string, sizeof(string));
	printf("Hello! %d bytes written!\n", bytes_written);
	Seek(filehandler, 0, OFFSET_BEGINNING);
	bytes_written = Read(filehandler, newstring, sizeof(newstring));
	printf("Goodbye! %d bytes read. Content: %s\n", bytes_written, newstring);
	Close(filehandler);
	*/
	filehandler = Open("tree.pbm", MODE_OLDFILE);
	if (filehandler == NULL)
		printf("Could not open the file!\n");
	else {
		bytes_written = Read(filehandler, string, sizeof(char)*2);
		printf("%d Bytes read. Content: %s\n", bytes_written, string);
		if(strcmp(string, "P6") == 0) {
			printf("This is a PBM image file!\n");
			Seek( filehandler, 1, OFFSET_CURRENT ); //skip the whitespace
			bytes_written = Read(filehandler, string, sizeof(char)*2);
			width = strtol(string, NULL, 10);
			Seek( filehandler, 1, OFFSET_CURRENT ); //skip the whitespace
			bytes_written = Read(filehandler, string, sizeof(char)*2);
			height = strtol(string, NULL, 10);
			printf("The image is %d pixel wide and %d pixel tall.\n", width, height);
			Seek( filehandler, 5, OFFSET_CURRENT); //skip whitespaces and brightness value
			// Read image content
			// allocate image data memory
			imagedata = malloc(sizeof(char)*width*height);
			if (imagedata == NULL)
				printf( "Could not allocate memory for imagedata!" );
			else {
				bytes_written = Read(filehandler, imagedata, sizeof(char)*width*height);
				printf("%d of %d bytes read.\n", bytes_written, sizeof(char)*width*height);
				for (loop = 0; loop < 25; loop++)
					printf("Content #%d: %d\n", loop, imagedata[loop]);
				free(imagedata);
			}
		}
		else
			printf("I don't know what file this is.\n");
		Close(filehandler);
	}
	//result = strtol(newstring, NULL, 10);
	//printf("And as an int: %d\n", result);
}

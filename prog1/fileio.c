//////////////////////////////////////////////////////////////////////
// Intentionally flawed system call library that implements          //
// (unfortunately, not) "safe" file I/O, "preventing" writing "MZ"   //
// at the beginning of a file.                                       //
//                                                                   //
// Written by Golden G. Richard III (@nolaforensix), 7/2017          //
//                                                                   //
// Props to Brian Hay for a similar exercise he used in a recent     //
// training.                                                        //
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"

//
// GLOBALS
//

FSError fserror;

//
// private functions
//

static int seek_file(File file, SeekAnchor start, long offset) {
    if (! file->fp || (start != BEGINNING_OF_FILE &&
                   start != CURRENT_POSITION && start != END_OF_FILE)) {
        return 0;
    }
    else {
        if (! fseek(file->fp, offset, start == BEGINNING_OF_FILE ? SEEK_SET :
                    (start == END_OF_FILE ? SEEK_END : SEEK_CUR))) {
            return 1;
        }
        else {
            return 0;
        }
    }
}

//
// public functions
//

// open or create a file with pathname 'name' and return a File
// handle.  The file is always opened with read/write access. If the
// open operation fails, the global 'fserror' is set to OPEN_FAILED,
// otherwise to NONE.
File open_file(char *name) {
    File f = malloc(sizeof(File));
    fserror=NONE;
    // try to open existing file
    f->fp=fopen(name, "r+");
    if (! f->fp) {
        // fail, fall back to creation
        f->fp=fopen(name, "w+");
        if (! f->fp) {
            fserror=OPEN_FAILED;
            return NULL;
        }
    }
    read_file_from(f, f->firstTwoBytes, 2, BEGINNING_OF_FILE, 0);
    return f;
}

// close a 'file'.  If the close operation fails, the global 'fserror'
// is set to CLOSE_FAILED, otherwise to NONE.
void close_file(File file) {
    if (file->fp && ! fclose(file->fp)) {
        fserror=NONE;
    }
    else {
        fserror=CLOSE_FAILED;
    }
	free(file);
}

// read at most 'num_bytes' bytes from 'file' into the buffer 'data',
// starting 'offset' bytes from the 'start' position.  The starting
// position is BEGINNING_OF_FILE, CURRENT_POSITION, or END_OF_FILE. If
// the read operation fails, the global 'fserror' is set to READ_FAILED,
// otherwise to NONE.
unsigned long read_file_from(File file, void *data, unsigned long num_bytes,
                             SeekAnchor start, long offset) {
    
    unsigned long bytes_read=0L;
    
    fserror=NONE;
    if (! file->fp || ! seek_file(file, start, offset)) {
        fserror=READ_FAILED;
    }
    else {
        bytes_read=fread(data, 1, num_bytes, file->fp);
        if (ferror(file->fp)) {
            fserror=READ_FAILED;
        }
    }
    return bytes_read;
}

// write 'num_bytes' to 'file' from the buffer 'data', starting
// 'offset' bytes from the 'start' position.  The starting position is
// BEGINNING_OF_FILE, CURRENT_POSITION, or END_OF_FILE.  If an attempt
// is made to modify a file such that "MZ" appears in the first two
// bytes of the file, the write operation fails and ILLEGAL_MZ is
// stored in the global 'fserror'.  If the write fails for any other
// reason, the global 'fserror' is set to WRITE_FAILED, otherwise to
// NONE.
unsigned long write_file_at(File file, void *data, unsigned long num_bytes,
                            SeekAnchor start, long offset) {
    unsigned long bytes_written=0L;
    int legalWrite = 1; 
    fserror=NONE;
    if (! file->fp || ! seek_file(file, start, offset)) 
	{
        fserror=WRITE_FAILED;
		legalWrite = 0;
    }
	if( num_bytes > 0)
	{
		long position = ftell(file->fp);
		
    	 if (position == 0L) 
		{
			//catch someone writing 'MZ' at beginning of file
			if(num_bytes > 1 && !strncmp(data, "MZ",2))
			{
        		fserror=ILLEGAL_MZ;
				legalWrite = 0;
			}
			//catch someone writing 'M' at beginning of file when the second byte is 'Z'
			else if(num_bytes == 1 && ((char*)data)[0] == 'M' && file->firstTwoBytes[1] == 'Z')
			{
				fserror = ILLEGAL_MZ;
				legalWrite = 0;
			}
			//otherwise update first two bytes
			else
			{
				if(num_bytes == 1)
				{
					file->firstTwoBytes[0] = ((char*) data)[0];
				}
				else
				{
					strncpy(file->firstTwoBytes, (char*)data, 2*sizeof(char));
				}

			}
		
    	}
		else if(position == 1L)
		{
			//catch someone writing 'Z' to secondbyte while first byte is 'M'
			if( ((char*)data)[0] == 'Z' && file->firstTwoBytes[0] == 'M')
			{
				fserror = ILLEGAL_MZ;
				legalWrite = 0;
			}
			//otherwise update  firstTwoBytes
			else 
			{	
					file->firstTwoBytes[1] = ((char*)data)[0];
			}
		}
    	if(legalWrite)
		{
       		bytes_written=fwrite(data, 1, num_bytes, file->fp);
        	if (bytes_written < num_bytes) 
			{
            	fserror=WRITE_FAILED;
        	}
    	}
	}
    return bytes_written;
}


// print a string representation of the error indicated by the global
// 'fserror'.
void fs_print_error(void) {
    printf("FS ERROR: ");
    switch (fserror) {
        case NONE:
            puts("NONE");
            break;
        case OPEN_FAILED:
            puts("OPEN_FAILED");
            break;
        case CLOSE_FAILED:
            puts("CLOSE_FAILED");
            break;
        case READ_FAILED:
            puts("READ_FAILED");
            break;
        case WRITE_FAILED:
            puts("WRITE_FAILED");
            break;
        case ILLEGAL_MZ:
            puts("ILLEGAL_MZ: SHAME ON YOU!");
            break;
        default:
            puts("** UNKNOWN ERROR **");
    }
}

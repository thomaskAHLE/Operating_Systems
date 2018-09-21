#include<stdio.h>
#include"fileio.h"
void test1()
{
	printf("Opening file important.dat\n");
	File f;
	f = open_file("important.dat");
	fs_print_error();
	printf("Writing MZ to Beginning of file\n");
	write_file_at(f,"MZ",2,BEGINNING_OF_FILE, 0L);
	fs_print_error();
	printf("Writing MZ to End of file\n");
	write_file_at(f,"MZ",2,END_OF_FILE, 0L);
	fs_print_error();
	printf("Writing MZ to current position of file\n");
	write_file_at(f,"MZ",2,CURRENT_POSITION, 0L);
	fs_print_error();
	printf("Closing important.dat \n");
	close_file(f);
	fs_print_error();

}
void test2()
{
	printf("Opening file important.dat\n");
	File f;
	f = open_file("important.dat");
	fs_print_error();
	printf("Writing M to Beginning of file\n");
	write_file_at(f,"M",1,BEGINNING_OF_FILE, 0L);
	fs_print_error();
	printf("Writing Z to End of file\n");
	write_file_at(f,"Z",1,END_OF_FILE, 0L);
	fs_print_error();
	printf("Closing important.dat \n");
	close_file(f);
	fs_print_error();
}
void test3()
{
	printf("Opening file important2.dat\n");
	File f ;
	f = open_file("important2.dat");
	fs_print_error();
	printf("Writing M to Beginning of file important2\n");
	write_file_at(f,"M",1,BEGINNING_OF_FILE, 0L);
	fs_print_error();
	printf("Opening file important3.dat\n");
	File f1;
	f1 = open_file("important3.dat");
	fs_print_error();
	printf("Writing Z to Current Posiiton important3\n");
	write_file_at(f1,"Z",1,CURRENT_POSITION, 1L);
	fs_print_error();
	printf("Writing Z to END of file important2\n");
	write_file_at(f,"Z",1,END_OF_FILE, 0L);
	fs_print_error();
	printf("Writing M to Beginning of file important3\n");
	write_file_at(f1,"M",1,BEGINNING_OF_FILE, 0L);
	fs_print_error();
		
	printf("Closing important2.dat \n");
	close_file(f);
	fs_print_error();
	printf("Closing important3.dat \n");
	close_file(f1);
	fs_print_error();


}
void test4()
{
	printf("Opening file important2.dat\n");
	File f ;
	f = open_file("important2.dat");
	fs_print_error();
	printf("Writing M to Beginning of file important2\n");
	write_file_at(f,"M",1,BEGINNING_OF_FILE, 0L);
	fs_print_error();
	printf("Closing important2.dat \n");
	close_file(f);
	fs_print_error();
	printf("Opening file important2.dat\n");
	f = open_file("important2.dat");
	fs_print_error();
	printf("Writing Z to Beginning of file important2 offset by 1\n");
	write_file_at(f,"Z",1,BEGINNING_OF_FILE, 1L);
	fs_print_error();
	printf("Closing important2.dat \n");
	close_file(f);
	fs_print_error();
	
}
void test5()
{
	printf("Opening file important.dat\n");
	File f;
	f = open_file("important.dat");
	fs_print_error();
	printf("Writing 01234 to Beginning of file\n");
	write_file_at(f,"01234",5,BEGINNING_OF_FILE, 0L);
	fs_print_error();
	printf("Writing MZ to Current posiiton -5\n");
	write_file_at(f,"MZ",2,CURRENT_POSITION, -5L);
	fs_print_error();
	printf("Writing MZ to end of file -5\n");
	write_file_at(f,"MZ",2,END_OF_FILE, -5L);
	fs_print_error();
	printf("Writing M to end of file -5\n");
	write_file_at(f,"M",1,END_OF_FILE, -5L);
	fs_print_error();
	printf("Writing Z to end of file -4\n");
	write_file_at(f,"Z",1,END_OF_FILE, -4L);
	fs_print_error();
	close_file(f);
}
int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
}

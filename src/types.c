#include <config.h>


#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <dirent.h>
#include <sys/stat.h>

char *c_strTITLE = PACKAGE_STRING" SecretCode Edition";
char *c_strCORP ="Mad House Software (C)2004";

coloredtext_t c_about[] = {
		{{COLOR_7 } , "Programming:"},
		{{COLOR_15} , " Remnyov Alexander aka Master San[MH]"},
		{{COLOR_7 } , "Artists:"},
		{{COLOR_15} , " Remnyov Alexander,"},
		{{COLOR_15} , " Vasilyev Artyom aka Timon"},
		{{COLOR_7 } , "Designers:"},
		{{COLOR_15} , " Remnyov Alexander, Vasilyev Artyom"},
		{{COLOR_7 } , "Levels designers:"},
		{{COLOR_15} , " Remnyov Alexander, Vasilyev Artyom"},
		{{COLOR_7 } , "Special thanks:"},
		{{COLOR_15} , " Vasilyev Artyom"},
		{{COLOR_1 } , ""},
		{{COLOR_13} , "Данная редакция, в отличии от"},
		{{COLOR_13} , "обычной версии, содержит секретные"},
		{{COLOR_13} , "коды, добавленые по многочисленным"},
		{{COLOR_13} , "просьбам."},
		{}
};

bool debug_noAI = true;


/**
 * @description проверка допустимости ввода символа (добавление 10.05.2006)
 * @param[in] chr - символ
 * @return
 * @return =true  - символ разрешен
 * @return =false - символ не разрешен
 */
bool checkchar(char chr)
{
	long c = 0;
	bool allow = false;
	while(chars_allowed[c] && !allow)
	{
		allow = (chars_allowed[c] == chr);
		c++;
	}
	return allow;
};

float sqrf(float v)
{
	return v*v;
}

void randomize()
{
	srandom(1024);
}

int xrand(int hi)
{
	return random() % hi;
}

struct errno_ent_t
{
	int value;
	char * name;
} errno_list[] = {
	{EPERM   , "EPERM"},	/* Operation not permitted */
	{ENOENT  , "ENOENT"},	/* No such file or directory */
	{ESRCH   , "ESRCH"},	/* No such process */
	{EINTR   , "EINTR"},	/* Interrupted system call */
	{EIO     , "EIO"},	/* I/O error */
	{ENXIO   , "ENXIO"},	/* No such device or address */
	{E2BIG   , "E2BIG"},	/* Argument list too long */
	{ENOEXEC , "ENOEXEC"},	/* Exec format error */
	{EBADF   , "EBADF"},	/* Bad file number */
	{ECHILD  , "ECHILD"},	/* No child processes */
	{EAGAIN  , "EAGAIN"},	/* Try again */
	{ENOMEM  , "ENOMEM"},	/* Out of memory */
	{EACCES  , "EACCES"},	/* Permission denied */
	{EFAULT  , "EFAULT"},	/* Bad address */
	{ENOTBLK , "ENOTBLK"},	/* Block device required */
	{EBUSY   , "EBUSY"},	/* Device or resource busy */
	{EEXIST  , "EEXIST"},	/* File exists */
	{EXDEV   , "EXDEV"},	/* Cross-device link */
	{ENODEV  , "ENODEV"},	/* No such device */
	{ENOTDIR , "ENOTDIR"},	/* Not a directory */
	{EISDIR  , "EISDIR"},	/* Is a directory */
	{EINVAL  , "EINVAL"},	/* Invalid argument */
	{ENFILE  , "ENFILE"},	/* File table overflow */
	{EMFILE  , "EMFILE"},	/* Too many open files */
	{ENOTTY  , "ENOTTY"},	/* Not a typewriter */
	{ETXTBSY , "ETXTBSY"},	/* Text file busy */
	{EFBIG   , "EFBIG"},	/* File too large */
	{ENOSPC  , "ENOSPC"},	/* No space left on device */
	{ESPIPE  , "ESPIPE"},	/* Illegal seek */
	{EROFS   , "EROFS"},	/* Read-only file system */
	{EMLINK  , "EMLINK"},	/* Too many links */
	{EPIPE   , "EPIPE"},	/* Broken pipe */
	{EDOM    , "EDOM"},	/* Math argument out of domain of func */
	{ERANGE  , "ERANGE"},	/* Math result not representable */
};



/*
 * добавление символа в конец строки
 */
char * str_addch(char * s0, char ch)
{
	int count = 0;
	while(s0[count]) count++;
	s0[count] = ch;
	count++;
	s0[count] = 0;
	return s0;
}

void check_directory(const char * path)
{
	DIR * dir = opendir(path);
	if(!dir) mkdir(path, 0755);
	else closedir(dir);
}


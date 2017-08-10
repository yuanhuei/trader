#ifndef DELETERECORD_H
#define DELETERECORD_H
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <io.h>  
#include <direct.h>  
#include <errno.h>  

void deletedir(char * path)
{
	_finddata_t fileDir;
	char* dir = path;
	long lfDir;

	if ((lfDir = _findfirst(dir, &fileDir)) == -1l)
		printf("No file is found\n");
	else{
		printf("file list:\n");
		do{
			std::string name = "./traderecord/" + std::string(fileDir.name);
			int result=remove(name.c_str());
		} while (_findnext(lfDir, &fileDir) == 0);
	}
	_findclose(lfDir);
}


#endif
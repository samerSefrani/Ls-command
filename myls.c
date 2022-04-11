#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>



//Global vars
int numOfDirs = 0;
int numOfOptions = 0;
const int files_Names_max = 1000;
const int num_of_options_max = 10;
char permissions[15];
bool isDirEmpty = false;


//Flags
bool i_flag = false;
bool l_flag = false;
bool r_flag = false;



int isDirectory(const char *path) {
    //Refrenced: https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

int isFile(const char *path){
    //Refrenced: https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}


void generatePermissionsFromMode(int st_mode, char tempDirPath[1000]){
    //Refrenced: https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
    
    if(!isDirectory(tempDirPath)) strcat(permissions, "-" );  //only print - for files
    (S_ISDIR(st_mode)) ?  strcat(permissions, "d" ) : NULL;
    (st_mode & S_IRUSR) ? strcat(permissions, "r" ) : NULL;
    (st_mode & S_IWUSR) ? strcat(permissions, "w" ) : NULL;
    (st_mode & S_IXUSR) ? strcat(permissions, "x" ) : NULL;
    (st_mode & S_IRGRP) ? strcat(permissions, "r" ) : NULL;
    (st_mode & S_IWGRP) ? strcat(permissions, "w" ) : NULL;
    (st_mode & S_IXGRP) ? strcat(permissions, "x" ) : NULL;
    (st_mode & S_IROTH) ? strcat(permissions, "r" ) : NULL;
    (st_mode & S_IWOTH) ? strcat(permissions, "w" ) : NULL;
    (st_mode & S_IXOTH) ? strcat(permissions, "x" ) : NULL;
    return;
}


//print content for a specific directory
void printWith_i_l(char *options[num_of_options_max], char * dirPath){

	struct dirent **filesNames;
	int numberOfFiles = scandir(dirPath, &filesNames, NULL, alphasort);
    if(numberOfFiles < 0) {
		printf("Dir not opened: %s", dirPath);
		return;
	}

    int i = 0;
    char tempDirPath[1000]; 

    while(i < numberOfFiles){

        if(!strncmp(filesNames[i]->d_name, ".", 1)) {i++; continue;}
     
        strcpy(tempDirPath, " ");   //remove previous content from tempDirPath
        strcpy(tempDirPath, dirPath);

        if(strncmp(tempDirPath+(strlen(tempDirPath) - 1), "/", 1) != 0){ //if directory doesnt have "/" in the end -> append one
            strcat(tempDirPath, "/");
        }

        strcat(tempDirPath, filesNames[i]->d_name);

        FILE * file;
        struct stat buffer;
        
        file = fopen(tempDirPath, "r");
        //Refrenced: https://stackoverflow.com/questions/5929419/how-to-get-file-creation-date-in-linux
        long fd = fileno(file);
        fstat(fd, &buffer);

        long fileSize = buffer.st_size;
        int fileLinks = buffer.st_nlink;
        char *userName = "default";
        char *groupName = "default";

        struct passwd *pw = getpwuid(buffer.st_uid);
		if(pw != NULL){
            userName = pw -> pw_name;
		}

		struct group *grp = getgrgid(buffer.st_gid);
		if(grp != NULL){
            groupName = grp -> gr_name;
		}

        strcpy(permissions, "");   //remove previuos permissions
        generatePermissionsFromMode(buffer.st_mode, tempDirPath);

        //Refrenced: https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
        char fileDate[15];
        strftime(fileDate, 20, "%h %d %Y %H:%M", localtime(&(buffer.st_mtime)));

        if(i_flag) printf("%ld ", buffer.st_ino);
        if(i_flag && !l_flag && isDirEmpty) printf("%s", filesNames[i]->d_name);
        if(i_flag && !l_flag) printf("%s\n", filesNames[i]->d_name);
        if(l_flag) printf("%s %d %s %s %ld %s %s\n", permissions, fileLinks, userName, groupName, fileSize, fileDate, filesNames[i]->d_name);
        if(r_flag && !l_flag && !i_flag) printf("%s\n", filesNames[i]->d_name);
        
        fclose(file);
        i++;
    }
}


void checkForOptionsFlags(char *options[num_of_options_max]){

    for(int i = 0; i < numOfOptions; i++){

        if(strstr(options[i], "i")){
            i_flag = true;
        }
        if(strstr(options[i], "l")){
            l_flag = true;
        }
        if(strstr(options[i], "R")){
            r_flag = true;
        }
        if(!strstr(options[i], "i") && !strstr(options[i], "l") && !strstr(options[i], "R")){
            printf("Error: Unsupported Option\n");
            exit(1);
        }
        if(strstr(options[i], "--")){
        		printf("Error: Unsupported Option\n");
        		i_flag = false;
        		l_flag = false;
        		r_flag = false;
        }
    }
}

void printRecursively(char* path, char *options[num_of_options_max]){
    //Refrenced: https://stackoverflow.com/questions/26552503/print-directories-in-ascending-order-using-scandir-in-c
	struct dirent** tempFiles;
	int numberOfFiles;
	int i = 0;

	int res = isFile(path);
	if(res == -1 ) return;

	if((numberOfFiles = scandir(path, &tempFiles, NULL, alphasort)) == -1) return;

	printf("%s:\n",path);
    printWith_i_l(options, path);
	printf("\n");
	
	while(i < numberOfFiles){

		if(!strncmp(tempFiles[i]->d_name, ".", 1)){i++; continue;}

		char TempPath[strlen(path) + strlen(tempFiles[i]->d_name)];

        if(strncmp(path+(strlen(path) - 1), "/", 1) != 0){
            strcpy(TempPath,path);
			strcat(TempPath,"/");
        }else{
			strcpy(TempPath,path);
		}
        strcat(TempPath, tempFiles[i]->d_name);
		printRecursively( TempPath, options);
		i++;
	}
}


void startProgram(char *options[num_of_options_max], char **dirs){

    if(numOfDirs == 0){
        numOfDirs ++;
        isDirEmpty = !isDirEmpty;
    }

    for(int i = 0; i < numOfDirs ; i++){
        
        DIR *dir = opendir(isDirEmpty ? "." : dirs[i]);
        if(dir == NULL) {
            printf("Directory '%s' not found\n", dirs[i]);
            continue;
        }

        struct dirent *dirEntry;
        errno = 0;

        if(options){
            checkForOptionsFlags(options);
        }

        if(r_flag) {
            printRecursively(isDirEmpty ? "." : dirs[i], options);
            continue;    
        };


        if(numOfDirs > 1 && i > 0) printf("\n");
        //if(dirs) printf("%s\n", dirs[i]);

        while((dirEntry = readdir(dir))!= NULL) {
            
            if(!strncmp(dirEntry->d_name, ".", 1)) continue;    //dont print hidden files
            if(!options) printf("%s\n", dirEntry->d_name);
        }

        if(options) printWith_i_l(options, isDirEmpty ? "." : dirs[i]);

        if((dirEntry == NULL) && (errno != 0)) {
            perror("readdir");
            exit(EXIT_FAILURE);
        }

        closedir(dir);
    }

}


int main(int argc, char **argv){

    if(argc == 1){  //No option, no directory
        startProgram(NULL, NULL);
    }else{

        char *options[num_of_options_max];
        
        if(strncmp(argv[1], "-", 1) == 0){  //if the command start with -
            for(int i = 1; i < argc; i++){
                if(strncmp(argv[i], "-", 1) == 0){
                    options[numOfOptions++] = argv[i];
                }else{
                    numOfDirs++;
                }
            }
            char **dirsArr = argv + 1 + numOfOptions;
            startProgram(options, dirsArr);

        }else{

            if(isFile(argv[1]) && argc == 2){   //it has only one file
                printf("%s\n", argv[1]);
                return 0;
            }

            //has at least one dir. Filter the dir array
            char **dirsArr = argv + 1;
            numOfDirs = --argc; //dont count ./myls arg

            startProgram(NULL, dirsArr);
        }
    }

    return 0;
}
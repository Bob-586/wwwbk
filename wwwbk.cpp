/*
 * Author: Robert Strutts
 * Enjoy
 * 
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
 * 
 * 
 * Compilation example for GCC (v8 and up)
 * g++ -std=c++17 -Wall -Wextra -pedantic wwwbk.cpp -o wwwbk
*/

#include <iostream>
#include <string>         // std::string
#include <ctime>
#include <sys/types.h> // mkdir
#include <sys/stat.h> // mkdir
#include "FileWatcher.h"
#include <fstream> // copy_file

std::string web_folder = "/var/www"; // Do NOT end with a slash!
std::string web_backup_folder = "/var/www_backups"; // Do NOT end with a slash!

void copy_file ( const char* srce_file, const char* dest_file ) {
    std::ifstream srce( srce_file, std::ios::binary ) ;
    std::ofstream dest( dest_file, std::ios::binary ) ;
    dest << srce.rdbuf() ;
}

int make_new_dir_path ( std::string s, mode_t mode ) {
    size_t pos=0;
    std::string dir;
    int mdret;

    if(s[s.size()-1]!='/') {
        // force trailing / so we can handle everything in loop
        s+='/';
    }

    while (( pos=s.find_first_of('/',pos)) != std::string::npos ) {
        dir = s.substr(0, pos++);
        if (dir.size() == 0) continue; // if leading / first time is 0 length
        if ((mdret = mkdir(dir.c_str(),mode)) && errno != EEXIST) {
            return mdret;
        }
    }
    return mdret;
}

/*
 * Erase First Occurrence of given substring from main string.
 */
void erase_sub_string ( std::string &mainStr, const std::string &toErase ) {
    // Search for the substring in string
    size_t pos = mainStr.find(toErase);
    if (pos != std::string::npos) {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
}

void get_date_and_time ( char* date_time_str ) {
    time_t curr_time;
    tm * curr_tm;
	
    time(&curr_time);
    curr_tm = localtime(&curr_time);
	
    strftime(date_time_str, 50, "%m-%d-%Y %T", curr_tm);
}

bool found_script ( std::string path ) {
	
    std::string swap (".swp"); // Temp Swap file
    std::size_t found_swap = path.find(swap);
	
    std::string backup ("~"); // Backup file
    std::size_t found_backup = path.find(backup);
	
    std::string php (".php"); // Backup PHP Files
    std::size_t found_php = path.find(php);

    std::string js (".js"); // Backup JavaScript Files
    std::size_t found_js = path.find(js);

    std::string ts (".ts"); // Backup TypeScript Files
    std::size_t found_ts = path.find(ts);
    
    if ( found_swap != std::string::npos || found_backup != std::string::npos ) {
	return false; // Skip Swap/Locks and Backup Files
    }

    if ( found_php != std::string::npos || found_js != std::string::npos || found_ts != std::string::npos ) {
	return true;
    } else {
	return false;
    }	
	
}

std::string get_save_folder ( std::string org_path ) {
    char date_time_string[100];
    get_date_and_time(date_time_string);    

    std::string base_filename = org_path.substr(org_path.find_last_of("/") + 1); // Get FileName from path
    
    erase_sub_string(org_path, web_folder + "/"); // Remove ROOT folder from path
    
    erase_sub_string(org_path, base_filename); // Remove FileName from path
    
    std::string temp_save_path = org_path.substr(0, org_path.find_last_of("/"));
    std::size_t length_of_save_path = temp_save_path.length();
	
    std::string save_folder = "";
	
    if (length_of_save_path == 0) {
	save_folder = web_backup_folder + "/" + date_time_string + base_filename;
    } else {
	std::string my_folder = web_backup_folder + "/" + temp_save_path;
	const char *new_folder = my_folder.c_str();
		
	make_new_dir_path(new_folder, 0775); // Make new Folder
	
	save_folder = web_backup_folder + "/" + temp_save_path + "/" + date_time_string + base_filename;
    }
    return save_folder;
}	

int main() {
		
    std::cout << "Watching for changes on www \n";
	
    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
    FileWatcher fw { web_folder, std::chrono::milliseconds(5000) };

    // Start monitoring a folder for changes and (in case of changes)
    // run a user provided lambda function
    fw.start( [] (std::string path_watch, FileStatus status ) -> void {
        // Process only regular files, all other file types are ignored
        if(!std::filesystem::is_regular_file(std::filesystem::path(path_watch)) && status != FileStatus::erased) {
            return;
        }
            
        std::string web_save_path = get_save_folder(path_watch);
        std::string str_cmd = "cp '" + path_watch + "' '" +  web_save_path + "'"; 

        // Convert string to const char * as system requires, parameter of type const char * 
        // const char *command = str_cmd.c_str();     

            switch(status) {
                case FileStatus::created:
                    if ( found_script(path_watch) ) {	
                        std::cout << "File created: " << path_watch << '\n';
                        std::cout << str_cmd << "\n";
                        // system(command);
                        copy_file( path_watch.c_str(), web_save_path.c_str() );
                    }
                    break;
                case FileStatus::modified:
                    if ( found_script(path_watch) ) {	
                        std::cout << "File modified: " << path_watch << '\n';
                        std::cout << str_cmd << "\n";
                        // system(command);
                        copy_file( path_watch.c_str(), web_save_path.c_str() );
                    }
                    break;
                case FileStatus::erased:
                    //std::cout << "File erased: " << path_watch << '\n';
                    break;
                default:
                    //std::cout << "Error! Unknown file status.\n";
                    break;
            }
    }); // end of lambda function
    
    return 0; // Success to OS
}

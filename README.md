# wwwbk for Linux
Backup the /var/www folders (all JavaScript, TypeScript, and PHP code) into /var/www_backups folders...

Run as regular user, not as root!

### Download:
$ git clone https://github.com/Bob-586/wwwbk.git ~/wwwbk

## Goto folder of program :
$ cd ~/wwwbk
## Then Compile the program :
$ chmod u+x compile_me.sh

$ ./compile_me.sh

### Create the Backup Folder
$ sudo mkdir /var/www_backups

$ sudo chown $USER:www-data /var/www_backups

$ sudo chmod 775 /var/www_backups

## To RUN :
$ wwwbk

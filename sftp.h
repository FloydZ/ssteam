#ifndef SFTP_H_INCLUDED
#define SFTP_H_INCLUDED

extern int verify_knownhost();
extern int sftp_list_dir_console(char*);
extern int sftp_list_dir(char*, char*[]);
extern int sftp_helloworld();
extern int ssh_start(char*, char*, char*, char*, int mode);
extern int ssh_close();
#endif // SFTP_H_INCLUDED

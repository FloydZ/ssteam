#include <errno.h>
#include <string.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdlib.h>
#include <stdio.h>

#include "sftp.h"

sftp_session sftp;
ssh_session session;

int verify_knownhost(){
    int state, hlen;
    unsigned char *hash = NULL;
    char *hexa;
    char buf[10];
    state = ssh_is_server_known(session);
    hlen = 0;//ssh_get_pubkey_hash(session, &hash);
    if (hlen < 0)
        return -1;
    switch (state)
    {
    case SSH_SERVER_KNOWN_OK:
        break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
        fprintf(stderr, "Host key for server changed: it is now:\n");
        ssh_print_hexa("Public key hash", hash, hlen);
        fprintf(stderr, "For security reasons, connection will be stopped\n");
        free(hash);
        return -1;
    case SSH_SERVER_FOUND_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other"
                "type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to"
                "confuse your client into thinking the key does not exist\n");
        free(hash);
        return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
        fprintf(stderr, "Could not find known host file.\n");
        fprintf(stderr, "If you accept the host key here, the file will be"
                "automatically created.\n");
    /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:
        hexa = ssh_get_hexa(hash, hlen);
        fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
        fprintf(stderr, "Public key hash: %s\n", hexa);
        free(hexa);

        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            free(hash);
            return -1;
        }
        if (strncasecmp(buf, "yes", 3) != 0)
        {
            free(hash);
            return -1;
        }
        if (ssh_write_knownhost(session) < 0)
        {
            fprintf(stderr, "Error %s\n", strerror(errno));
            free(hash);
            return -1;
        }

        break;
    case SSH_SERVER_ERROR:
        fprintf(stderr, "Error %s", ssh_get_error(session));
        free(hash);
        return -1;
    }
    free(hash);
    return 0;
}

int sftp_list_dir_consolse(char* direction){
    sftp_dir dir;
    sftp_attributes attributes;
    int rc;
    dir = sftp_opendir(sftp, direction);
    if (!dir)
    {
        fprintf(stderr, "Directory not opened: %s\n", ssh_get_error(session));
        return SSH_ERROR;
    }
    printf("Name Size Perms Owner\tGroup\n");
    while ((attributes = sftp_readdir(sftp, dir)) != NULL)
    {
        printf("%-20s %10llu %.8o %s(%d)\t%s(%d)\n",
               attributes->name,
               (long long unsigned int) attributes->size,
               attributes->permissions,
               attributes->owner,
               attributes->uid,
               attributes->group,
               attributes->gid);
        sftp_attributes_free(attributes);
    }
    if (!sftp_dir_eof(dir))
    {
        fprintf(stderr, "Can't list directory: %s\n",
                ssh_get_error(session));
        sftp_closedir(dir);
        return SSH_ERROR;
    }
    rc = sftp_closedir(dir);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Can't close directory: %s\n",
                ssh_get_error(session));
        return rc;
    }
    return -1;
}

int sftp_list_dir(char* direction, char *str[]){
    sftp_dir dir;
    sftp_attributes attributes;
    int rc;
    dir = sftp_opendir(sftp, direction);

    if (!dir){
        fprintf(stderr, "Directory not opened: %s\n", ssh_get_error(session));
        return SSH_ERROR;
    }

    int i = 0;
    while ((attributes = sftp_readdir(sftp, dir)) != NULL){
        str[i] = malloc(strlen(attributes->name) + 1);
        strcpy(str[i], attributes->name);
        sftp_attributes_free(attributes);
        i++;
    }
    str[i] = NULL;

    if (!sftp_dir_eof(dir)){
        fprintf(stderr, "Can't list directory: %s\n",
                ssh_get_error(session));
        sftp_closedir(dir);
        return SSH_ERROR;
    }

    rc = sftp_closedir(dir);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Can't close directory: %s\n",
                ssh_get_error(session));
        return rc;
    }
    return -1;
}

int sftp_helloworld(){
    int rc;

    sftp = sftp_new(session);
    if (sftp == NULL)
    {
        fprintf(stderr, "Error allocating SFTP session: %s\n",
                ssh_get_error(session));
        return SSH_ERROR;
    }

    rc = sftp_init(sftp);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error initializing SFTP session: %d.\n", sftp_get_error(sftp));
        sftp_free(sftp);
        return rc;
    }
    return SSH_OK;
}

int authenticate_password(){
    char *password;
    int rc;
    password = getpass("Enter your password: ");
    rc = ssh_userauth_password(session, NULL, password);
    if (rc == SSH_AUTH_ERROR)
    {
        fprintf(stderr, "Authentication failed: %s\n",
        ssh_get_error(session));
        return SSH_AUTH_ERROR;
    }
    return rc;
}

int ssh_start(char* password, char* user, char* host, char* port, int mode){
    int rc;

    if (session != NULL){
        fprintf(stderr, "SSH connection already open");
        return SSH_ERROR;
    }
// Open session and set options
    session = ssh_new();
    if (session == NULL)
        exit(-1);

    ssh_options_set(session, SSH_OPTIONS_HOST, host);
    ssh_options_set(session, SSH_OPTIONS_PORT_STR, port);


// Connect to server
    rc = ssh_connect(session);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(session));
        ssh_free(session);
        exit(-1);
    }
// Verify the server's identity
// For the source code of verify_knowhost(), check previous example
    if (verify_knownhost(session) < 0)
    {
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    if (mode == 0){
        rc = ssh_userauth_password(session, user, password);
        if (rc != SSH_AUTH_SUCCESS){
            fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(session));
            ssh_disconnect(session);
            ssh_free(session);
            return SSH_AUTH_ERROR;
        }
    }
    else if (mode == 1){
         rc = ssh_userauth_publickey_auto(session, user, password);
        if (rc == SSH_AUTH_ERROR){
            fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
            ssh_disconnect(session);
            ssh_free(session);
            return SSH_AUTH_ERROR;
        }
    }

    sftp_helloworld();

    return SSH_OK;
}

int ssh_close(){
    sftp_free(sftp);
    ssh_disconnect(session);
    ssh_free(session);

    return 1;
}

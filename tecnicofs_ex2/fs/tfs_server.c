#include "operations.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define S (1)
#define MAXBUFFER (1024)

//packed
typedef struct {
    int status;
    char pipe_name[40];
} session_id;

session_id session_id_array[S];

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *srv_pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", srv_pipename);

    //remover fifos ou ficheiros preexistentes
    unlink(srv_pipename);

    int fclient = -1, fsrv; 

    if(mkfifo(srv_pipename,0777) < 0){
        //if(errno == EEXIST){
        printf("Couldn't create\n");
        return -1;
        //}
    }

    if((fsrv = open(srv_pipename,O_RDONLY)) < 0){
        return -1;
    }

    for(int i = 0; i < S; i++){
        session_id_array[i].status = 0;
    }
    char client_p_name[40];
    int actual_client_session;
    tfs_init();
    for(;;){
        char opcode;
        int flag_cond = -1;
        if(read(fsrv,&opcode,sizeof(char)) <= 0){
            return -1;
        }
        switch(opcode){
            case TFS_OP_CODE_MOUNT:
                if(read(fsrv,client_p_name,40*sizeof(char)) <= 0)
                    return -1;
                for(int i = 0; i < S; i++){
                    if(session_id_array[i].status == 1 && strcmp(session_id_array[i].pipe_name,client_p_name) == 0){
                        actual_client_session = i+1;
                        flag_cond = 1;
                        break;
                    }
                    if(session_id_array[i].status == 0){
                        session_id_array[i].status = 1;
                        strcpy(session_id_array[i].pipe_name,client_p_name);
                        actual_client_session = i+1;
                        flag_cond = 1;
                        break;
                    }
                }
                uint8_t mensagem1[sizeof(int)];
                if((fclient = open(session_id_array[actual_client_session-1].pipe_name,O_WRONLY)) < 0)
                    return -1;
                //se n for possivel usar esse cliente
                if(flag_cond == -1){
                    memcpy(mensagem1,&flag_cond,sizeof(char));
                    if(write(fclient,mensagem1,sizeof(int)) <= 0)
                        return -1;
                    if(close(fclient) < 0)
                        return -1;
                    if(unlink(client_p_name) < 0)
                        return -1;
                    break;
                }
                memcpy(mensagem1,&actual_client_session,sizeof(int));
                if(write(fclient,mensagem1,sizeof(int)) <= 0)
                        return -1;  
                break;
            case TFS_OP_CODE_UNMOUNT:

                if(read(fsrv,&actual_client_session,sizeof(int)) <= 0)
                    return -1;
                if(session_id_array[actual_client_session-1].status == 0){
                    uint8_t mensagem2[sizeof(int)];
                    memcpy(mensagem2,&flag_cond,sizeof(int));
                    if(write(fclient,mensagem2,sizeof(int)) <= 0)
                        return -1;
                    break;
                }
                uint8_t mensagem2[sizeof(int)];
                int res6 = 0;
                session_id_array[actual_client_session-1].status = 0;
                memset(session_id_array[actual_client_session-1].pipe_name,0,40);
                memcpy(mensagem2,&res6,sizeof(int));
                if(write(fclient,mensagem2,sizeof(int)) <= 0)
                        return -1;
                if(close(fclient) < 0)
                        return -1;
                break;

            case TFS_OP_CODE_OPEN:
                if(read(fsrv,&actual_client_session,sizeof(int)) <= 0)
                    return -1;
                char name_file[40];
                int open_flag,res1;
                uint8_t mensagem3[sizeof(int)];
                if(read(fsrv,name_file,40*sizeof(char)) <= 0)
                    return -1;
                if(read(fsrv,&open_flag,sizeof(int)) <= 0)
                    return -1;
                res1 = tfs_open(name_file,open_flag);
                memcpy(mensagem3,&res1,sizeof(int));
                if(write(fclient,mensagem3,sizeof(int)) <= 0)
                    return -1;
                break;
            case TFS_OP_CODE_CLOSE:
                if(read(fsrv,&actual_client_session,sizeof(int)) <= 0)
                    return -1;
                int fhandle1, res2;
                uint8_t mensagem4[sizeof(int)];
                if(read(fsrv,&fhandle1,sizeof(int)) <= 0)
                    return -1;
                res2 = tfs_close(fhandle1);
                memcpy(mensagem4,&res2,sizeof(int));
                if(write(fclient,mensagem4,sizeof(int)) <= 0)
                    return -1;
                break;
            case TFS_OP_CODE_WRITE:
                if(read(fsrv,&actual_client_session,sizeof(int)) <= 0)
                    return -1;
                int fhandle2;
                size_t write_len;
                int res3;
                uint8_t mensagem5[sizeof(ssize_t)];
                if(read(fsrv,&fhandle2,sizeof(int)) <= 0)
                    return -1;
                if(read(fsrv,&write_len,sizeof(size_t)) <= 0)
                    return -1;
                char *buffer_to_write = (char*)malloc(write_len);
                if(read(fsrv,buffer_to_write,write_len) <= 0)
                    return -1;
                res3 = (int)tfs_write(fhandle2,buffer_to_write,write_len);
                memcpy(mensagem5,&res3,sizeof(int));
                if(write(fclient,mensagem5,sizeof(int)) <= 0)
                    return -1;
                break;
            case TFS_OP_CODE_READ:
                if(read(fsrv,&actual_client_session,sizeof(int)) <= 0)
                    return -1;
                int fhandle3;
                if(read(fsrv,&fhandle3,sizeof(int)) <= 0)
                    return -1;
                size_t write_len2;
                int res4;
                if(read(fsrv,&write_len2,sizeof(size_t)) <= 0)
                    return -1;
                char *buffer_read= (char*)malloc(write_len2);
                res4 = (int)tfs_read(fhandle3,buffer_read,write_len2);
                char *buffer_aux = (char*)malloc((size_t)res4);
                memcpy(buffer_aux,buffer_read,(size_t)res4);
                uint8_t *mensagem6 = (uint8_t*)malloc(sizeof(int) + (unsigned long)res4);
                memcpy(mensagem6,&res4,sizeof(int));
                memcpy(mensagem6+sizeof(int),buffer_aux,(unsigned long)res4);
                if(write(fclient,mensagem6,sizeof(int)+(size_t)res4) <= 0)
                    return -1;
                break;

            case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED:
                if(read(fsrv,&actual_client_session,sizeof(int)) <= 0)
                    return -1;
                int res5;
                res5 = tfs_destroy_after_all_closed();
                uint8_t mensagem7[sizeof(int)];
                memcpy(mensagem7,&res5,sizeof(int));
                if(write(fclient,mensagem7,sizeof(int)) <= 0)
                    return -1;

                for(int i=0;i<S;i++){
                    session_id_array[i].status = 0;
                    memset(session_id_array[i].pipe_name,0,40);
                }
                close(fsrv);
                close(fclient);

                unlink(srv_pipename);
                return 0;
            default:
                break;  
        }
    }
    return 0;
}

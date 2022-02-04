#include "tecnicofs_client_api.h"
#include <unistd.h>
#include <stdio.h>
//#include <errno.h>

int actual_session_id = 0;
static char client_p_path[40];
static char server_p_path[40];
int fserv, fclient;
//extern int errno ;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    unlink(client_pipe_path);
    strcpy(server_p_path,server_pipe_path);
    strcpy(client_p_path,client_pipe_path);
    if(mkfifo(client_pipe_path,0777) == -1){
        //if(errno == EEXITS)
        return -1;
    }
    //named pipe do servidor -> aberto para escrever
    if((fserv = open(server_p_path,O_WRONLY)) < 0){
        return -1;
    }    


    //mensagem a enviar
    uint8_t mensagem[41*sizeof(char)];
    int resposta;
    char opcode = TFS_OP_CODE_MOUNT;

    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),client_pipe_path,40*sizeof(char));


    //envia mensagem para o servidor
    if(write(fserv,mensagem,41*sizeof(char)) < 0)
        return -1;

    //named pipe do cliente -> aberto para ler
    if ((fclient = open(client_p_path,O_RDONLY)) < 0){
        return -1;
    }

    //lê a mensagem vinda do servidor
    if(read(fclient,&resposta,sizeof(int)) < 0)
        return -1;

    actual_session_id = resposta;
    if (actual_session_id == -1)
        return -1;

    return 0;
}

int tfs_unmount() {
    uint8_t mensagem[sizeof(char)+sizeof(int)];
    int resposta;
    char opcode = TFS_OP_CODE_UNMOUNT;
    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),&actual_session_id,sizeof(int));

    if(write(fserv,mensagem,sizeof(char)+sizeof(int)) <= 0)
        return -1;
    if(read(fclient,&resposta,sizeof(int)) <= 0)
        return -1;

    if (resposta == -1)
        return -1;

    //fecha os named pipes (cliente e servidor) que o cliente tinha aberto
    if(close(fclient) < 0) {
        return -1;
    }
    if (close(fserv) < 0){
        return -1;
    }
    //apaga o named pipe do cliente
    if(unlink(client_p_path) < 0) {
        return -1;
    }
    return 0;
}

int tfs_open(char const *name, int flags) {
    uint8_t mensagem[41*sizeof(char)+(2*sizeof(int))];
    int resposta;
    char opcode = TFS_OP_CODE_OPEN;
    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),&actual_session_id,sizeof(int));
    memcpy(mensagem+sizeof(char)+sizeof(int),name,40);
    memcpy(mensagem+41*sizeof(char)+sizeof(int),&flags,sizeof(int));
    if(write(fserv,mensagem,41*sizeof(char)+(2*sizeof(int))) <= 0)
        return -1;
    if(read(fclient,&resposta,sizeof(int)) <= 0)
        return -1;
    if (resposta == -1)
        return -1;

    return resposta;
}

int tfs_close(int fhandle) {
    uint8_t mensagem[sizeof(char)+(2*sizeof(int))];
    int resposta;
    char opcode = TFS_OP_CODE_CLOSE;
    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),&actual_session_id,sizeof(int));
    memcpy(mensagem+sizeof(char)+sizeof(int),&fhandle,sizeof(int));

    if(write(fserv,mensagem,sizeof(char)+(2*sizeof(int))) <= 0)
        return -1;
    if(read(fclient,&resposta,sizeof(int)) <= 0)
        return -1;
    if (resposta == -1)
        return -1;

    return resposta;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    uint8_t mensagem[((len+1)*sizeof(char))+(2*sizeof(int))+sizeof(size_t)];
    int resposta;
    char opcode = TFS_OP_CODE_WRITE;
    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),&actual_session_id,sizeof(int));
    memcpy(mensagem+sizeof(char)+sizeof(int),&fhandle,sizeof(int));
    memcpy(mensagem+sizeof(char)+(2*sizeof(int)),&len,sizeof(size_t));
    memcpy(mensagem+sizeof(char)+(2*sizeof(int))+sizeof(size_t),buffer,len);
    if(write(fserv,mensagem,((len+1)*sizeof(char))+(2*sizeof(int))+sizeof(size_t)) <= 0)
        return -1;
    if(read(fclient,&resposta,sizeof(int)) <= 0)
        return -1;
    if (resposta == -1)
        return -1;
    return resposta;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    uint8_t mensagem[sizeof(char)+2*sizeof(int)+sizeof(size_t)];
    char opcode = TFS_OP_CODE_READ;
    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),&actual_session_id,sizeof(int));
    memcpy(mensagem+sizeof(char)+sizeof(int),&fhandle,sizeof(int));
    memcpy(mensagem+sizeof(char)+(2*sizeof(int)),&len,sizeof(size_t));

    if(write(fserv,mensagem,(sizeof(char)+(2*sizeof(int))+sizeof(size_t))) == -1)
        return -1;
    //if(errno == EPIPE)
        //return destroy_session

    int read_len;
    //verificar se leu tudo
    if(read(fclient,&read_len,sizeof(int)) <= 0)
        return -1;
    if(read(fclient,buffer,(size_t)read_len) <= 0)
        return -1;
    if (read_len == -1)
        return -1;
    return read_len;
}

int tfs_shutdown_after_all_closed() {
    uint8_t mensagem[sizeof(char)+sizeof(int)];
    int resposta;
    char opcode = TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED;
    memcpy(mensagem,&opcode,sizeof(char));
    memcpy(mensagem+sizeof(char),&actual_session_id,sizeof(int));

    if(write(fserv,mensagem,sizeof(char)+sizeof(int)) <= 0)
        return -1;

    if(read(fclient,&resposta,sizeof(int)) <= 0)
        return -1;
    if (resposta == -1)
        return -1;

    tfs_unmount();
    return 0;
}
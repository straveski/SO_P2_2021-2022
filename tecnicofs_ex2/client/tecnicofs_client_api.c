#include "tecnicofs_client_api.h"

int actual_session_id = 0;
static char client_p_path[40];
static char server_p_path[40];
int fserv, fclient;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    char mensagem[41], resposta[1];
    strcpy(server_p_path,server_pipe_path);
    strcpy(client_p_path,client_pipe_path);
    if(mkfifo(client_pipe_path,0666) < 0)
        return -1;

    //named pipe do servidor -> aberto para escrever
    if(fserv = open(server_pipe_path,O_WRONLY) < 0)
        return -1;
    //named pipe do cliente -> aberto para ler
    if (fclient = open(client_pipe_path,O_RDONLY) < 0)
        return -1;

    //mensagem a enviar
    mensagem[0] = TFS_OP_CODE_MOUNT;
    strcpy(mensagem[2],client_pipe_path);

    //envia mensagem para o servidor
    write(fserv,mensagem,42);

    //lê a mensagem vinda do servidor
    read(fclient,resposta,1);

    actual_session_id = (int*)resposta;
    if (actual_session_id == -1)
        return -1;

    return 0;
}

int tfs_unmount() {
    char mensagem[2], resposta[1];
    mensagem[0] = TFS_OP_CODE_UNMOUNT;
    mensagem[1] = (char*)actual_session_id;

    write(fserv,mensagem,2);
    read(fclient,resposta,1);
    if ((int*)resposta == -1)
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
    char mensagem[43],resposta[1];
    mensagem[0] = TFS_OP_CODE_OPEN;
    mensagem[1] = (char*)actual_session_id;
    memcpy(mensagem[2],name,40);
    mensagem[43] = (char*)flags;

    write(fserv,mensagem,43);
    read(fclient,resposta,1);
    if ((int*)resposta == -1)
        return -1;

    return 0;
}

int tfs_close(int fhandle) {
    char mensagem[3],resposta[1];
    mensagem[0] = TFS_OP_CODE_CLOSE;
    mensagem[1] = (char*)actual_session_id;
    mensagem[2] = (char*)fhandle;

    write(fserv,mensagem,3);
    read(fclient,resposta,1);
    if ((int*)resposta == -1)
        return -1;

    return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    
    return 0;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    
    return -1;
}

int tfs_shutdown_after_all_closed() {
    char mensagem[2],resposta[1];
    mensagem[0] = TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED;
    mensagem[1] = (char*)actual_session_id;

    write(fserv,mensagem,2);
    read(fclient,resposta,1);
    if ((int*)resposta == -1)
        return -1;

    tfs_unmount();
    return 0;
}

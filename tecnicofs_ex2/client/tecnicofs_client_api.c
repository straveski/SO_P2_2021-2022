#include "tecnicofs_client_api.h"

int actual_session_id;
static char client_path[40];
static char server_pipe_path[40];

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    //criar named pipe do cliente
    if(mkfifo(client_pipe_path,0666) < 0)
        return -1;


    //se sucesso -> o session_id associado à nova sessão terá sido guardado numa variável do cliente que indica qual a sessão que o cliente tem ativa neste momento;
    //senao -> return -1

    //named pipe do cliente -> aberto para ler
    open(client_pipe_path,O_RDONLY);

    //named pipe do servidor -> aberto para escrever
    open(server_pipe_path,O_WRONLY);
    return -1;
}

int tfs_unmount() {
    /* TODO: Implement this */
    //fecha os named pipes (cliente e servidor) que o cliente tinha aberto
    close(session_id_array[actual_session_id-1].pipe_name);
    close(server_pipe_path);

    //apaga o named pipe do cliente
    unlink(session_id_array[actual_session_id-1].pipe_name);
    return -1;
}

/*DUVIDAS:
- como vejo se dá erro ou não?
- nao tenho que ter variaveis globais para poder aceder aos pipes no tfs_unmount?
- session_id???? temos que criar né?
- podemos adicionar includes?
- como sei qual é a flag no mkfifo?
- tamanho maximo dos pipes vamos ver onde?*/

int tfs_open(char const *name, int flags) {
    /* TODO: Implement this */
    return -1;
}

int tfs_close(int fhandle) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

int tfs_shutdown_after_all_closed() {
    /* TODO: Implement this */
    return -1;
}

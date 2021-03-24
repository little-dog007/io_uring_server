#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>


#include "http.h"
#include "util.h"





void handle_get_method(struct io_uring *ring,char *path, int client_socket) {
    char final_path[1024];

    if (path[strlen(path) - 1] == '/') {
        strcpy(final_path, "public");
        strcat(final_path, path);
        strcat(final_path, "index.html");
    }
    else {
        strcpy(final_path, "public");
        strcat(final_path, path);
    }
    strcpy(final_path,"plugin_module-debug.zip");
    printf("path :%s\n",final_path);


   
    struct stat path_stat;
    if (stat(final_path, &path_stat) == -1) {
        printf("404 Not Found: %s (%s)\n", final_path, path);
        handle_http_404(ring,client_socket);
    }
    else {
        if (S_ISREG(path_stat.st_mode)) {
            struct request *req = zh_malloc(sizeof(*req) + (sizeof(struct iovec) * 6));
            req->iovec_count = 6;
            req->client_socket = client_socket;
            send_headers(final_path, path_stat.st_size, req->iov);
            copy_file_contents(final_path, path_stat.st_size, &req->iov[5],req);
            printf("200 %s %ld bytes\n", final_path, path_stat.st_size);
            add_write_request( ring,req);
        }
        else {
            handle_http_404(ring,client_socket);
            printf("404 Not Found: %s\n", final_path);
        }
    }
}


void handle_http_method(struct io_uring *ring,char *method_buffer, int client_socket) {
    char *method, *path, *saveptr;

    method = strtok_r(method_buffer, " ", &saveptr);
    strtolower(method);
    path = strtok_r(NULL, " ", &saveptr);

    if (strcmp(method, "get") == 0) {
        handle_get_method(ring,path, client_socket);
    }
    else {
        handle_unimplemented_method(ring,client_socket);
    }
}


int handle_client_request(struct io_uring *ring,struct request *req) {
    char http_request[1024];
    /* Get the first line, which will be the request */
    printf("request %s\n",req->iov[0]);
    if(get_line(req->iov[0].iov_base, http_request, sizeof(http_request))) {
        fprintf(stderr, "Malformed request\n");
        exit(1);
    }
   
    handle_http_method(ring,http_request, req->client_socket);
    return 0;
}

void server_loop(struct io_uring*ring,int server_socket) {
    struct io_uring_cqe *cqe;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    add_accept_request(ring,server_socket, &client_addr, &client_addr_len);

    while (1) {
        int ret = io_uring_wait_cqe(ring, &cqe);
        struct request *req = (struct request *) cqe->user_data;
        if (ret < 0)
            fatal_error("io_uring_wait_cqe");
        if (cqe->res < 0) {
            fprintf(stderr, "Async request failed: %s for event: %d\n",
                    strerror(-cqe->res), req->event_type);
            exit(1);
        }

        switch (req->event_type) {
            case EVENT_TYPE_ACCEPT:
                add_accept_request(ring,server_socket, &client_addr, &client_addr_len);
                add_read_request(ring,cqe->res);
                free(req);
                break;
            case EVENT_TYPE_READ:
                if (!cqe->res) {
                    fprintf(stderr, "Empty request!\n");
                    break;
                }
                handle_client_request(ring,req);
                free(req->iov[0].iov_base);
                free(req);
                break;
            case EVENT_TYPE_WRITE:{
                bool complete = check_copy_complete(ring,req,cqe->res,req->client_socket);
               
                for (int i = 0; i < req->iovec_count-1; i++) {
                    free(req->iov[i].iov_base);
                }
                if(complete){
                    printf("close socket\n");
                    shutdown(req->client_socket,SHUT_RD);
                    free(req->file_buff);
                    close(req->client_socket);
                }
                free(req);
                break;
            }
        }
        /* Mark this request as processed */
        io_uring_cqe_seen(&ring, cqe);
    }
}

// 不应该使用全局变量。 只是用于信号函数
struct io_uring *handler_ring = 0;

void sigint_handler(int signo) {
    printf("^C pressed. Shutting down.\n");
    if(handler_ring != 0){
        io_uring_queue_exit(handler_ring);
    }
    
    exit(0);
}

int main() {
    if (check_kernel_version()) {
        return EXIT_FAILURE;
    }
    
    check_for_index_file();
    int server_socket = setup_listening_socket(DEFAULT_SERVER_PORT);
    printf("io_uring_server listening on port: %d\n", DEFAULT_SERVER_PORT);

    struct io_uring ring;
    handler_ring = &ring;
    signal(SIGINT, sigint_handler);
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    server_loop(&ring,server_socket);

    return 0;
}
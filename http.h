
#include <liburing.h>

#include"util.h"

#ifndef HTTP_H
#define HTTP_H

#define SERVER_STRING           "io_uring server\r\n"
#define DEFAULT_SERVER_PORT     8000
#define QUEUE_DEPTH             256
#define READ_SZ                 8192

#define EVENT_TYPE_ACCEPT       0
#define EVENT_TYPE_READ         1
#define EVENT_TYPE_WRITE        2




struct request {
    int event_type;
    int iovec_count;
    int client_socket;

    // 因为要发多次，所以需要记录文件状态
    char * file_buff;
    int file_size;
    int cur_file_pos;
    struct iovec iov[];
    
};

// 失败html
const char *unimplemented_content = \
                                "HTTP/1.0 400 Bad Request\r\n"
                                "Content-type: text/html\r\n"
                                "\r\n"
                                "<html>"
                                "<head>"
                                "<title>io_uring_server: Unimplemented</title>"
                                "</head>"
                                "<body>"
                                "<h1>Bad Request (Unimplemented)</h1>"
                                "<p>Your client sent a request io_uring_server did not understand and it is probably not your fault.</p>"
                                "</body>"
                                "</html>";

// 404html
const char *http_404_content = \
                                "HTTP/1.0 404 Not Found\r\n"
                                "Content-type: text/html\r\n"
                                "\r\n"
                                "<html>"
                                "<head>"
                                "<title>io_uring_server: Not Found</title>"
                                "</head>"
                                "<body>"
                                "<h1>Not Found (404)</h1>"
                                "<p>Your client is asking for an object that was not found on this server.</p>"
                                "</body>"
                                "</html>";


                                /*
 * This function is responsible for setting up the main listening socket used by the
 * web server.
 * */

int setup_listening_socket(int port) {
    int sock;
    struct sockaddr_in srv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        fatal_error("socket()");

    int enable = 1;
    if (setsockopt(sock,
                   SOL_SOCKET, SO_REUSEADDR,
                   &enable, sizeof(int)) < 0)
        fatal_error("setsockopt(SO_REUSEADDR)");
 

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* We bind to a port and turn this socket into a listening
     * socket.
     * */
    if (bind(sock,
             (const struct sockaddr *)&srv_addr,
             sizeof(srv_addr)) < 0)
        fatal_error("bind()");



    if (listen(sock, 10) < 0)
        fatal_error("listen()");

    return (sock);
}


int add_accept_request(struct io_uring*ring, int server_socket, struct sockaddr_in *client_addr,
                       socklen_t *client_addr_len) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, server_socket, (struct sockaddr *) client_addr,
                         client_addr_len, 0);
    struct request *req = malloc(sizeof(*req));
    req->event_type = EVENT_TYPE_ACCEPT;
    io_uring_sqe_set_data(sqe, req);
    io_uring_submit(ring);

    return 0;
}

int add_read_request(struct io_uring*ring,int client_socket) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    struct request *req = malloc(sizeof(*req) + sizeof(struct iovec));
    req->iov[0].iov_base = malloc(READ_SZ);
    req->iov[0].iov_len = READ_SZ;
    req->event_type = EVENT_TYPE_READ;
    req->client_socket = client_socket;
    memset(req->iov[0].iov_base, 0, READ_SZ);
    /* Linux kernel 5.5 has support for readv, but not for recv() or read() */
    io_uring_prep_readv(sqe, client_socket, &req->iov[0], 1, 0);
    io_uring_sqe_set_data(sqe, req);
    io_uring_submit(ring);
    return 0;
}

int add_write_request(struct io_uring*ring,struct request *req) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    req->event_type = EVENT_TYPE_WRITE;
    io_uring_prep_writev(sqe, req->client_socket, req->iov, req->iovec_count, 0);
    io_uring_sqe_set_data(sqe, req);
    io_uring_submit(ring);
    return 0;
}

void _send_static_string_content(struct io_uring*ring,const char *str, int client_socket) {
    struct request *req = zh_malloc(sizeof(*req) + sizeof(struct iovec));
    unsigned long slen = strlen(str);
    req->iovec_count = 1;
    req->client_socket = client_socket;
    req->iov[0].iov_base = zh_malloc(slen);
    req->iov[0].iov_len = slen;
    memcpy(req->iov[0].iov_base, str, slen);
    add_write_request(ring,req);
}

// 只支持get方法

void handle_unimplemented_method(struct io_uring*ring,int client_socket) {
    _send_static_string_content(ring,unimplemented_content, client_socket);
}

// 404

void handle_http_404(struct io_uring*ring,int client_socket) {
    _send_static_string_content(ring,http_404_content, client_socket);
}

int get_header_length(struct request*req){
    int len = 0;
    for(int i=0;i<req->iovec_count-1;++i){
        len += req->iov[i].iov_len;
    }
    return len;
}

//初始化 iov

void copy_file_contents(char *file_path, off_t file_size, struct iovec *iov,struct request * req) {
    int fd;

    char *buf = zh_malloc(file_size);
    fd = open(file_path, O_RDONLY);
    if (fd < 0)
        fatal_error("read");

    /* We should really check for short reads here */
    int ret = read(fd, buf, file_size);
    if (ret < file_size) {
        fprintf(stderr, "Encountered a short read.\n");
    }
    close(fd);

    // 因为需要计算获取异步读写ret，ret中包含http头长度，应该减去
    req->file_buff = buf;
    req->cur_file_pos = -get_header_length(req);
    req->file_size = file_size;

    iov->iov_base = buf;
    iov->iov_len = file_size;
    
    printf("file buff %p ,end buff %p\n",buf,buf+file_size);
}

bool check_copy_complete(struct io_uring * uring,struct request *req,int ret,int socket){
    struct iovec * iov = &req->iov[req->iovec_count-1];
    int cur_size = req->cur_file_pos;

    printf("ret: %d, cur_size:%d \n",ret,cur_size);
    char *p = iov->iov_base;
    printf("file buff %p ,end buff %p\n",p,p+iov->iov_len);

   
    if(ret + cur_size<req->file_size){
        struct request *req_ = malloc(sizeof(*req_) + sizeof(struct iovec));
        req_->cur_file_pos = ret + cur_size;
        req_->file_buff = req->file_buff;
        req_->file_size = req->file_size;
        req_->client_socket = socket;
        req_->iovec_count = 1;

        req_->iov[0].iov_base = (char*)req_->file_buff+ req_->cur_file_pos;
        req_->iov[0].iov_len = req_->file_size - req_->cur_file_pos;

        sleep(1);
        
      
        
        add_write_request(uring,req_);
        return false;
    }
    return true;

}


// 发送头部

void send_headers(const char *path, off_t len, struct iovec *iov) {
    char small_case_path[1024];
    char send_buffer[1024];
    strcpy(small_case_path, path);
    strtolower(small_case_path);

    char *str = "HTTP/1.0 200 OK\r\n";
    unsigned long slen = strlen(str);
    iov[0].iov_base = zh_malloc(slen);
    iov[0].iov_len = slen;
    memcpy(iov[0].iov_base, str, slen);

    slen = strlen(SERVER_STRING);
    iov[1].iov_base = zh_malloc(slen);
    iov[1].iov_len = slen;
    memcpy(iov[1].iov_base, SERVER_STRING, slen);

    /*
     * Check the file extension for certain common types of files
     * on web pages and send the appropriate content-type header.
     * Since extensions can be mixed case like JPG, jpg or Jpg,
     * we turn the extension into lower case before checking.
     * */
    const char *file_ext = get_filename_ext(small_case_path);
    if (strcmp("jpg", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: image/jpeg\r\n");
    if (strcmp("jpeg", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: image/jpeg\r\n");
    if (strcmp("png", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: image/png\r\n");
    if (strcmp("gif", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: image/gif\r\n");
    if (strcmp("htm", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: text/html\r\n");
    if (strcmp("html", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: text/html\r\n");
    if (strcmp("js", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: application/javascript\r\n");
    if (strcmp("css", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: text/css\r\n");
    if (strcmp("txt", file_ext) == 0)
        strcpy(send_buffer, "Content-Type: text/plain\r\n");
    if(strcmp("zip",file_ext) == 0){
        strcpy(send_buffer,"Content-Type: application/zip\r\n");
    }
    slen = strlen(send_buffer);
    iov[2].iov_base = zh_malloc(slen);
    iov[2].iov_len = slen;
    memcpy(iov[2].iov_base, send_buffer, slen);

    /* Send the Content-length header, which is the file size in this case. */
    sprintf(send_buffer, "Content-length: %ld\r\n", len);
    slen = strlen(send_buffer);
    iov[3].iov_base = zh_malloc(slen);
    iov[3].iov_len = slen;
    memcpy(iov[3].iov_base, send_buffer, slen);

    /*
     * When the browser sees a '\r\n' sequence in a line on its own,
     * it understands there are no more headers. Content may follow.
     * */
    strcpy(send_buffer, "\r\n");
    slen = strlen(send_buffer);
    iov[4].iov_base = zh_malloc(slen);
    iov[4].iov_len = slen;
    memcpy(iov[4].iov_base, send_buffer, slen);

    printf("0 :%s\n",iov[0].iov_base);
    printf("1: %s\n",iov[1].iov_base);
    printf("2 :%s\n",iov[2].iov_base);
    printf("3 :%s\n",iov[3].iov_base);
    printf("4 :%s\n",iov[4].iov_base);
}

void check_for_index_file() {
    struct stat st;
    int ret = stat("public/index.html", &st);
    if(ret < 0 ) {
        fprintf(stderr, "ZeroHTTPd needs the \"public\" directory to be "
                "present in the current directory.\n");
        fatal_error("stat: public/index.html");
    }
}

#endif HTTP_H
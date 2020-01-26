#include <stdio.h>
#include <unistd.h>     // read
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* The program hangs on the read() and write() and it is normal.
 * From man page:
 * FIFO special file has to be open at both ends simultaneously before
 * you can proceed to do any input or output operations on it.  Opening
 * a FIFO for reading normally blocks until some other process opens the
 * same FIFO for writing, and vice versa.
 * 
 * Use next cmds to first write smth to in.fifo file and read this from out.fifo:
 * roman@DESKTOP-AFO4O38:~$ echo 'string' > ./in.fifo
 * roman@DESKTOP-AFO4O38:~$ cat ./out.fifo
 */

int main(int argc, char **argv) {
    //const char * in_fifo  = "/home/box/in.fifo";
    const char * in_fifo  = "/home/roman/in.fifo";
    //const char * out_fifo = "/home/box/out.fifo";
    const char * out_fifo = "/home/roman/out.fifo";
    int mk_infifo_rc  = mkfifo(in_fifo, 0666);
    int mk_outfifo_rc = mkfifo(out_fifo, 0666);
    
    printf("%d - mk_infifo_rc; %d - mk_outfifo_rc\n", mk_infifo_rc, mk_outfifo_rc);
    
    int fd_in  = open(in_fifo, O_RDONLY | O_NONBLOCK);
    int fd_out = open(out_fifo, O_WRONLY );

    char buf[1024];
    ssize_t n_read  = read(fd_in, buf, 1024);
    ssize_t n_write = write(fd_out, buf, n_read);
    
    printf("%zu bytes read; %zu bytes written\n", n_read, n_write);

    return 0;
}

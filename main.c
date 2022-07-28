#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
 
int main(int argc, char** argv) {
    uint8_t keys[128];
    int fd;
 
    fd = open(argv[1], O_RDONLY);
 
    while(1)
    {
        struct input_event ev;
 
        if(read(fd, &ev, sizeof(struct input_event)) < 0)
        {
            if(errno == EINTR)
                continue;
 
            break;
        }

        printf("read evcode is %d evvalue is %d\n", ev.code, ev.value);
              
        if(EV_KEY != ev.type)
            continue;
 
        switch(ev.code)
        {
            if(1 == ev.value)
            {
                printf("down");
            }
            else
            {
                printf("up");
            }
             
        }
        printf(".");
    }
 
    close(fd);
 
    return 0;
}
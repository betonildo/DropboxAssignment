#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define PATH_MAX 4096

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

char* concat(char* str1, char* str2);

int main(int argc, char **argv) {

  int length, i = 0;
  int fd;
  int wd;
  char buffer[BUF_LEN];

  fd = inotify_init();

  if (fd < 0) perror("inotify_init");

  // unsigned int watchEvents = IN_MODIFY | IN_CREATE | IN_DELETE;
  wd = inotify_add_watch(fd, argv[1], IN_ALL_EVENTS);
  printf("%d\n", wd);
  while ( (length = read(fd, buffer, BUF_LEN)) > 0) {  
    
    i = 0;
    if (length < 0) perror("read");

    while (i < length) {
      struct inotify_event *event = (struct inotify_event*) &buffer[i];
      if (event->len) {
        if (event->mask & IN_CREATE) {
          if (event->mask & IN_ISDIR) {
            
            printf("The directory %s was created.\n", event->name);
            char* newDirToMonitor = concat(argv[1], event->name);
            int wdn = inotify_add_watch(fd, newDirToMonitor, IN_ALL_EVENTS);
            printf("%d\n", wdn);
          }
          else printf("The file %s was created.\n", event->name);
        }
        else if (event->mask & IN_DELETE) {
          if (event->mask & IN_ISDIR) printf("The directory %s was deleted.\n", event->name);
          else printf("The file %s was deleted.\n", event->name);
        }
        else if (event->mask & IN_MODIFY) {
          if (event->mask & IN_ISDIR) printf("The directory %s was modified.\n", event->name);
          else printf("The file %s was modified.\n", event->name);
        }
      }
      unsigned int nextPace = EVENT_SIZE + event->len;
      i += nextPace;
    }

    memset(buffer, 0, BUF_LEN);
  }

  inotify_rm_watch(fd, wd);
  close(fd);
  
  return 0;
}


char* concat(char* str1, char* str2) {
    int strTotalLength = strlen(str1) + strlen(str2) + 1;
    char* newstr = (char*) calloc(strTotalLength, sizeof(char));
    strcat(newstr, str1);
    strcat(newstr, str2);
    printf("'%s'\n", newstr);
    return newstr;
}
#ifndef READER_H
#define READER_H

struct queue;

int lookup(char* address, char* port);
int reader(int, struct queue*);

#endif // READER_H

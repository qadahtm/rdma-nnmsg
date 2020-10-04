#include <iostream>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <string>

using namespace std;
struct msg{
    unsigned int seq;
	char *buf;
};
struct msg* create_message(char *buf, long long seq){
	struct msg *temp = (struct msg *) malloc(sizeof(struct msg));
	temp->buf = (char *)malloc(227);
	strcpy(temp->buf, buf);
	temp->seq = seq;
	return temp;
}

unsigned int get_sequence(struct msg *temp){
    return temp->seq;
}
char * get_message(struct msg *temp){
    return temp->buf;
}

char * serialise(struct msg *temp){
    char *buf = temp -> buf;
    unsigned int seq = temp->seq;
    char seq_buf[21];
    sprintf(seq_buf, "%u", seq);
    char* to_return = (char *)malloc(248);
    memset(to_return, 0, 248);
    strcpy(to_return, seq_buf);
    strcpy(to_return + 29, buf);
    return to_return;
}

struct msg* deserialize(char *temp){
    unsigned int temp_s = strtoull(temp + 8, 0, 0);
    char *buf = (temp + 37);
    return create_message(buf, temp_s);
}
int main(){
    struct msg* temp = create_message("Message LOL", 123);
    char *sbuf = serialise(temp);
    char *a = (char *)malloc(256);
    memset(a, 0, 256);
    memcpy(a + 8, sbuf, 248);
    struct msg *temp_s = deserialize(a);
    cout << "DEBUG:A " << get_message(temp_s) << " SEQ: " << get_sequence(temp_s) << endl;
}
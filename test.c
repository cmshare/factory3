#include <stdio.h>
#include <string.h>
int main(void){
 char str[32];
 char *username="18913949200";
 int i;
 for(i=0;i<32;i++)str[i]=i;
  
 strncpy(str,username,21);
 for(i=0;i<32;i++){
 printf("%d\n",(int)str[i]);
}
 return 0;
}

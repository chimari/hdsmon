#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPEECH_COM "echo '%s' | open_jtalk -x /var/lib/mecab/dic/open-jtalk/naist-jdic -m /usr/share/hts-voice/mei/mei_normal.htsvoice -r 1.0 -ow /tmp/tmp.wav && aplay /tmp/tmp.wav > /dev/null 2>&1"
//#define SPEECH_COM "echo '%s' | festival --tts --pipe"

main()
{
  FILE *fp;
  char buf[1000];
  char com[1010];
  int i;
 
  fp=fopen("/tmp/speech","r");
  while( fgets(buf,1000,fp)!=NULL){
 
    if ((buf[0]=='#')
	&&(buf[1]=='s')
	&&(buf[2]=='a')
	&&(buf[3]=='y')
	&&(buf[4]=='#')){
      i=5;
      while((i<999) && buf[i]!='\n'){i++;}
      buf[i]=0x00;
      //      sprintf(com,"say -v Vicki '%s'",buf+5);
      sprintf(com,SPEECH_COM,buf+5);
      system(com);
    }
    else if ((buf[0]=='#')
	     &&(buf[1]=='m')
	     &&(buf[2]=='a')
	     &&(buf[3]=='c')
	     &&(buf[4]=='o')
	     &&(buf[5]=='p')
	     &&(buf[6]=='i')
	     &&(buf[7]=='x')
	     &&(buf[8]=='#')){
      i=9;
      while((i<999) && buf[i]!='\n'){i++;}
      buf[i-1]=0x00;
      sprintf(com,"macopix --message \"%s\"",buf+9);
      system(com);
    }
  }
}

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<time.h>
#include<unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include<curses.h>

#include"hdscmon.h"

gint time_func(gpointer data){
  struct stat status_stat;
  static time_t up_time;

  if(stat(STATUSDATA,&status_stat)){
    // fprintf(stderr,"STATUS read error\n");
  }
  else if (status_stat.st_mtime !=up_time){
    // modified timeが新しいときのみ再読み込み 
    file_search((hds_param *)data);
    // update_gui((hds_param *)data);
  }
  up_time=status_stat.st_mtime;

}

void file_search(hds_param *hds){
  FILE *fp_read;
  char c_tmp[BUFFSIZE];
  static char buf[LINE_NUMBER][BUFFSIZE];
  int *cover_tmp;
  int read_line=0;

  gboolean e_flag=FALSE;

#ifdef DEBUG2
  g_print("   start file search\n");
#endif

  if((fp_read=fopen(STATUSDATA,"r"))==NULL){
    fprintf(stderr,"STATUS read error\n");
    return;
  }

#ifdef DEBUG
  fprintf(stderr,"STATUS reading...\n");
#endif

  read_line=0;
  while(!feof(fp_read)){
    fgets(buf[read_line],BUFFSIZE-1,fp_read);
    read_line++;
  }
  fclose(fp_read);

  if(read_line!=LINE_NUMBER){
    //g_print("!!! DATA reading error...\n");
    move(0,0);
    printw("!");
    return;
  }
  
  attron(A_REVERSE);
  move(0,0);
  printw(" ");
  attroff(A_REVERSE);
  
#ifdef DEBUG2
  g_print("   reading update-time\n");
#endif

  read_line=0;
  /* 更新日時 */
  strcpy(hds->update_time,get_param(buf[read_line]," = "));
  read_line++;
  /* Driving */
  hds->status_driving=get_status(buf[read_line]," = ");
  read_line++;

#ifdef DEBUG2
  g_print("   reading mode\n");
#endif

  /*** モード ***/
  read_line++;
  /* OBS-MODE */
  hds->mode_obs=get_status(buf[read_line]," = ");
  read_line++;
  /* I2-CELL */
  hds->mode_i2=get_status2(buf[read_line]," = ");
  read_line++;
  /* Light Monitor */
  hds->mode_lm=get_status2(buf[read_line]," = ");
  read_line++;


#ifdef DEBUG2
  g_print("   reading slit\n");
#endif

  /*** SLIT ***/
  read_line++;
  /* Slit Width */
  strcpy(c_tmp,get_param(buf[read_line],"= "));
  read_line++;
  sscanf(c_tmp,"%f",&hds->slit_width);
  /* Slit Length */
  strcpy(c_tmp,get_param(buf[read_line],"= "));
  read_line++;
  sscanf(c_tmp,"%f",&hds->slit_length);
  /* Filer1 */
  strcpy(hds->slit_filter1,get_param(buf[read_line]," = "));
  read_line++;
  /* Filer2 */
  strcpy(hds->slit_filter2,get_param(buf[read_line]," = "));
  read_line++;

#ifdef DEBUG2
  g_print("   reading shutter\n");
#endif

  /*** SHUTTER ***/
  read_line++;
  /* Shutter */
  hds->shutter_shutter=get_oc(buf[read_line]," = ");
  read_line++;
  /* Hartman Upper */
  hds->shutter_hu=get_oc(buf[read_line]," = ");
  read_line++;
  /* Hartman Lower */
  hds->shutter_hl=get_oc(buf[read_line]," = ");
  read_line++;


#ifdef DEBUG2
  g_print("   reading collimator/grating\n");
#endif

  /*** Collimator/Graing ***/
  read_line++;
  /* Collimator */
  hds->col_col=get_br(buf[read_line]," = ");
  read_line++;
  /* Cross Disperser */
  hds->col_cross=get_br(buf[read_line]," = ");
  read_line++;
  /* Echelle */
  hds->col_echelle=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* Cross Scan */
  hds->col_scan=atof(get_param(buf[read_line]," = "));
  read_line++;
  
#ifdef DEBUG2
  g_print("   reading detector\n");
#endif

  /*** Detector ***/
  read_line++;
  /* Rotate */
  hds->cam_rotate=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* Z */
  hds->cam_z=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* X */
  hds->cam_x=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* Y */
  hds->cam_y=atof(get_param(buf[read_line]," = "));
  read_line++;


#ifdef DEBUG2
  g_print("   reading cover\n");
#endif

  /*** Cover ***/
  read_line++;
  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->cover_lens1=cover_tmp[0];
  hds->cover_colr=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->cover_lens2=cover_tmp[0];
  hds->cover_colb=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->cover_lens3=cover_tmp[0];
  hds->cover_crossr=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->cover_cam1=cover_tmp[0];
  hds->cover_crossb=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->cover_cam2=cover_tmp[0];
  hds->cover_mirr=cover_tmp[1];

#ifdef DEBUG2
  g_print("   reading option\n");
#endif

  /*** Option ***/
  read_line++;
  hds->option_i2=get_io(buf[read_line]," = ");
  read_line++;
  hds->option_lm=get_io(buf[read_line]," = ");
  read_line++;
  hds->temp_i2i=atof(get_param(buf[read_line],"= "));
  read_line++;
  hds->temp_i2o=atof(get_param(buf[read_line],"= "));
  read_line++;

#ifdef DEBUG2
  g_print("   reading cryostadt\n");
#endif

  /*** Cryostadt ***/
  read_line++;
  strcpy(hds->temp_time,get_param(buf[read_line]," = "));
  read_line++;
  hds->temp_ccd=atof(get_param(buf[read_line],"= "))/10;
  read_line++;

#ifdef DEBUG2
  g_print("   reading temp\n");
#endif

  /*** Nasmyth Room Temperature ***/
  read_line++;
  hds->temp_nr1=atof(get_param(buf[read_line],"= "));
  read_line++;
  hds->temp_nr2=atof(get_param(buf[read_line],"= "));
  read_line++;

  /* Image Slicer */
  read_line++;
  hds->is_unit=get_status_is(buf[read_line]," = ");
  read_line++;
  hds->is_width=atof(get_param(buf[read_line]," = "));
  read_line++;
  hds->is_slic=get_status_is(buf[read_line]," = ");
  read_line++;

  /*** Setting ***/
  hds->setting=get_setting(hds);

  
#ifdef DEBUG
  debug_print(hds);
#endif

#ifdef DEBUG2
  g_print("   closing\n");
#endif

  return;
}

int get_setting(hds_param *hds){
  if((hds->col_col==1)&&(hds->col_cross==1)){ // BLUE
    if(absvalue((double)(hds->col_scan-DELTA_CROSS-4.95))<E_SCAN){       //StdUa
      return (STDUA);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-4.75))<E_SCAN){  //StdUb
      return (STDUB);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-5.35))<E_SCAN){  //StdBa
      return (STDBA);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-5.526))<E_SCAN){ //StdBc
      return (STDBC);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-6.19))<E_SCAN){  //StdYa
      return (STDYA);
    }
    else{
      return (NOSTD);
    }
  }
  else if((hds->col_col==0)&&(hds->col_cross==0)){ //RED
    if(absvalue((double)(hds->col_scan-DELTA_CROSS-4.37))<E_SCAN){       //StdYb
      return (STDYB);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-4.583))<E_SCAN){  //StdYc
      return (STDYC);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-4.300))<E_SCAN){  //StdYd
      return (STDYD);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-5.126))<E_SCAN){  //StdRa
      return (STDRA);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-5.30))<E_SCAN){ //StdRb
      return (STDRB);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-7.00))<E_SCAN){  //StdNIRa
      return (STDNIRA);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-6.35))<E_SCAN){  //StdNIRb
      return (STDNIRB);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-5.933))<E_SCAN){  //StdNIRc
      return (STDNIRC);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-5.00))<E_SCAN){  //StdI2a
      return (STDI2A);
    }
    else if(absvalue((double)(hds->col_scan-DELTA_CROSS-3.90))<E_SCAN){  //StdI2b
      return (STDI2B);
    }
    else{
      return (NOSTD);
    }
  }
  else{
      return (NOSTD);
  }
}

double absvalue(double ip){
  if(ip<0){
    return(-ip);
  }
  else{
    return(ip);
  }
}


/* パラメータ リード関数 */
char *get_param(char * buf , char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  return(b);
}
  

/* status パラメータ リード関数 for ImgSlicer  1:IS#1 0:NOUSE */
int get_status_is(char *buf,  char *token){
  static char b[BUFFSIZE];
  char *c=NULL;
  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  return((int)atoi(b));
}

/* status パラメータ リード関数  1:Yes 0:No -1:UNKNOWN */
int get_status(char *buf, char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  if(!strcmp(capiterize(b),"YES")){
    return(1);
  }
  else if(!strcmp(capiterize(b),"NO")){
    return(0);
  }
  else{
    return(-1);
  }
}

/* status パラメータ リード関数その2  1:1 0:0 -1:UNKNOWN */
int get_status2(char *buf,  char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  if(!strcmp(capiterize(b),"1")){
    return(1);
  }
  else if(!strcmp(capiterize(b),"0")){
    return(0);
  }
  else{
    return(-1);
  }
}

/* OPEN/CLOSE パラメータ リード関数  1:OPEN 0:CLOSE -1:UNKNOWN */
int get_oc(char *buf, char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  if(!strcmp(capiterize(b),"OPEN")){
    return(1);
  }
  else if(!strcmp(capiterize(b),"CLOSE")){
    return(0);
  }
  else{
    return(-1);
  }
}

/* OPEN/CLOSE パラメータ リード関数そにょ2  1:OPEN 0:CLOSE -1:UNKNOWN */
int *get_oc2(char *buf, char *token){
  static int b[3];
  char *c=NULL;
  char cc[BUFFSIZE];

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  sscanf(c,"%s",cc);
  if(!strcmp(capiterize(cc),"OPEN")){
    b[0]=1;
  }
  else if(!strcmp(capiterize(cc),"CLOSE")){
    b[0]=0;
  }
  else{
    b[0]=-1;
  }
  c=(char *)strstr(c,token);
  c+=strlen(token);
  sscanf(c,"%s",cc);
  if(!strcmp(capiterize(cc),"OPEN")){
    b[1]=1;
  }
  else if(!strcmp(capiterize(cc),"CLOSE")){
    b[1]=0;
  }
  else{
    b[1]=-1;
  }

  return(b);
}

/* BLUE/RED パラメータ リード関数  2:MIRROR 1:BLUE 0:RED -1:UNKNOWN */
int get_br(char *buf, char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  if(!strcmp(capiterize(b),"BLUE")){
    return(1);
  }
  else if(!strcmp(capiterize(b),"RED")){
    return(0);
  }
  else if(!strcmp(capiterize(b),"MIRROR")){
    return(2);
  }
  else{
    return(-1);
  }
}


/* IN/OUT パラメータ リード関数  1:IN 0:OUT -1:UNKNOWN */
int get_io(char *buf, char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  if(!strcmp(capiterize(b),"IN")){
    return(1);
  }
  else if(!strcmp(capiterize(b),"OUT")){
    return(0);
  }
  else{
    return(-1);
  }
}


/* 文字列の大文字化 */
char *capiterize(char *c_in){
  static char b[BUFFSIZE];
  int i;
  for(i=0;i<strlen(c_in);i++){
    b[i]=toupper(c_in[i]);
  }
  b[i]='\0';

  return(b);
}
				       


int title_draw(){
  /* TITLE */
  attron(A_BOLD|A_REVERSE);
  move(0,0);
  printw("    Status of SUBARU HDS                                                       ");
  attroff(A_BOLD|A_REVERSE);
    


  /* Slit */
  attron(A_REVERSE);
  move(2,1);
  printw("Slit");
  attroff(A_REVERSE);

  attron(A_BOLD|A_UNDERLINE);
  move(3,2);
  printw("Width");
  
  move(4,2);
  printw("Length");
  attroff(A_BOLD|A_UNDERLINE);


  /* Camera */
  attron(A_REVERSE);
  move(6,1);
  printw("Camera");
  attroff(A_REVERSE);

  attron(A_BOLD);
  move(7,2);
  printw("Rotate");
  
  attron(A_UNDERLINE);
  move(8,7);
  printw("Z");
  attroff(A_UNDERLINE);

  move(9,7);
  printw("X");

  move(10,7);
  printw("Y");
  attroff(A_BOLD);


  /* Shutter */
  attron(A_REVERSE);
  move(12,1);
  printw("Shutter");
  attroff(A_REVERSE);

  attron(A_BOLD);
  attron(A_UNDERLINE);
  move(13,2);
  printw("Main Shutter");
  attroff(A_UNDERLINE);
  
  move(14,2);
  printw("Hartmann[Up/Low]");
  attroff(A_BOLD);


  /* I2-cell */
  attron(A_REVERSE);
  move(16,1);
  printw("I2-cell");
  attroff(A_REVERSE);

  attron(A_BOLD);
  move(17,2);
  printw("mode");
  
  move(17,17);
  printw("Pos.");
  attroff(A_BOLD);


  /* Cover */
  attron(A_REVERSE);
  move(19,1);
  printw("Cover");
  attroff(A_REVERSE);

  attron(A_BOLD);
  move(20,1);
  printw("Collimator");
  
  move(21,1);
  printw("Blue");

  move(21,8);
  printw("Red");

  move(20,18);
  printw("Cross Disperser");
  
  move(21,18);
  printw("Blue");

  move(21,25);
  printw("Red");

  move(21,32);
  printw("Mirror");

  move(20,42);
  printw("Camera Lens");
  
  move(21,42);
  printw("#1");

  move(21,49);
  printw("#2");

  move(21,56);
  printw("#3");

  move(20,66);
  printw("Camera");
  
  move(21,66);
  printw("#1");

  move(21,73);
  printw("#2");
  attroff(A_BOLD);


  /* Wave Setting */
  attron(A_REVERSE);
  move(2,41);
  printw("Wavelength Setting");
  attroff(A_REVERSE);

  attron(A_BOLD);
  attron(A_UNDERLINE);
  move(3,42);
  printw("Slit Filter");
  attroff(A_UNDERLINE);
  
  move(4,42);
  printw("Collimator");

  move(5,42);
  printw("Echelle");

  move(6,42);
  printw("Cross");

  attron(A_UNDERLINE);
  move(7,42);
  printw("Cross Scan");
  attroff(A_UNDERLINE);
  attroff(A_BOLD);


  /* Temperature */
  attron(A_REVERSE);
  move(9,41);
  printw("Temperature");
  attroff(A_REVERSE);

  attron(A_BOLD);
  move(10,42);
  printw("I2-cell(In/Out)");
  
  move(11,42);
  printw("Ns.Room(#1/#2)");

  move(12,42);
  printw("Detector");
  attroff(A_BOLD);
  

  /* Image Slicer */
  attron(A_REVERSE);
  move(14,41);
  printw("Image Slicer");
  attroff(A_REVERSE);


  /* Light Monitor */
  attron(A_REVERSE);
  move(16,41);
  printw("Light Monitor");
  attroff(A_REVERSE);

  attron(A_BOLD);
  move(17,42);
  printw("mode");
  
  move(17,57);
  printw("Pos.");
  attroff(A_BOLD);


  attron(A_BOLD|A_REVERSE);
  move(23,0);
  printw("  OBS-mode                                    (Update                       )  ");
  attroff(A_BOLD|A_REVERSE);
 
  move(0,0);
  return(0);
}

void init_hds(hds_param *hds){
  hds->slit_width=-9999;
  hds->slit_length=-9999;

  hds->cam_rotate=-9999;
  hds->cam_x=-9999;
  hds->cam_y=-9999;
  hds->cam_z=-9999;

  hds->shutter_shutter=-9999;
  hds->shutter_hu=-9999;
  hds->shutter_hl=-9999;

  hds->col_col=-9999;
  hds->col_echelle=-9999;
  hds->col_cross=-9999;
  hds->col_scan=-9999;

  hds->temp_i2o=-9999;
  hds->temp_i2i=-9999;
  hds->temp_nr1=-9999;
  hds->temp_nr2=-9999;
  hds->temp_ccd=-9999;

  hds->option_i2=-9999;
  hds->option_lm=-9999;

  hds->cover_colb=-9999;
  hds->cover_colr=-9999;
  hds->cover_crossb=-9999;
  hds->cover_crossr=-9999;
  hds->cover_mirr=-9999;
  hds->cover_lens1=-9999;
  hds->cover_lens2=-9999;
  hds->cover_lens3=-9999;
  hds->cover_cam1=-9999;
  hds->cover_cam2=-9999;

  hds->setting=-9999;

  return;
}



void set_oc2(hds_param *hds, int oc_status, int oc_old, int oc_old2)
{
  if(oc_status!=oc_old){
    attron(A_REVERSE);
  }

  if(oc_status==0){
    printw("Close");
  }
  else if(oc_status==1){
    attron(A_UNDERLINE);
    printw("Open");
    attroff(A_UNDERLINE);
    printw(" ");
  }
  else{
    printw("???  ");
  }
  attroff(A_REVERSE);

  return;
}


void set_io(hds_param *hds, int io_status, int io_old, int io_old2){
  if(io_status!=io_old){
    attron(A_REVERSE);
  }

  if(io_status==0){
    printw("Out");
  }
  else if(io_status==1){
    printw("In ");
  }
  else{
    printw("???");
  }
  attroff(A_REVERSE);

  return;
}


void set_br(hds_param *hds, int br_status, int br_old, int br_old2){
  if(br_status!=br_old){
    attron(A_REVERSE);
  }

  if(br_status==0){
    printw("Red   ");
  }
  else if(br_status==1){
    printw("Blue  ");
  }
  else if(br_status==2){
    printw("Mirror");
  }
  else{
    printw("???   ");
  }
  attroff(A_REVERSE);

  return;
}


int value_draw(hds_param *hds){
  static hds_param hds_old, hds_old2;
  char* tmp[256];


  if(hds->init_status==FALSE){
    init_hds(&hds_old);
    init_hds(&hds_old2);
  }

  attron(A_BOLD|A_REVERSE);
  move(0,60);
  if(hds->status_driving==1){
    attron(A_BLINK);
    printw("*** DRIVING! ***");
    attron(A_BLINK);
    attroff(A_BLINK);
  }
  else{
    printw("                ");
  }
  attroff(A_BOLD|A_REVERSE);

  //// Slit
  move(2,9);
  if(hds_old.slit_width!=hds->slit_width){
    attron(A_REVERSE);
  }
  printw("[R=%6d]", (int)(3.6e4/hds->slit_width));
  attroff(A_REVERSE);

  move(3,9);
  if(hds_old.slit_width!=hds->slit_width){
    attron(A_REVERSE);
  }
  printw("%7.4f[\"] = %5d[um]", hds->slit_width, 
	 (int)(hds->slit_width/2*1e3));
  attroff(A_REVERSE);

  move(4,9);
  if(hds_old.slit_length!=hds->slit_length){
    attron(A_REVERSE);
  }
  printw("%7.4f[\"] = %5d[um]", hds->slit_length, 
	 (int)(hds->slit_length/2*1e3));
  attroff(A_REVERSE);


  //// Camera
  move(7,9);
  if(hds_old.cam_rotate!=hds->cam_rotate){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[um]", hds->cam_rotate,
	 (int)(hds->cam_rotate*3600));
  attroff(A_REVERSE);

  move(8,9);
  if(hds_old.cam_z!=hds->cam_z){
    attron(A_REVERSE);
  }
  printw("%7.4f[mm]", hds->cam_z);
  attroff(A_REVERSE);

  move(9,9);
  if(hds_old.cam_x!=hds->cam_x){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->cam_x,
	 (int)(hds->cam_x*3600));
  attroff(A_REVERSE);

  move(10,9);
  if(hds_old.cam_y!=hds->cam_y){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->cam_y,
	 (int)(hds->cam_y*3600));
  attroff(A_REVERSE);


  //// Shutter
  move(13,20);
  set_oc2(hds,hds->shutter_shutter, hds_old.shutter_shutter, 
	 hds_old2.shutter_shutter);

  move(14,20);
  set_oc2(hds,hds->shutter_hu, hds_old.shutter_hu, 
	 hds_old2.shutter_hu);
  move(14,25);
  printw("/");
  move(14,26);
  set_oc2(hds,hds->shutter_hl, hds_old.shutter_hl, 
	 hds_old2.shutter_hl);

  
  //// I2-Cell
  move(17,6);
  if(hds_old.mode_i2!=hds->mode_i2){
    attron(A_REVERSE);
  }
  if(hds->mode_i2==1){
    printw("=ON ");
  }
  else{
    printw("=OFF");
  }
  attroff(A_REVERSE);

  move(17,22);
  set_io(hds, hds->option_i2, hds_old.option_i2,
	 hds_old2.option_i2);


  //// Wavelength
  move(2,65);
  printw("            ");
  if((hds_old.setting!=hds->setting)){
    attron(A_REVERSE);
  }
  move(2,65);
  printw("[%s]", setting_name[hds->setting]);
  attroff(A_REVERSE);

  move(3,54);
  if((strcmp(hds_old.slit_filter1,hds->slit_filter1)!=0)||
     (strcmp(hds_old.slit_filter2,hds->slit_filter2)!=0)){
    attron(A_REVERSE);
  }
  printw("%s/%s",hds->slit_filter1,hds->slit_filter2);
  attroff(A_REVERSE);

  move(4,54);
  set_br(hds, hds->col_col, hds_old.col_col,hds_old2.col_col);

  move(5,54);
  if((hds_old.col_echelle!=hds->col_echelle)){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->col_echelle,
	 (int)(hds->col_echelle*3600));
  attroff(A_REVERSE);

  move(6,54);
  set_br(hds, hds->col_cross, hds_old.col_cross,hds_old2.col_cross);

  move(7,54);
  if((hds_old.col_scan!=hds->col_scan)){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->col_scan,
	 (int)(hds->col_scan*3600));
  attroff(A_REVERSE);

  
  //// Temperature
  move(10,60);
  printw("%5.2f / %5.2f[C]", hds->temp_i2i,hds->temp_i2o);

  move(11,60);
  printw("%5.2f / %5.2f[C]", hds->temp_nr1,hds->temp_nr2);

  move(12,60);
  printw("%5.1f[K]", hds->temp_ccd);

  move(9,55);
  printw("(%s)", hds->temp_time);


  //// ImageSlicer
  move(14,56);
  if(hds_old.is_unit!=hds->is_unit){
    attron(A_REVERSE);
  }
  if(hds->is_unit==0){
    printw("None         ");
  }
  else{
    printw("IS#%d (%4.2fx%d)",hds->is_unit,hds->is_width*2.0,hds->is_slic);
  }
  attroff(A_REVERSE);


  //// LightMonitor
  move(17,46);
  if(hds_old.mode_lm!=hds->mode_lm){
    attron(A_REVERSE);
  }
  if(hds->mode_lm==1){
    printw("=ON ");
  }
  else{
    printw("=OFF");
  }
  attroff(A_REVERSE);

  move(17,62);
  set_io(hds, hds->option_lm, hds_old.option_lm,
	 hds_old2.option_lm);


  //// Cover
  move(22,1);
  set_oc2(hds,hds->cover_colb, hds_old.cover_colb, hds_old2.cover_colb);

  move(22,8);
  set_oc2(hds,hds->cover_colr, hds_old.cover_colr, hds_old2.cover_colr); 

  move(22,18);
  set_oc2(hds,hds->cover_crossb, hds_old.cover_crossb, hds_old2.cover_crossb);

  move(22,25);
  set_oc2(hds,hds->cover_crossr, hds_old.cover_crossr, hds_old2.cover_crossr);

  move(22,32);
  set_oc2(hds,hds->cover_mirr, hds_old.cover_mirr, hds_old2.cover_mirr); 

  move(22,42);
  set_oc2(hds,hds->cover_lens1, hds_old.cover_lens1, hds_old2.cover_lens1); 

  move(22,49);
  set_oc2(hds,hds->cover_lens2, hds_old.cover_lens2, hds_old2.cover_lens2); 

  move(22,56);
  set_oc2(hds,hds->cover_lens3, hds_old.cover_lens3, hds_old2.cover_lens3); 

  move(22,66);
  set_oc2(hds,hds->cover_cam1, hds_old.cover_cam1, hds_old2.cover_cam1); 

  move(22,73);
  set_oc2(hds,hds->cover_cam2, hds_old.cover_cam2, hds_old2.cover_cam2);


  // Update time
  attron(A_REVERSE);
  move(23,55);
  printw("%s",hds->update_time);

  // OBS mode
  move(23,10);
  if(hds->mode_obs==1){
    printw("=ON ");
  }
  else{
    printw("=OFF");
  }
  attroff(A_REVERSE);

  move(0,79);

  hds_old2=hds_old;
  hds_old=*hds;
  hds->init_status=TRUE;

  return(0);
}

int main(){
  hds_param hds, *hds_p;

  initscr();

  file_search(&hds); /* 最初の一回 */
  hds_p=&hds;
  hds_p->init_status=FALSE;
  

  title_draw();

  while(1){
    time_func(hds_p);
    value_draw(hds_p);
    refresh();
    sleep(2);
  }
  endwin();
}

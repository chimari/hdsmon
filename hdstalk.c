//    HDS status monitor talking program 
//    - Please edit STATUSDATA in hdstalk.h, which should include
//      the status information of HDS and be refered by this program.
//    - For compile,
//      cc -o hdtalk hdstalk.c `glib-config --cflags` `glib-config --libs` 
//                                           2004.02.21  A.Tajitsu

#define VERSION 0.5

#undef DEBUG
#undef DEBUG2

#include<glib.h>

#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<time.h>
#include<unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include<string.h>


#include"hdstalk.h"    /* hdstalk用設定ヘッダ */


void usage();
void talk_message();
gint time_func();
gint time_func0();
void file_search();
void update_gui();

void  init_hds();
char *get_param();
int   get_status();
int   get_status_is();
int   get_status2();
void  set_oc();
int   get_oc();
int  *get_oc2();
int   get_br();
int   get_io();
int   get_setting();
void  skip_line();
#ifdef DEBUG
void  debug_print();
#endif

char *capiterize();
double absvalue();


gboolean macopix_flag;

// Main
int main(int argc, char* argv[]){
  gint timer;
  hds_param hds, *hds_p;
  char rcfile[BUFFSIZE];
  int i_opt;

  macopix_flag=FALSE;
  
  i_opt = 1;
  while(i_opt < argc) {
    if ((strcmp(argv[i_opt], "-h") == 0) ||
	     (strcmp(argv[i_opt], "--help") == 0)) {
      i_opt++;
      usage();
    }
    else if ((strcmp(argv[i_opt], "-m") == 0) ||
	     (strcmp(argv[i_opt], "--macopix") == 0)) {
      macopix_flag=TRUE;
      i_opt++;
    }

  }

  file_search(&hds); /* 最初の一回 */
  hds_p=&hds;

  hds_p->init_status=FALSE;

  while(1){
    time_func((gpointer)hds_p);
    usleep(INTERVAL*1000);
  }
}

void usage(){
  g_print("usage : hdstalk [-m]\n");
  g_print("    -m : use macopix\n\n");
  exit(0);
}

void talk_message(gchar* str){
  if(macopix_flag)  g_print("#macopix# %s\n",str);
  g_print("#say# %s\n",str);
  usleep(strlen(str)*150*1000);
}

gint time_func(gpointer data){
  struct stat status_stat;
  static time_t up_time;

  if(stat(STATUSDATA,&status_stat)){
    fprintf(stderr,"STATUS read error\n");
  }
  else if (status_stat.st_mtime !=up_time){
    // modified timeが新しいときのみ再読み込み 
#ifdef DEBUG2
      g_print("Status reading...\n");
#endif
    file_search((hds_param *)data);
#ifdef DEBUG2
      g_print("Updating GUI...\n");
#endif
    update_gui((hds_param *)data);
  }
  up_time=status_stat.st_mtime;
}

void update_gui(hds_param *hds){
  gchar *tmp;
  static hds_param hds_old, hds_old2;

  tmp=g_malloc0(sizeof(gchar)*BUFFSIZE);


  if(hds->init_status==FALSE){
    init_hds(&hds_old);
    init_hds(&hds_old2);
  }


  // Update time
  g_print("%s\n",hds->update_time);

  // OBS mode
  if(hds_old.mode_obs!=hds->mode_obs){
    if(hds->mode_obs==1){
      talk_message("OBS mode ON.");
    }
    else{
      talk_message("OBS mode OFF.");
    }
  }

  // Driving
  /*
  if(hds->status_driving==0){
    change_color_bg(hds, hds->w_status_driving);
  }
  else if(hds->status_driving==1){
    change_color(hds, hds->w_status_driving,red);
  }
  else{
    change_color(hds, hds->w_status_driving,gray1);
  }
  */  

  // Slit
  if(hds_old.slit_width!=hds->slit_width){
    sprintf(tmp, "Slit width is %7.2f arcsec.", hds->slit_width);
    talk_message(tmp);
  }

  if(hds_old.slit_length!=hds->slit_length){
    sprintf(tmp, "Slit length is %7.2f arcsec.", hds->slit_length);
    talk_message(tmp);
  }

  // Camera
  if(hds_old.cam_rotate!=hds->cam_rotate){
    sprintf(tmp, "Camera ratation is %5d arcsec.", (int)(hds->cam_rotate*3600));
    talk_message(tmp);
  }

  if(hds_old.cam_z!=hds->cam_z){
    sprintf(tmp, "Camera Z is %7d micron.", (int)(hds->cam_z*1000));
    talk_message(tmp);
  }

  if((hds_old.cam_x!=hds->cam_x)){
    sprintf(tmp, "Camera X is %5d arcsec.", (int)(hds->cam_x*3600));
    talk_message(tmp);
  }

  if((hds_old.cam_y!=hds->cam_y)){
    sprintf(tmp, "Camera Y is %5d arcsec.", (int)(hds->cam_y*3600));
    talk_message(tmp);
  }


  // Wavelength
  if((hds_old.setting!=hds->setting)){
    sprintf(tmp, "Wavelength setting is %s.", setting_talk[hds->setting]);
    talk_message(tmp);
  }

  if((hds_old.col_scan!=hds->col_scan)){
    sprintf(tmp, "Cross disperser scan angle is %5d arcsec.", (int)(hds->col_scan*3600));
    talk_message(tmp);
  }

  if((hds_old.col_echelle!=hds->col_echelle)){
    sprintf(tmp, "Eshelle angle is %5d arcsec.", (int)(hds->col_echelle*3600));
    talk_message(tmp);
  }

  if(strcmp(hds_old.slit_filter1,hds->slit_filter1)!=0){
    sprintf(tmp, "Filter 1 is %s.", hds->slit_filter1);
    talk_message(tmp);
  }

  if(strcmp(hds_old.slit_filter2,hds->slit_filter2)!=0){
    sprintf(tmp, "Filter 2 is %s.", hds->slit_filter2);
    talk_message(tmp);
  }

  if((hds_old.col_col!=hds->col_col)){
    switch(hds->col_col){
    case COL_RED:
      talk_message("Collimator is Red.");
      break;
    case COL_BLUE:
      talk_message("Collimator is Blue.");
      break;
    }
  }

  if((hds_old.col_cross!=hds->col_cross)){
    switch(hds->col_cross){
    case COL_RED:
      talk_message("Cross Disperser is Red.");
      break;
    case COL_BLUE:
      talk_message("Cross Disperser is Blue.");
      break;
    case COL_MIRROR:
      talk_message("Cross Disperser is Mirror.");
      break;
    }
  }



  // I2-Cell
  if(hds_old.mode_i2!=hds->mode_i2){
    if(hds->mode_i2==1){
      talk_message("I2 Cell mode is ON..");
    }
    else{
      talk_message("I2 Cell mode is OFF..");
    }
  }

  if((hds_old.option_i2!=hds->option_i2)){
    if(hds->option_i2==0){
      talk_message("I2 Cell is Out..");
    }
    else if(hds->option_i2==1){
      talk_message("I2 Cell is inserted.");
    }
    else{
      talk_message("I2 Cell position is unknown.");
    }
  }


  // LM
  if(hds_old.mode_lm!=hds->mode_lm){
    if(hds->mode_lm==1){
      talk_message("Light Monitor mode is ON..");
    }
    else{
      talk_message("Light Monitor mode is OFF..");
    }
  }

  if((hds_old.option_lm!=hds->option_lm)){
    if(hds->option_lm==0){
      talk_message("Light Monitor is Out..");
    }
    else if(hds->option_lm==1){
      talk_message("Light Monitor is In.");
    }
    else{
      talk_message("Light Monitor position is unknown.");
    }
  }

  // Image Slicer
  if(hds_old.is_unit!=hds->is_unit){
    if(hds->is_unit==0){
      talk_message("Image Slicer is unused.");
    }
    else{
      sprintf(tmp, "Image Slicer Unit Number %d is equipped.", (int)hds->is_unit);
      talk_message(tmp);
      sprintf(tmp, "Image Slicer Dimension is %4.2lf arcsec times %d.",
	      hds->is_width*2.0,hds->is_slic);
      talk_message(tmp);
    }
  }

  if((hds_old.option_lm!=hds->option_lm)){
    if(hds->option_lm==0){
      talk_message("Light Monitor is Out..");
    }
    else if(hds->option_lm==1){
      talk_message("Light Monitor is In.");
    }
    else{
      talk_message("Light Monitor position is unknown.");
    }
  }


  // Shutter
  set_oc(hds->shutter_shutter, hds_old.shutter_shutter, "Main Shutter"); 

  set_oc(hds->shutter_hu, hds_old.shutter_hu, "Upper Haltmann Shutter");
  set_oc(hds->shutter_hl, hds_old.shutter_hu, "Lower Haltmann Shutter");


  // Cover
  set_oc(hds->cover_colb, hds_old.cover_colb, "Blue Collimator Cover");
  set_oc(hds->cover_colr, hds_old.cover_colr, "Red Collimator Cover");
  set_oc(hds->cover_crossb, hds_old.cover_crossb, "Blue Cross Cover");
  set_oc(hds->cover_crossr, hds_old.cover_crossr, "Red Cross Cover");
  set_oc(hds->cover_mirr, hds_old.cover_mirr, "Mirror Cross Cover");
  set_oc(hds->cover_lens1, hds_old.cover_lens1, "First Lens Cover");
  set_oc(hds->cover_lens2, hds_old.cover_lens2, "Second Lens Cover");
  set_oc(hds->cover_lens3, hds_old.cover_lens3, "Third Lens Cover");
  set_oc(hds->cover_cam1, hds_old.cover_cam1, "First Camera Cover");
  set_oc(hds->cover_cam2, hds_old.cover_cam2, "Second Camera Cover");

  g_free(tmp);

  hds_old2=hds_old;
  hds_old=*hds;
  hds->init_status=TRUE;
  return;
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
  

int get_setting(hds_param *hds){
  if((hds->col_col==1)&&(hds->col_cross==1)){ // BLUE
    if(absvalue((double)(hds->col_scan-D_CROSS-4.95))<E_SCAN){       //StdUa
      return (STDUA);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-4.75))<E_SCAN){  //StdUb
      return (STDUB);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-5.35))<E_SCAN){  //StdBa
      return (STDBA);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-5.526))<E_SCAN){ //StdBc
      return (STDBC);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-6.19))<E_SCAN){  //StdYa
      return (STDYA);
    }
    else{
      return (NOSTD);
    }
  }
  else if((hds->col_col==0)&&(hds->col_cross==0)){ //RED
    if(absvalue((double)(hds->col_scan-D_CROSS-4.37))<E_SCAN){       //StdYb
      return (STDYB);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-4.583))<E_SCAN){  //StdYc
      return (STDYC);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-4.300))<E_SCAN){  //StdYd
      return (STDYD);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-5.126))<E_SCAN){  //StdRa
      return (STDRA);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-5.30))<E_SCAN){ //StdRb
      return (STDRB);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-7.00))<E_SCAN){  //StdNIRa
      return (STDNIRA);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-6.35))<E_SCAN){  //StdNIRb
      return (STDNIRB);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-5.933))<E_SCAN){  //StdNIRc
      return (STDNIRC);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-5.00))<E_SCAN){  //StdI2a
      return (STDI2A);
    }
    else if(absvalue((double)(hds->col_scan-D_CROSS-3.90))<E_SCAN){  //StdI2b
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


void set_oc(int oc_status, int oc_old, gchar* name)
{
  gchar tmp[BUFFSIZE];

  if(oc_status!=oc_old){
    if(oc_status==0){
      sprintf(tmp, "%s Close", name);
      talk_message(tmp);
    }
    else if(oc_status==1){
      sprintf(tmp, "%s Open", name);
      talk_message(tmp);
    }
    else{
      sprintf(tmp, "%s status is unknown", name);
      talk_message(tmp);
    }
  }

  return;
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
    g_print("!!! DATA reading error...\n");
    return;
  }
  
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
				       



void debug_print(hds_param *hds){
#ifdef DEBUG
  printf("Status obtained = %s\n",hds->update_time);

  if(hds->status_driving==1){
    printf("Driving         = Yes\n");
  }
  else if(hds->status_driving==0){
    printf("Driving         = No\n");
  }
  else if(hds->status_driving==-1){
    printf("Driving         = Unknown\n");
  }


  if(hds->mode_obs==1){
    printf("OBS-MODE        = Yes\n");
  }
  else if(hds->mode_obs==0){
    printf("OBS-MODE        = No\n");
  }
  else if(hds->mode_obs==-1){
    printf("OBS-MODE        = Unknown\n");
  }

  if(hds->mode_i2==1){
    printf("I2-CELL         = Yes\n");
  }
  else if(hds->mode_i2==0){
    printf("I2-CELL         = No\n");
  }
  else if(hds->mode_i2==-1){
    printf("I2-CELL         = Unknown\n");
  }

  if(hds->mode_lm==1){
    printf("Light Monitor   = Yes\n");
  }
  else if(hds->mode_lm==0){
    printf("Light Monitor   = No\n");
  }
  else if(hds->mode_lm==-1){
    printf("Light Monitor   = Unknown\n");
  }


  printf("Slit Width      =  %7.5f[arcsec]/ %7.5f[mm]\n",
	 hds->slit_width,hds->slit_width/2);
  printf("Slit Length     = %8.5f[arcsec]/%8.5f[mm]\n",
	 hds->slit_length,hds->slit_length/2);
  printf("Filter1         = %s\n",hds->slit_filter1);
  printf("Filter2         = %s\n",hds->slit_filter2);

  if(hds->shutter_shutter==1){
    printf("SHUTTER         = Open\n");
  }
  else if(hds->shutter_shutter==0){
    printf("SHUTTER         = Close\n");
  }
  else if(hds->shutter_shutter==-1){
    printf("SHUTTER         = Unknown\n");
  }

  if(hds->shutter_hu==1){
    printf("Hartman Upper   = Open\n");
  }
  else if(hds->shutter_hu==0){
    printf("Hartman Upper   = Close\n");
  }
  else if(hds->shutter_hu==-1){
    printf("Hartman Upper   = Unknown\n");
  }

  if(hds->shutter_hl==1){
    printf("Hartman Lower   = Open\n");
  }
  else if(hds->shutter_hl==0){
    printf("Hartman Lower   = Close\n");
  }
  else if(hds->shutter_hl==-1){
    printf("Hartman Lower   = Unknown\n");
  }


  if(hds->col_col==1){
    printf("Collimator      = Blue\n");
  }
  else if(hds->col_col==0){
    printf("Collimator      = Red\n");
  }
  else if(hds->col_col==-1){
    printf("Collimator      = Unknown\n");
  }

  if(hds->col_cross==1){
    printf("Cross Disperser = Blue\n");
  }
  else if(hds->col_cross==0){
    printf("Cross Disperser = Red\n");
  }
  else if(hds->col_cross==2){
    printf("Cross Disperser = Mirror\n");
  }
  else if(hds->col_cross==-1){
    printf("Cross Disperser = Unknown\n");
  }

  printf("Echelle         = %8.5f[deg]/%6d[sec]\n",
	 hds->col_echelle,(int)(hds->col_echelle*3600));
  printf("Cross Scan      = %8.5f[deg]/%6d[sec]\n",
	 hds->col_scan,(int)(hds->col_scan*3600));

  printf("Camera Rotate   = %8.5f[deg]/%6d[sec]\n",
	 hds->cam_rotate,(int)(hds->cam_rotate*3600));
  printf("Camera Z        = %8.5f[mm]\n",hds->cam_z);
  printf("Camera X        = %8.5f[deg]/%6d[sec]\n",
	 hds->cam_x,(int)(hds->cam_x*3600));
  printf("Camera Y        = %8.5f[deg]/%6d[sec]\n",
	 hds->cam_y,(int)(hds->cam_y*3600));


  if(hds->cover_lens1==1){
    printf("Cover Lens1     = Open\n");
  }
  else if(hds->cover_lens1==0){
    printf("Cover Lens1     = Close\n");
  }
  else if(hds->cover_lens1==-1){
    printf("Cover Lens1     = Unknown\n");
  }

  if(hds->cover_lens2==1){
    printf("Cover Lens2     = Open\n");
  }
  else if(hds->cover_lens2==0){
    printf("Cover Lens2     = Close\n");
  }
  else if(hds->cover_lens2==-1){
    printf("Cover Lens2     = Unknown\n");
  }

  if(hds->cover_lens3==1){
    printf("Cover Lens3     = Open\n");
  }
  else if(hds->cover_lens3==0){
    printf("Cover Lens3     = Close\n");
  }
  else if(hds->cover_lens3==-1){
    printf("Cover Lens3     = Unknown\n");
  }

  if(hds->cover_cam1==1){
    printf("Cover Cam1      = Open\n");
  }
  else if(hds->cover_cam1==0){
    printf("Cover Cam1      = Close\n");
  }
  else if(hds->cover_cam1==-1){
    printf("Cover Cam1      = Unknown\n");
  }

  if(hds->cover_cam2==1){
    printf("Cover Cam2      = Open\n");
  }
  else if(hds->cover_cam2==0){
    printf("Cover Cam2      = Close\n");
  }
  else if(hds->cover_cam2==-1){
    printf("Cover Cam2      = Unknown\n");
  }

  if(hds->cover_colr==1){
    printf("Cover Col. Red  = Open\n");
  }
  else if(hds->cover_colr==0){
    printf("Cover Col. Red  = Close\n");
  }
  else if(hds->cover_colr==-1){
    printf("Cover Col. Red  = Unknown\n");
  }

  if(hds->cover_colb==1){
    printf("Cover Col. Blue = Open\n");
  }
  else if(hds->cover_colb==0){
    printf("Cover Col. Blue = Close\n");
  }
  else if(hds->cover_colb==-1){
    printf("Cover Col. Blue = Unknown\n");
  }

  if(hds->cover_crossr==1){
    printf("Cover Cross R   = Open\n");
  }
  else if(hds->cover_crossr==0){
    printf("Cover Cross R   = Close\n");
  }
  else if(hds->cover_crossr==-1){
    printf("Cover Cross R   = Unknown\n");
  }

  if(hds->cover_crossb==1){
    printf("Cover Cross B   = Open\n");
  }
  else if(hds->cover_crossb==0){
    printf("Cover Cross B   = Close\n");
  }
  else if(hds->cover_crossb==-1){
    printf("Cover Cross B   = Unknown\n");
  }

  if(hds->cover_mirr==1){
    printf("Cover Cross M   = Open\n");
  }
  else if(hds->cover_mirr==0){
    printf("Cover Cross M   = Close\n");
  }
  else if(hds->cover_mirr==-1){
    printf("Cover Cross M   = Unknown\n");
  }


  if(hds->option_i2==1){
    printf("I2-Cell         = In\n");
  }
  else if(hds->option_i2==0){
    printf("I2-Cell         = Out\n");
  }
  else if(hds->option_i2==-1){
    printf("I2-Cell         = Unknown\n");
  }

  if(hds->option_lm==1){
    printf("Light Monitor   = In\n");
  }
  else if(hds->option_lm==0){
    printf("Light Monitor   = Out\n");
  }
  else if(hds->option_lm==-1){
    printf("Light Monitor   = Unknown\n");
  }

  printf("Temp I2-inside    = %6.3f[deg C]\n",hds->temp_i2i);
  printf("Temp I2-outside   = %6.3f[deg C]\n",hds->temp_i2o);
  printf("Temp Nasmyth1     = %6.3f[deg C]\n",hds->temp_nr1);
  printf("Temp Nasmyth2     = %6.3f[deg C]\n",hds->temp_nr2);
  printf("Temp CCD          = %5.1f[K]\n",hds->temp_ccd);
  printf("CCD Temp Time     = %s\n",hds->temp_time);

#endif
}

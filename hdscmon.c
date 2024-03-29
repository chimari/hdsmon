#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<time.h>
#include<unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include<curses.h>
#include<math.h>

#include"hdscmon.h"

gint time_func(hds_param *hds){
  struct stat status_stat;
  static time_t up_time;

  if(stat(STATUSDATA,&status_stat)){
    // fprintf(stderr,"STATUS read error\n");
  }
  else if (status_stat.st_mtime !=up_time){
    // modified time$B$,?7$7$$$H$-$N$_:FFI$_9~$_(B 
    file_search(hds);
  }
  up_time=status_stat.st_mtime;

}

void param_read(hds_param *hds){
  FILE *fp_read;
  gint i_set;
  gchar buf[BUFFSIZE];
  gchar *sp, *ret;

  // Default value
  for(i_set=0;i_set<NUM_SET; i_set++){
    hds->cross_val[i_set]=orig_cross_val[i_set];
  }

  hds->dcr_b=0;
  hds->dcr_r=0;
  hds->ec0_b=900;
  hds->ec0_r=900;
  hds->camz_b=g_strdup("Cam Z (B)");
  hds->camz_r=g_strdup("Cam Z (R)");

  // Scan HDSparam.dat
  if((fp_read=fopen(HDSPARAM,"r"))==NULL){
    fprintf(stderr,"HDSPARAM read error\n");
    return;
  }

  for(i_set=1;i_set<NUM_SET; i_set++){
    sp=g_strdup_printf(":StdSetup_%s:", setting_name[i_set]);

    fseek(fp_read, 0, SEEK_SET);
    while(!feof(fp_read)){
      fgets(buf,BUFFSIZE-1,fp_read);
      ret=strstr(buf,sp);
      if(ret){
	hds->cross_val[i_set]=g_strtod(strtok(ret+strlen(sp),":"), NULL);
	//printf("%s : %d\n",
	//      setting_name[i_set],
	//     hds->cross_val[i_set]);
	break;
      }
    }

    g_free(sp);
  }

  fseek(fp_read, 0, SEEK_SET);
  sp=g_strdup("#dCross_B:");
  while(!feof(fp_read)){
    fgets(buf,BUFFSIZE-1,fp_read);
    ret=strstr(buf,sp);
    if(ret){
      hds->dcr_b=g_strtod(strtok(ret+strlen(sp),":"), NULL);
      // printf("dCross_B : %d\n",
      //	     hds->dcr_b);
      break;
    }
  }
  g_free(sp);

  fseek(fp_read, 0, SEEK_SET);
  sp=g_strdup("#dCross_R:");
  while(!feof(fp_read)){
    fgets(buf,BUFFSIZE-1,fp_read);
    ret=strstr(buf,sp);
    if(ret){
      hds->dcr_r=g_strtod(strtok(ret+strlen(sp),":"), NULL);
      //printf("dCross_R : %d\n",
      //	     hds->dcr_r);
      break;
    }
  }
  g_free(sp);

  fseek(fp_read, 0, SEEK_SET);
  sp=g_strdup("#Echelle0_B:");
  while(!feof(fp_read)){
    fgets(buf,BUFFSIZE-1,fp_read);
    ret=strstr(buf,sp);
    if(ret){
      hds->ec0_b=g_strtod(strtok(ret+strlen(sp),":"), NULL);
      //printf("Echelle0_B : %d\n",
      //	     hds->ec0_b);
      break;
    }
  }
  g_free(sp);

  fseek(fp_read, 0, SEEK_SET);
  sp=g_strdup("#Echelle0_R:");
  while(!feof(fp_read)){
    fgets(buf,BUFFSIZE-1,fp_read);
    ret=strstr(buf,sp);
    if(ret){
      hds->ec0_r=g_strtod(strtok(ret+strlen(sp),":"), NULL);
      // printf("Echelle0_R : %d\n",
      //	     hds->ec0_r);
      break;
    }
  }
  g_free(sp);

  fseek(fp_read, 0, SEEK_SET);
  sp=g_strdup("#CamZ_B:");
  while(!feof(fp_read)){
    fgets(buf,BUFFSIZE-1,fp_read);
    ret=strstr(buf,sp);
    if(ret){
      if(hds->camz_b) g_free(hds->camz_b);
      hds->camz_b=g_strdup(strtok(ret+strlen(sp),":"));
      break;
    }
  }
  g_free(sp);

  fseek(fp_read, 0, SEEK_SET);
  sp=g_strdup("#CamZ_R:");
  while(!feof(fp_read)){
    fgets(buf,BUFFSIZE-1,fp_read);
    ret=strstr(buf,sp);
    if(ret){
      if(hds->camz_r) g_free(hds->camz_r);
      hds->camz_b=g_strdup(strtok(ret+strlen(sp),":"));
      break;
    }
  }
  g_free(sp);


  fclose(fp_read);
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
  /* $B99?7F|;~(B */
  strcpy(hds->update_time,get_param(buf[read_line]," = "));
  read_line++;
  /* Driving */
  hds->status_driving=get_status(buf[read_line]," = ");
  read_line++;

#ifdef DEBUG2
  g_print("   reading mode\n");
#endif

  /*** $B%b!<%I(B ***/
  read_line++;
  /* OBS-MODE */
  hds->now.mode_obs=get_status(buf[read_line]," = ");
  read_line++;
  /* I2-CELL */
  hds->now.mode_i2=get_status2(buf[read_line]," = ");
  read_line++;
  /* Light Monitor */
  hds->now.mode_lm=get_status2(buf[read_line]," = ");
  read_line++;


#ifdef DEBUG2
  g_print("   reading slit\n");
#endif

  /*** SLIT ***/
  read_line++;
  /* Slit Width */
  strcpy(c_tmp,get_param(buf[read_line],"= "));
  read_line++;
  sscanf(c_tmp,"%f",&hds->now.slit_width);
  /* Slit Length */
  strcpy(c_tmp,get_param(buf[read_line],"= "));
  read_line++;
  sscanf(c_tmp,"%f",&hds->now.slit_length);
  /* Filer1 */
  strcpy(hds->now.slit_filter1,get_param(buf[read_line]," = "));
  read_line++;
  /* Filer2 */
  strcpy(hds->now.slit_filter2,get_param(buf[read_line]," = "));
  read_line++;

#ifdef DEBUG2
  g_print("   reading shutter\n");
#endif

  /*** SHUTTER ***/
  read_line++;
  /* Shutter */
  hds->now.shutter_shutter=get_oc(buf[read_line]," = ");
  read_line++;
  /* Hartman Upper */
  hds->now.shutter_hu=get_oc(buf[read_line]," = ");
  read_line++;
  /* Hartman Lower */
  hds->now.shutter_hl=get_oc(buf[read_line]," = ");
  read_line++;


#ifdef DEBUG2
  g_print("   reading collimator/grating\n");
#endif

  /*** Collimator/Graing ***/
  read_line++;
  /* Collimator */
  hds->now.col_col=get_br(buf[read_line]," = ");
  read_line++;
  /* Cross Disperser */
  hds->now.col_cross=get_br(buf[read_line]," = ");
  read_line++;
  /* Echelle */
  hds->now.col_echelle=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* Cross Scan */
  hds->now.col_scan=atof(get_param(buf[read_line]," = "));
  read_line++;
  
#ifdef DEBUG2
  g_print("   reading detector\n");
#endif

  /*** Detector ***/
  read_line++;
  /* Rotate */
  hds->now.cam_rotate=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* Z */
  hds->now.cam_z=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* X */
  hds->now.cam_x=atof(get_param(buf[read_line]," = "));
  read_line++;
  /* Y */
  hds->now.cam_y=atof(get_param(buf[read_line]," = "));
  read_line++;


#ifdef DEBUG2
  g_print("   reading cover\n");
#endif

  /*** Cover ***/
  read_line++;
  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->now.cover_lens1=cover_tmp[0];
  hds->now.cover_colr=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->now.cover_lens2=cover_tmp[0];
  hds->now.cover_colb=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->now.cover_lens3=cover_tmp[0];
  hds->now.cover_crossr=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->now.cover_cam1=cover_tmp[0];
  hds->now.cover_crossb=cover_tmp[1];

  cover_tmp=get_oc2(buf[read_line],"= ");
  read_line++;
  hds->now.cover_cam2=cover_tmp[0];
  hds->now.cover_mirr=cover_tmp[1];

#ifdef DEBUG2
  g_print("   reading option\n");
#endif

  /*** Option ***/
  read_line++;
  hds->now.option_i2=get_io(buf[read_line]," = ");
  read_line++;
  hds->now.option_lm=get_io(buf[read_line]," = ");
  read_line++;
  hds->now.temp_i2i=atof(get_param(buf[read_line],"= "));
  read_line++;
  hds->now.temp_i2o=atof(get_param(buf[read_line],"= "));
  read_line++;

#ifdef DEBUG2
  g_print("   reading cryostadt\n");
#endif

  /*** Cryostadt ***/
  read_line++;
  strcpy(hds->temp_time,get_param(buf[read_line]," = "));
  read_line++;
  hds->now.temp_ccd=atof(get_param(buf[read_line],"= "))/10;
  read_line++;

#ifdef DEBUG2
  g_print("   reading temp\n");
#endif

  /*** Nasmyth Room Temperature ***/
  read_line++;
  hds->now.temp_nr1=atof(get_param(buf[read_line],"= "));
  read_line++;
  hds->now.temp_nr2=atof(get_param(buf[read_line],"= "));
  read_line++;

  /* Image Slicer */
  read_line++;
  hds->now.is_unit=get_status_is(buf[read_line]," = ");
  read_line++;
  hds->now.is_width=atof(get_param(buf[read_line]," = "));
  read_line++;
  hds->now.is_slic=get_status_is(buf[read_line]," = ");
  read_line++;

  /*** Setting ***/
  hds->now.setting=get_setting(hds);

  
#ifdef DEBUG
  debug_print(hds);
#endif

#ifdef DEBUG2
  g_print("   closing\n");
#endif

  return;
}


int get_setting(hds_param *hds){
  gint i_set;
  gint scan_val=(gint)(hds->now.col_scan*3600.);
  
  if((hds->now.col_col==1)&&(hds->now.col_cross==1)){ // BLUE
    for(i_set=STDUB; i_set<=STDYA; i_set++){
      if(absvalue(scan_val-hds->cross_val[i_set])<E_SCAN){
	return (i_set);
      }
    }
    return (NOSTD);
  }
  else if((hds->now.col_col==0)&&(hds->now.col_cross==0)){ //RED
    for(i_set=STDI2B; i_set<=STDNIRA; i_set++){
      if(absvalue(scan_val-hds->cross_val[i_set])<E_SCAN){
	return (i_set);
      }
    }
    return (NOSTD);
  }
  else{
    return (NOSTD);
  }
}


gint absvalue(gint ip){
  if(ip<0){
    return(-ip);
  }
  else{
    return(ip);
  }
}

/* $B%Q%i%a!<%?(B $B%j!<%I4X?t(B */
char *get_param(char * buf , char *token){
  static char b[BUFFSIZE];
  char *c=NULL;

  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  return(b);
}
  

/* status $B%Q%i%a!<%?(B $B%j!<%I4X?t(B for ImgSlicer  1:IS#1 0:NOUSE */
int get_status_is(char *buf,  char *token){
  static char b[BUFFSIZE];
  char *c=NULL;
  c=(char *)strstr(buf,token);
  c+=strlen(token);
  if(c[strlen(c)-1]=='\n') c[strlen(c)-1]='\0';
  strcpy(b,c);
  return((int)atoi(b));
}

/* status $B%Q%i%a!<%?(B $B%j!<%I4X?t(B  1:Yes 0:No -1:UNKNOWN */
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

/* status $B%Q%i%a!<%?(B $B%j!<%I4X?t$=$N(B2  1:1 0:0 -1:UNKNOWN */
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

/* OPEN/CLOSE $B%Q%i%a!<%?(B $B%j!<%I4X?t(B  1:OPEN 0:CLOSE -1:UNKNOWN */
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

/* OPEN/CLOSE $B%Q%i%a!<%?(B $B%j!<%I4X?t$=$K$g(B2  1:OPEN 0:CLOSE -1:UNKNOWN */
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

/* BLUE/RED $B%Q%i%a!<%?(B $B%j!<%I4X?t(B  2:MIRROR 1:BLUE 0:RED -1:UNKNOWN */
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


/* IN/OUT $B%Q%i%a!<%?(B $B%j!<%I4X?t(B  1:IN 0:OUT -1:UNKNOWN */
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


/* $BJ8;zNs$NBgJ8;z2=(B */
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

void init_hds(stat_param *stat){
  stat->slit_width=-9999;
  stat->slit_length=-9999;

  stat->cam_rotate=-9999;
  stat->cam_x=-9999;
  stat->cam_y=-9999;
  stat->cam_z=-9999;

  stat->shutter_shutter=-9999;
  stat->shutter_hu=-9999;
  stat->shutter_hl=-9999;

  stat->col_col=-9999;
  stat->col_echelle=-9999;
  stat->col_cross=-9999;
  stat->col_scan=-9999;

  stat->temp_i2o=-9999;
  stat->temp_i2i=-9999;
  stat->temp_nr1=-9999;
  stat->temp_nr2=-9999;
  stat->temp_ccd=-9999;

  stat->option_i2=-9999;
  stat->option_lm=-9999;

  stat->is_unit=-9999;
  stat->is_slic=-9999;
  stat->is_width=-9999;

  stat->cover_colb=-9999;
  stat->cover_colr=-9999;
  stat->cover_crossb=-9999;
  stat->cover_crossr=-9999;
  stat->cover_mirr=-9999;
  stat->cover_lens1=-9999;
  stat->cover_lens2=-9999;
  stat->cover_lens3=-9999;
  stat->cover_cam1=-9999;
  stat->cover_cam2=-9999;

  stat->setting=-9999;

  strcpy(stat->slit_filter1, "???");
  strcpy(stat->slit_filter2, "???");

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
  char* tmp[256];


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
  if(fabs(hds->old1.slit_width-hds->now.slit_width)>0.0001){
    attron(A_REVERSE);
  }
  printw("[R=%6d]", (int)(3.6e4/hds->now.slit_width));
  attroff(A_REVERSE);

  move(3,9);
  if(fabs(hds->old1.slit_width-hds->now.slit_width)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[\"] = %5d[um]", hds->now.slit_width, 
	 (int)(hds->now.slit_width/2*1e3));
  attroff(A_REVERSE);

  move(4,9);
  if(fabs(hds->old1.slit_length-hds->now.slit_length)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[\"] = %5d[um]", hds->now.slit_length, 
	 (int)(hds->now.slit_length/2*1e3));
  attroff(A_REVERSE);


  //// Camera
  move(7,9);
  if(fabs(hds->old1.cam_rotate-hds->now.cam_rotate)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[um]", hds->now.cam_rotate,
	 (int)(hds->now.cam_rotate*3600));
  attroff(A_REVERSE);

  move(8,9);
  if(fabs(hds->old1.cam_z-hds->now.cam_z)>0.0001){
    attron(A_REVERSE);
  }
  printw("%+d[um]", (int)(hds->now.cam_z*1000));
  attroff(A_REVERSE);

  move(9,9);
  if(fabs(hds->old1.cam_x-hds->now.cam_x)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->now.cam_x,
	 (int)(hds->now.cam_x*3600));
  attroff(A_REVERSE);

  move(10,9);
  if(fabs(hds->old1.cam_y-hds->now.cam_y)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->now.cam_y,
	 (int)(hds->now.cam_y*3600));
  attroff(A_REVERSE);


  //// Shutter
  move(13,20);
  set_oc2(hds,hds->now.shutter_shutter, hds->old1.shutter_shutter, 
	 hds->old2.shutter_shutter);

  move(14,20);
  set_oc2(hds,hds->now.shutter_hu, hds->old1.shutter_hu, 
	 hds->old2.shutter_hu);
  move(14,25);
  printw("/");
  move(14,26);
  set_oc2(hds,hds->now.shutter_hl, hds->old1.shutter_hl, 
	 hds->old2.shutter_hl);

  
  //// I2-Cell
  move(17,6);
  if(fabs(hds->old1.mode_i2-hds->now.mode_i2)>0.0001){
    attron(A_REVERSE);
  }
  if(hds->now.mode_i2==1){
    printw("=ON ");
  }
  else{
    printw("=OFF");
  }
  attroff(A_REVERSE);

  move(17,22);
  set_io(hds, hds->now.option_i2, hds->old1.option_i2,
	 hds->old2.option_i2);


  //// Wavelength
  move(2,65);
  printw("            ");
  if((hds->old1.setting!=hds->now.setting)){
    attron(A_REVERSE);
  }
  move(2,65);
  printw("           ");
  move(2,65);
  if(hds->now.setting==0){
    printw("[Non-Std]");
  }
  else{
    printw("[Std%s]", setting_name[hds->now.setting]);
  }
  attroff(A_REVERSE);

  move(3,54);
  printw("                ");
  move(3,54);
  if((strcmp(hds->old1.slit_filter1,hds->now.slit_filter1)!=0)||
     (strcmp(hds->old1.slit_filter2,hds->now.slit_filter2)!=0)){
    attron(A_REVERSE);
  }
  printw("%s/%s",hds->now.slit_filter1,hds->now.slit_filter2);
  attroff(A_REVERSE);

  move(4,54);
  set_br(hds, hds->now.col_col, hds->old1.col_col,hds->old2.col_col);

  move(4,60);
  if((strncmp(hds->now.slit_filter1, "FREE", 4)==0)&&
     (strncmp(hds->now.slit_filter2, "FREE", 4)==0)){
    printw("(==> %+5.3lfV)",ideal_colv0[hds->now.is_unit]);
  }
  else{
    printw("(==> %+5.3lfV)",ideal_colv1[hds->now.is_unit]);
  }

  move(5,54);
  if(fabs(hds->old1.col_echelle-hds->now.col_echelle)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->now.col_echelle,
	 (int)(hds->now.col_echelle*3600));
  attroff(A_REVERSE);

  move(6,54);
  set_br(hds, hds->now.col_cross, hds->old1.col_cross,hds->old2.col_cross);

  move(7,54);
  if(fabs(hds->old1.col_scan-hds->now.col_scan)>0.0001){
    attron(A_REVERSE);
  }
  printw("%7.4f[deg] = %5d[sec]", hds->now.col_scan,
	 (int)(hds->now.col_scan*3600));
  attroff(A_REVERSE);

  
  //// Temperature
  move(10,60);
  printw("%5.2f / %5.2f[C]", hds->now.temp_i2i,hds->now.temp_i2o);

  move(11,60);
  printw("%5.2f / %5.2f[C]", hds->now.temp_nr1,hds->now.temp_nr2);

  move(12,60);
  printw("%5.1f[K]", hds->now.temp_ccd);

  move(9,55);
  printw("(%s)", hds->temp_time);


  //// ImageSlicer
  move(14,56);
  if(hds->old1.is_unit!=hds->now.is_unit){
    attron(A_REVERSE);
  }
  if(hds->now.is_unit==0){
    printw("None         ");
  }
  else{
    printw("IS#%d (%4.2fx%d)",hds->now.is_unit,hds->now.is_width*2.0,hds->now.is_slic);
  }
  attroff(A_REVERSE);


  //// LightMonitor
  move(17,46);
  if(hds->old1.mode_lm!=hds->now.mode_lm){
    attron(A_REVERSE);
  }
  if(hds->now.mode_lm==1){
    printw("=ON ");
  }
  else{
    printw("=OFF");
  }
  attroff(A_REVERSE);

  move(17,62);
  set_io(hds, hds->now.option_lm, hds->old1.option_lm,
	 hds->old2.option_lm);


  //// Cover
  move(22,1);
  set_oc2(hds,hds->now.cover_colb, hds->old1.cover_colb, hds->old2.cover_colb);

  move(22,8);
  set_oc2(hds,hds->now.cover_colr, hds->old1.cover_colr, hds->old2.cover_colr); 

  move(22,18);
  set_oc2(hds,hds->now.cover_crossb, hds->old1.cover_crossb, hds->old2.cover_crossb);

  move(22,25);
  set_oc2(hds,hds->now.cover_crossr, hds->old1.cover_crossr, hds->old2.cover_crossr);

  move(22,32);
  set_oc2(hds,hds->now.cover_mirr, hds->old1.cover_mirr, hds->old2.cover_mirr); 

  move(22,42);
  set_oc2(hds,hds->now.cover_lens1, hds->old1.cover_lens1, hds->old2.cover_lens1); 

  move(22,49);
  set_oc2(hds,hds->now.cover_lens2, hds->old1.cover_lens2, hds->old2.cover_lens2); 

  move(22,56);
  set_oc2(hds,hds->now.cover_lens3, hds->old1.cover_lens3, hds->old2.cover_lens3); 

  move(22,66);
  set_oc2(hds,hds->now.cover_cam1, hds->old1.cover_cam1, hds->old2.cover_cam1); 

  move(22,73);
  set_oc2(hds,hds->now.cover_cam2, hds->old1.cover_cam2, hds->old2.cover_cam2);


  // Update time
  attron(A_REVERSE);
  move(23,55);
  printw("%s",hds->update_time);

  // OBS mode
  move(23,10);
  if(hds->now.mode_obs==1){
    printw("=ON ");
  }
  else{
    printw("=OFF");
  }
  attroff(A_REVERSE);

  move(0,79);

  hds->old2=hds->old1;
  hds->old1=hds->now;

  hds->init_status=TRUE;

  return(0);
}

int main(){
  hds_param *hds;

  initscr();

  hds=g_malloc0(sizeof(hds_param));

  param_read(hds);

  init_hds(&hds->old1);
  init_hds(&hds->old2);
  file_search(hds); /* $B:G=i$N0l2s(B */
  hds->init_status=FALSE;

  title_draw();

  while(1){
    time_func(hds);
    value_draw(hds);
    refresh();
    sleep(2);
  }
  endwin();
}

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
#include<locale.h>
#include<math.h>


#include"hdstalk.h"    /* hdsjtalk用設定ヘッダ */

gchar *setting_name_j[NUM_SET]={
  "ノンスタンダード", 
  "スタンダード UB", 
  "スタンダード UA", 
  "スタンダード BA", 
  "スタンダード BC", 
  "スタンダード YA", 
  "スタンダード IツーB",
  "スタンダード YD", 
  "スタンダード YB",
  "スタンダード YC", 
  "スタンダード IツーA", 
  "スタンダード RA", 
  "スタンダード RB", 
  "スタンダード NIRC",
  "スタンダード NIRB", 
  "スタンダード NIRA"
};


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
gint absvalue();


gboolean macopix_flag;

void usage(){
  g_print("usage : hdstalk [-m]\n");
  g_print("    -m : use macopix\n\n");
  exit(0);
}

void talk_message(gchar* str){
  if(macopix_flag)  g_print("#macopix# %s\n",str);
  g_print("#say#%s\n",str);
  usleep(strlen(str)*150*1000);
}

gint time_func(hds_param *hds){
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
    file_search(hds);
#ifdef DEBUG2
      g_print("Updating GUI...\n");
#endif
    update_gui(hds);
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
	//     setting_name[i_set],
	//   hds->cross_val[i_set]);
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


void update_gui(hds_param *hds){
  gchar tmp[BUFFSIZE];

  // Update time
  g_print("%s\n",hds->update_time);

  // OBS mode
  if(hds->old1.mode_obs!=hds->now.mode_obs){
    if(hds->now.mode_obs==1){
      talk_message("OBSモードをオンにしました");
    }
    else{
      talk_message("OBSモードをオフにしました");
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
  if(fabs(hds->old1.slit_width-hds->now.slit_width)>0.0001){
    sprintf(tmp, "スリット幅を %7.2f秒に変更しました", hds->now.slit_width);
    talk_message(tmp);
  }

  if(fabs(hds->old1.slit_length-hds->now.slit_length)>0.0001){
    sprintf(tmp, "スリット長を %7.2f秒に変更しました", hds->now.slit_length);
    talk_message(tmp);
  }

  // Camera
  if(fabs(hds->old1.cam_rotate-hds->now.cam_rotate)>0.0001){
    if(hds->now.cam_rotate<0){
      sprintf(tmp, "カメラの回転角度を マイナス%5d秒に変更しました", -(int)(hds->now.cam_rotate*3600));
    }
    else{
      sprintf(tmp, "カメラの回転角度を %5d秒に変更しました", (int)(hds->now.cam_rotate*3600));
    }
    talk_message(tmp);
  }

  if(fabs(hds->old1.cam_z-hds->now.cam_z)>0.0001){
    if(hds->now.cam_z<0){
      sprintf(tmp, "カメラ Z を マイナス%7dミクロンに変更しました", -(int)(hds->now.cam_z*1000));
    }
    else{
      sprintf(tmp, "カメラ Z を %7dミクロンに変更しました", (int)(hds->now.cam_z*1000));
    }
    talk_message(tmp);
  }

  if(fabs(hds->old1.cam_x-hds->now.cam_x)>0.0001){
    if(hds->now.cam_x<0){
      sprintf(tmp, "カメラ X を マイナス%5d秒に変更しました", -(int)(hds->now.cam_x*3600));
    }
    else{
      sprintf(tmp, "カメラ X を %5d秒に変更しました", (int)(hds->now.cam_x*3600));
    }
    talk_message(tmp);
  }

  if(fabs(hds->old1.cam_y-hds->now.cam_y)>0.0001){
    if(hds->now.cam_y<0){
      sprintf(tmp, "カメラ Y を マイナス%5d秒に変更しました", -(int)(hds->now.cam_y*3600));
    }
    else{
      sprintf(tmp, "カメラ Y を %5d秒に変更しました", (int)(hds->now.cam_y*3600));
    }
    talk_message(tmp);
  }


  // Wavelength
  if((hds->old1.setting!=hds->now.setting)){
    sprintf(tmp, "波長設定を %sに変更しました", setting_name_j[hds->now.setting]);
    talk_message(tmp);
  }

  if(fabs(hds->old1.col_scan-hds->now.col_scan)>0.0001){
    if(hds->now.col_scan<0){
      sprintf(tmp, "クロスディスパーザーのスキャン角度を マイナス%5d秒に変更しました", -(int)(hds->now.col_scan*3600));
    }
    else{
      sprintf(tmp, "クロスディスパーザーのスキャン角度を %5d秒に変更しました", (int)(hds->now.col_scan*3600));
    }
    talk_message(tmp);
  }

  if(fabs(hds->old1.col_echelle-hds->now.col_echelle)>0.0001){
    if(hds->now.col_echelle<0){
      sprintf(tmp, "エシェルのスキャン角度を マイナス%5d秒に変更しました.", -(int)(hds->now.col_echelle*3600));
    }
    else{
      sprintf(tmp, "エシェルのスキャン角度を %5d秒に変更しました.", (int)(hds->now.col_echelle*3600));
    }
    talk_message(tmp);
  }

  if(strcmp(hds->old1.slit_filter1,hds->now.slit_filter1)!=0){
    sprintf(tmp, "フィルター 1を %sに変更しました", hds->now.slit_filter1);
    talk_message(tmp);
  }

  if(strcmp(hds->old1.slit_filter2,hds->now.slit_filter2)!=0){
    sprintf(tmp, "フィルター 2を %sに変更しました", hds->now.slit_filter2);
    talk_message(tmp);
  }

  if((hds->old1.col_col!=hds->now.col_col)){
    switch(hds->now.col_col){
    case COL_RED:
      talk_message("コリメーターを 赤に変更しました");
      break;
    case COL_BLUE:
      talk_message("コリメーターを 青に変更しました");
      break;
    }
  }

  if((hds->old1.col_cross!=hds->now.col_cross)){
    switch(hds->now.col_cross){
    case COL_RED:
      talk_message("クロスディスパーザーを 赤に変更しました");
      break;
    case COL_BLUE:
      talk_message("クロスディスパーザーを 青に変更しました");
      break;
    case COL_MIRROR:
      talk_message("クロスディスパーザーを 平面鏡にしました");
      break;
    }
  }



  // I2-Cell
  if(hds->old1.mode_i2!=hds->now.mode_i2){
    if(hds->now.mode_i2==1){
      talk_message("アイツーセルモードをオンにしました");
    }
    else{
      talk_message("アイツーセルモードをオフにしました");
    }
  }

  if((hds->old1.option_i2!=hds->now.option_i2)){
    if(hds->now.option_i2==0){
      talk_message("アイツーセルがこうろからはずれました");
    }
    else if(hds->now.option_i2==1){
      talk_message("アイツーセルをこうろに挿入しました");
    }
    else{
      talk_message("アイツーセルのポジションがわかりません");
    }
  }


  // LM
  if(hds->old1.mode_lm!=hds->now.mode_lm){
    if(hds->now.mode_lm==1){
      talk_message("こうりょうモニターモードをオンにしました");
    }
    else{
      talk_message("こうりょうモニターモードをオフにしました");
    }
  }

  if((hds->old1.option_lm!=hds->now.option_lm)){
    if(hds->now.option_lm==0){
      talk_message("こうりょうモニターがこうろからはずれました");
    }
    else if(hds->now.option_lm==1){
      talk_message("こうりょうモニターをこうろに挿入しました");
    }
    else{
      talk_message("こうりょうモニターのポジションがわかりません");
    }
  }

  // Image Slicer
  if(hds->old1.is_unit!=hds->now.is_unit){
    if(hds->now.is_unit==0){
      talk_message("イメージスライサーを使用していません");
    }
    else{
      sprintf(tmp, "イメージスライサー ナンバー%dを使用します", (int)hds->now.is_unit);
      talk_message(tmp);
      sprintf(tmp, "イメージスライサーは %4.2lf秒かける%dです",
	      hds->now.is_width*2.0,hds->now.is_slic);
      talk_message(tmp);
    }
  }


  // Shutter
  set_oc(hds->now.shutter_shutter, hds->old1.shutter_shutter, "シャッター"); 

  set_oc(hds->now.shutter_hu, hds->old1.shutter_hu, "上部ハルトマンシャッター");
  set_oc(hds->now.shutter_hl, hds->old1.shutter_hu, "下部ハルトマンシャッター");


  // Cover
  set_oc(hds->now.cover_colb, hds->old1.cover_colb, "コリメーター 青のカバー");
  set_oc(hds->now.cover_colr, hds->old1.cover_colr, "コリメーター 赤のカバー");
  set_oc(hds->now.cover_crossb, hds->old1.cover_crossb, "クロスディスパーザー 青のカバー");
  set_oc(hds->now.cover_crossr, hds->old1.cover_crossr, "クロスディスパーザー 赤のカバー");
  set_oc(hds->now.cover_mirr, hds->old1.cover_mirr, "クロスディスパーザー 平面鏡のカバー");
  set_oc(hds->now.cover_lens1, hds->old1.cover_lens1, "第一レンズカバー");
  set_oc(hds->now.cover_lens2, hds->old1.cover_lens2, "第二レンズカバー");
  set_oc(hds->now.cover_lens3, hds->old1.cover_lens3, "第三レンズカバー");
  set_oc(hds->now.cover_cam1, hds->old1.cover_cam1, "第一カメラミラーカバー");
  set_oc(hds->now.cover_cam2, hds->old1.cover_cam2, "第二カメラミラーカバー");

  hds->old2=hds->old1;
  hds->old1=hds->now;
  hds->init_status=TRUE;
  return;
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


void set_oc(int oc_status, int oc_old, gchar* name)
{
  gchar tmp[BUFFSIZE];

  if(oc_status!=oc_old){
    if(oc_status==0){
      sprintf(tmp, "%sをとじました", name);
      talk_message(tmp);
    }
    else if(oc_status==1){
      sprintf(tmp, "%sを開けました", name);
      talk_message(tmp);
    }
    else{
      sprintf(tmp, "%sのステータスがわかりません", name);
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


  if(hds->now.mode_obs==1){
    printf("OBS-MODE        = Yes\n");
  }
  else if(hds->now.mode_obs==0){
    printf("OBS-MODE        = No\n");
  }
  else if(hds->now.mode_obs==-1){
    printf("OBS-MODE        = Unknown\n");
  }

  if(hds->now.mode_i2==1){
    printf("I2-CELL         = Yes\n");
  }
  else if(hds->now.mode_i2==0){
    printf("I2-CELL         = No\n");
  }
  else if(hds->now.mode_i2==-1){
    printf("I2-CELL         = Unknown\n");
  }

  if(hds->now.mode_lm==1){
    printf("Light Monitor   = Yes\n");
  }
  else if(hds->now.mode_lm==0){
    printf("Light Monitor   = No\n");
  }
  else if(hds->now.mode_lm==-1){
    printf("Light Monitor   = Unknown\n");
  }


  printf("Slit Width      =  %7.5f[arcsec]/ %7.5f[mm]\n",
	 hds->now.slit_width,hds->now.slit_width/2);
  printf("Slit Length     = %8.5f[arcsec]/%8.5f[mm]\n",
	 hds->now.slit_length,hds->now.slit_length/2);
  printf("Filter1         = %s\n",hds->now.slit_filter1);
  printf("Filter2         = %s\n",hds->now.slit_filter2);

  if(hds->now.shutter_shutter==1){
    printf("SHUTTER         = Open\n");
  }
  else if(hds->now.shutter_shutter==0){
    printf("SHUTTER         = Close\n");
  }
  else if(hds->now.shutter_shutter==-1){
    printf("SHUTTER         = Unknown\n");
  }

  if(hds->now.shutter_hu==1){
    printf("Hartman Upper   = Open\n");
  }
  else if(hds->now.shutter_hu==0){
    printf("Hartman Upper   = Close\n");
  }
  else if(hds->now.shutter_hu==-1){
    printf("Hartman Upper   = Unknown\n");
  }

  if(hds->now.shutter_hl==1){
    printf("Hartman Lower   = Open\n");
  }
  else if(hds->now.shutter_hl==0){
    printf("Hartman Lower   = Close\n");
  }
  else if(hds->now.shutter_hl==-1){
    printf("Hartman Lower   = Unknown\n");
  }


  if(hds->now.col_col==1){
    printf("Collimator      = Blue\n");
  }
  else if(hds->now.col_col==0){
    printf("Collimator      = Red\n");
  }
  else if(hds->now.col_col==-1){
    printf("Collimator      = Unknown\n");
  }

  if(hds->now.col_cross==1){
    printf("Cross Disperser = Blue\n");
  }
  else if(hds->now.col_cross==0){
    printf("Cross Disperser = Red\n");
  }
  else if(hds->now.col_cross==2){
    printf("Cross Disperser = Mirror\n");
  }
  else if(hds->now.col_cross==-1){
    printf("Cross Disperser = Unknown\n");
  }

  printf("Echelle         = %8.5f[deg]/%6d[sec]\n",
	 hds->now.col_echelle,(int)(hds->now.col_echelle*3600));
  printf("Cross Scan      = %8.5f[deg]/%6d[sec]\n",
	 hds->now.col_scan,(int)(hds->now.col_scan*3600));

  printf("Camera Rotate   = %8.5f[deg]/%6d[sec]\n",
	 hds->now.cam_rotate,(int)(hds->now.cam_rotate*3600));
  printf("Camera Z        = %8.5f[mm]\n",hds->now.cam_z);
  printf("Camera X        = %8.5f[deg]/%6d[sec]\n",
	 hds->now.cam_x,(int)(hds->now.cam_x*3600));
  printf("Camera Y        = %8.5f[deg]/%6d[sec]\n",
	 hds->now.cam_y,(int)(hds->now.cam_y*3600));


  if(hds->now.cover_lens1==1){
    printf("Cover Lens1     = Open\n");
  }
  else if(hds->now.cover_lens1==0){
    printf("Cover Lens1     = Close\n");
  }
  else if(hds->now.cover_lens1==-1){
    printf("Cover Lens1     = Unknown\n");
  }

  if(hds->now.cover_lens2==1){
    printf("Cover Lens2     = Open\n");
  }
  else if(hds->now.cover_lens2==0){
    printf("Cover Lens2     = Close\n");
  }
  else if(hds->now.cover_lens2==-1){
    printf("Cover Lens2     = Unknown\n");
  }

  if(hds->now.cover_lens3==1){
    printf("Cover Lens3     = Open\n");
  }
  else if(hds->now.cover_lens3==0){
    printf("Cover Lens3     = Close\n");
  }
  else if(hds->now.cover_lens3==-1){
    printf("Cover Lens3     = Unknown\n");
  }

  if(hds->now.cover_cam1==1){
    printf("Cover Cam1      = Open\n");
  }
  else if(hds->now.cover_cam1==0){
    printf("Cover Cam1      = Close\n");
  }
  else if(hds->now.cover_cam1==-1){
    printf("Cover Cam1      = Unknown\n");
  }

  if(hds->now.cover_cam2==1){
    printf("Cover Cam2      = Open\n");
  }
  else if(hds->now.cover_cam2==0){
    printf("Cover Cam2      = Close\n");
  }
  else if(hds->now.cover_cam2==-1){
    printf("Cover Cam2      = Unknown\n");
  }

  if(hds->now.cover_colr==1){
    printf("Cover Col. Red  = Open\n");
  }
  else if(hds->now.cover_colr==0){
    printf("Cover Col. Red  = Close\n");
  }
  else if(hds->now.cover_colr==-1){
    printf("Cover Col. Red  = Unknown\n");
  }

  if(hds->now.cover_colb==1){
    printf("Cover Col. Blue = Open\n");
  }
  else if(hds->now.cover_colb==0){
    printf("Cover Col. Blue = Close\n");
  }
  else if(hds->now.cover_colb==-1){
    printf("Cover Col. Blue = Unknown\n");
  }

  if(hds->now.cover_crossr==1){
    printf("Cover Cross R   = Open\n");
  }
  else if(hds->now.cover_crossr==0){
    printf("Cover Cross R   = Close\n");
  }
  else if(hds->now.cover_crossr==-1){
    printf("Cover Cross R   = Unknown\n");
  }

  if(hds->now.cover_crossb==1){
    printf("Cover Cross B   = Open\n");
  }
  else if(hds->now.cover_crossb==0){
    printf("Cover Cross B   = Close\n");
  }
  else if(hds->now.cover_crossb==-1){
    printf("Cover Cross B   = Unknown\n");
  }

  if(hds->now.cover_mirr==1){
    printf("Cover Cross M   = Open\n");
  }
  else if(hds->now.cover_mirr==0){
    printf("Cover Cross M   = Close\n");
  }
  else if(hds->now.cover_mirr==-1){
    printf("Cover Cross M   = Unknown\n");
  }


  if(hds->now.option_i2==1){
    printf("I2-Cell         = In\n");
  }
  else if(hds->now.option_i2==0){
    printf("I2-Cell         = Out\n");
  }
  else if(hds->now.option_i2==-1){
    printf("I2-Cell         = Unknown\n");
  }

  if(hds->now.option_lm==1){
    printf("Light Monitor   = In\n");
  }
  else if(hds->now.option_lm==0){
    printf("Light Monitor   = Out\n");
  }
  else if(hds->now.option_lm==-1){
    printf("Light Monitor   = Unknown\n");
  }

  printf("Temp I2-inside    = %6.3f[deg C]\n",hds->now.temp_i2i);
  printf("Temp I2-outside   = %6.3f[deg C]\n",hds->now.temp_i2o);
  printf("Temp Nasmyth1     = %6.3f[deg C]\n",hds->now.temp_nr1);
  printf("Temp Nasmyth2     = %6.3f[deg C]\n",hds->now.temp_nr2);
  printf("Temp CCD          = %5.1f[K]\n",hds->now.temp_ccd);
  printf("CCD Temp Time     = %s\n",hds->temp_time);

#endif
}


// Main
int main(int argc, char* argv[]){
  gint timer;
  hds_param *hds;
  char rcfile[BUFFSIZE];
  int i_opt;

  setlocale(LC_ALL, "ja_JP.UTF-8");

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

  hds=g_malloc0(sizeof(hds_param));

  param_read(hds);

  init_hds(&hds->old1);
  init_hds(&hds->old2);
  file_search(hds); /* 最初の一回 */

  hds->init_status=FALSE;

  while(1){
    time_func(hds);
    usleep(INTERVAL*1000);
  }
}


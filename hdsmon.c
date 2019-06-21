//    HDS status monitor program 
//    - Please edit STATUSDATA in hdsmon.h, which should include
//      the status information of HDS and be refered by this program.
//    - For compile,
//      cc -o hdsmon hdsmon.c `gtk-config --cflags` `gtk-config --libs` 
//                                           2002.11.07  A.Tajitsu

#include"hdsmon.h"    /* hdsmon用設定ヘッダ */

//GdkFont *font_frame, *font_label, *font_small;


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

  //tmp=g_malloc0(sizeof(gchar)*BUFFSIZE);


  // Update time
  gtk_label_set_markup(GTK_LABEL(hds->w_update_time),hds->update_time);

  // OBS mode
  if(hds->mode_obs==1){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hds->w_mode_obs),TRUE);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hds->w_mode_obs),FALSE);
  }
  // Driving
  if(hds->status_driving==0){
    gtk_label_set_markup(GTK_LABEL(hds->w_status_driving), " ");
  }
  else if(hds->status_driving==1){
    gtk_label_set_markup(GTK_LABEL(hds->w_status_driving), 
			 "<span color=\"#FFFFFF\" bgcolor=\"FF0000\">    <b>Driving!!</b>    </span>");
  }
  else{
    gtk_label_set_markup(GTK_LABEL(hds->w_status_driving), 
			 "<span color=\"#888888\">  <b><i>Unknown</i></b>  </span>");
  }
    

  // Slit
  if(hds->old1.slit_width!=hds->now.slit_width){
    tmp=g_strdup_printf("<b>Slit</b>     [<i>R</i>=%6d]", 
			(int)(3.6e4/hds->now.slit_width));
    gtk_label_set_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(hds->w_slit_frame))),
			 tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%7.4f[arcsec]", hds->now.slit_width);
    gtk_label_set_markup(GTK_LABEL(hds->w_slit_width),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[&#xB5;m]",
			(int)(hds->now.slit_width/2*1e3));
    gtk_label_set_markup(GTK_LABEL(hds->w_bslit_width),tmp);
    g_free(tmp);
    
    change_color(hds->w_lslit_width, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.slit_width!=hds->old1.slit_width) {
    change_color(hds->w_lslit_width, COLOR_BROWN, TRUE, FALSE);
  }

  if(hds->old1.slit_length!=hds->now.slit_length){
    tmp=g_strdup_printf("%7.4f[arcsec]", hds->now.slit_length);
    gtk_label_set_markup(GTK_LABEL(hds->w_slit_length),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[&#xB5;m]", (int)(hds->now.slit_length/2*1e3));
    gtk_label_set_markup(GTK_LABEL(hds->w_bslit_length),tmp);
    g_free(tmp);

    gtk_label_set_markup(GTK_LABEL(hds->w_lslit_width),
			 "<span color=\"#FF0000\"><b>Width</b></span>");
    change_color(hds->w_lslit_length, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.slit_length!=hds->old1.slit_length) {
    change_color(hds->w_lslit_length, COLOR_BROWN, TRUE, FALSE);
  }

  // Camera
  if(hds->old1.cam_rotate!=hds->now.cam_rotate){
    tmp=g_strdup_printf("%7.4f[deg]", hds->now.cam_rotate);
    gtk_label_set_markup(GTK_LABEL(hds->w_cam_rotate),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[sec]", (int)(hds->now.cam_rotate*3600));
    gtk_label_set_markup(GTK_LABEL(hds->w_bcam_rotate),tmp);
    g_free(tmp);

    change_color(hds->w_lcam_rotate, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.cam_rotate!=hds->old1.cam_rotate) {
    change_color(hds->w_lcam_rotate, COLOR_GRAY2, TRUE, FALSE);
  }

  if(hds->old1.cam_z!=hds->now.cam_z){
    tmp=g_strdup_printf("%d[&#xB5;m]", (int)(hds->now.cam_z*1000));
    gtk_label_set_markup(GTK_LABEL(hds->w_cam_z),tmp);
    g_free(tmp);

    change_color(hds->w_lcam_z, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.cam_z!=hds->old1.cam_z) {
    change_color(hds->w_lcam_z, COLOR_BROWN, TRUE, FALSE);
  }

  if((hds->old1.cam_x!=hds->now.cam_x)){
    tmp=g_strdup_printf("%7.4f[deg]", hds->now.cam_x);
    gtk_label_set_markup(GTK_LABEL(hds->w_cam_x),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[sec]", (int)(hds->now.cam_x*3600));
    gtk_label_set_markup(GTK_LABEL(hds->w_bcam_x),tmp);
    g_free(tmp);

    change_color(hds->w_lcam_x, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.cam_x!=hds->old1.cam_z) {
    change_color(hds->w_lcam_x, COLOR_GRAY2, TRUE, FALSE);
  }

  if((hds->old1.cam_y!=hds->now.cam_y)){
    tmp=g_strdup_printf("%7.4f[deg]", hds->now.cam_y);
    gtk_label_set_markup(GTK_LABEL(hds->w_cam_y),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[sec]", (int)(hds->now.cam_y*3600));
    gtk_label_set_markup(GTK_LABEL(hds->w_bcam_y),tmp);
    g_free(tmp);

    change_color(hds->w_lcam_y, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.cam_y!=hds->old1.cam_y) {
    change_color(hds->w_lcam_y, COLOR_GRAY2, TRUE, FALSE);
  }


  // Wavelength
  if((hds->old1.setting!=hds->now.setting)){
    if(hds->now.setting==0){
      tmp=g_strdup("<b>Wavelength setting</b>     [Non-Standard]");
    }
    else{
      tmp=g_strdup_printf("<b>Wavelength setting</b>     [Std%s]",
			  setting_name[hds->now.setting]);
    }
    gtk_label_set_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(hds->w_col_frame))),
			 tmp);
    g_free(tmp);

    gtk_widget_set_tooltip_text(hds->w_lslit_length,
				setting_tip[hds->now.setting]);
    gtk_widget_set_tooltip_text(hds->w_lcol_filter,
				filter_tip[hds->now.setting]);
  }

  if((hds->old1.col_scan!=hds->now.col_scan)){
    tmp=g_strdup_printf("%7.4f[deg]", hds->now.col_scan);
    gtk_label_set_markup(GTK_LABEL(hds->w_col_scan),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[sec]", (int)(hds->now.col_scan*3600));
    gtk_label_set_markup(GTK_LABEL(hds->w_bcol_scan),tmp);
    g_free(tmp);

    change_color(hds->w_lcol_scan, COLOR_RED, TRUE, FALSE);

    switch(hds->now.col_col){
    case 0:
      tmp=g_strdup_printf("Echelle0 for Red = %d[sec]", hds->ec0_r);
      break;
      
    case 1:
      tmp=g_strdup_printf("Echelle0 for Blue = %d[sec]", hds->ec0_b);
      break;
      
    default:
      tmp=g_strdup("Echelle0 ~ 0.25[deg]=900[sec]");
      break;
    }
    gtk_widget_set_tooltip_text(hds->w_lcol_echelle,tmp);
    g_free(tmp);

    switch(hds->now.col_col){
    case 0:
      tmp=g_strdup_printf("dCross for Red = %+d[sec]", hds->dcr_r);
      break;
      
    case 1:
      tmp=g_strdup_printf("dCross for Blue = %+d[sec]", hds->dcr_b);
      break;
      
    default:
      tmp=g_strdup("dCross is unknown...]");
      break;
    }

    gtk_widget_set_tooltip_text(hds->w_lcol_scan,tmp);
    g_free(tmp);
  }
  else if(hds->old2.col_scan!=hds->old1.col_scan) {
    change_color(hds->w_lcol_scan, COLOR_BROWN, TRUE, FALSE);
  }

  if((hds->old1.col_echelle!=hds->now.col_echelle)){
    tmp=g_strdup_printf("%7.4f[deg]", hds->now.col_echelle);
    gtk_label_set_markup(GTK_LABEL(hds->w_col_echelle),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%5d[sec]", (int)(hds->now.col_echelle*3600));
    gtk_label_set_markup(GTK_LABEL(hds->w_bcol_echelle),tmp);
    g_free(tmp);

    change_color(hds->w_lcol_echelle, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.col_echelle!=hds->old1.col_echelle) {
    change_color(hds->w_lcol_echelle, COLOR_GRAY2, TRUE, FALSE);
  }

  if((strcmp(hds->old1.slit_filter1,hds->now.slit_filter1)!=0)||
     (strcmp(hds->old1.slit_filter2,hds->now.slit_filter2)!=0)){
    tmp=g_strdup_printf("%s", hds->now.slit_filter1);
    gtk_label_set_markup(GTK_LABEL(hds->w_col_filter),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%s", hds->now.slit_filter2);
    gtk_label_set_markup(GTK_LABEL(hds->w_bcol_filter),tmp);
    g_free(tmp);

    change_color(hds->w_lcol_filter, COLOR_RED, TRUE, FALSE);
  }
  else if((strcmp(hds->old2.slit_filter1,hds->old1.slit_filter1)!=0)||
	  (strcmp(hds->old2.slit_filter2,hds->old1.slit_filter2)!=0)) {
    change_color(hds->w_lcol_filter, COLOR_BROWN, TRUE, FALSE);
  }

  if((hds->old1.col_col!=hds->now.col_col)){
    set_br(hds->w_col_col, hds->now.col_col);
    change_color(hds->w_lcol_col, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.col_col!=hds->old1.col_col) {
    change_color(hds->w_lcol_col, COLOR_GRAY2, TRUE, FALSE);
  }

  if((hds->old1.col_cross!=hds->now.col_cross)){
    set_br(hds->w_col_cross, hds->now.col_cross);
    change_color(hds->w_lcol_cross, COLOR_RED, TRUE, FALSE);
  }
  else if(hds->old2.col_cross!=hds->old1.col_cross) {
    change_color(hds->w_lcol_cross, COLOR_GRAY2, TRUE, FALSE);
  }



  // I2-Cell
  if(hds->mode_i2==1){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hds->w_mode_i2),TRUE);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hds->w_mode_i2),FALSE);
  }

  if((hds->old1.option_i2!=hds->now.option_i2)){
    set_io(hds->w_option_i2, hds->now.option_i2);
    change_color(hds->w_loption_i2, COLOR_RED, TRUE, FALSE);
  }
  else if((hds->old1.option_i2!=hds->old2.option_i2)) {
    change_color(hds->w_loption_i2, COLOR_GRAY2, TRUE, FALSE);
  }


  // LM-Cell
  if(hds->mode_lm==1){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hds->w_mode_lm),TRUE);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hds->w_mode_lm),FALSE);
  }
  if((hds->old1.option_lm!=hds->now.option_lm)){
    set_io(hds->w_option_lm, hds->now.option_lm);
    change_color(hds->w_loption_lm, COLOR_RED, TRUE, FALSE);
  }
  else if((hds->old1.option_lm!=hds->old2.option_lm)) {
    change_color(hds->w_loption_lm, COLOR_GRAY2, TRUE, FALSE);
  }

  if((hds->old1.is_unit!=hds->now.is_unit)){
    if(hds->now.is_unit==0){
      tmp=g_strdup("None");
    }else{
      tmp=g_strdup_printf("IS #%d (%4.2fx%d)", 
			  hds->now.is_unit, 
			  hds->now.is_width*2.0,hds->now.is_slic);
    }
    gtk_label_set_markup(GTK_LABEL(hds->w_mode_is),tmp);
    g_free(tmp);

    change_color(hds->w_mode_is, COLOR_RED, FALSE, FALSE);
  }
  else if((hds->old1.is_unit!=hds->old2.is_unit)) {
    change_color(hds->w_mode_is, COLOR_BLACK, FALSE, FALSE);
  }

  
  if((strncmp(hds->now.slit_filter1, "FREE", 4)==0)&&
     (strncmp(hds->now.slit_filter2, "FREE", 4)==0)){
    tmp= g_strdup_printf("(<i>ideal pos = %+.3lfV</i>)", 
			 ideal_colv0[hds->now.is_unit]); 
  }
  else{
    tmp= g_strdup_printf("(<i>ideal pos = %+.3lfV</i>)", 
			 ideal_colv1[hds->now.is_unit]); 
  }
  gtk_label_set_markup(GTK_LABEL(hds->w_colv),tmp);

  // Temp
  tmp=g_strdup_printf("%5.2f", hds->now.temp_i2i);
  gtk_label_set_markup(GTK_LABEL(hds->w_temp_i2),tmp);
  g_free(tmp);

  tmp=g_strdup_printf("%5.2f[&#xB0;C]", hds->now.temp_i2o);
  gtk_label_set_markup(GTK_LABEL(hds->w_btemp_i2),tmp);
  g_free(tmp);

  tmp=g_strdup_printf("%5.2f", hds->now.temp_nr1);
  gtk_label_set_markup(GTK_LABEL(hds->w_temp_nr),tmp);
  g_free(tmp);

  tmp=g_strdup_printf("%5.2f[&#xB0;C]", hds->now.temp_nr2);
  gtk_label_set_markup(GTK_LABEL(hds->w_btemp_nr),tmp);
  g_free(tmp);


  tmp=g_strdup_printf("%5.1f[K]", hds->now.temp_ccd);
  gtk_label_set_markup(GTK_LABEL(hds->w_temp_ccd),tmp);
  g_free(tmp);

  tmp=g_strdup_printf("(%s)", hds->temp_time);
  gtk_label_set_markup(GTK_LABEL(hds->w_btemp_ccd),tmp);
  g_free(tmp);


  // Shutter
  set_oc2(hds->w_shutter_shutter, 
	  hds->w_lshutter_shutter,
	  hds->now.shutter_shutter, 
	  hds->old1.shutter_shutter, 
	  hds->old2.shutter_shutter,  
	  COLOR_BROWN);

  if((hds->old1.shutter_hu!=hds->now.shutter_hu)||
     (hds->old1.shutter_hl!=hds->now.shutter_hl)){
    set_oc(hds->w_shutter_h,  hds->now.shutter_hu);
    set_oc(hds->w_bshutter_h, hds->now.shutter_hl);

    change_color(hds->w_lshutter_h, COLOR_RED, TRUE, FALSE);
  }
  else if((hds->old1.shutter_hu!=hds->old2.shutter_hu)||
	  (hds->old1.shutter_hl!=hds->old2.shutter_hl)){
    change_color(hds->w_lshutter_h, COLOR_GRAY2, TRUE, FALSE);
  }


  // Cover
  set_oc2(hds->w_cover_colb, hds->w_lcover_colb,
	  hds->now.cover_colb, hds->old1.cover_colb, hds->old2.cover_colb,
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_colr, hds->w_lcover_colr,
	  hds->now.cover_colr, hds->old1.cover_colr, hds->old2.cover_colr, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_crossb, hds->w_lcover_crossb,
	  hds->now.cover_crossb, hds->old1.cover_crossb, hds->old2.cover_crossb, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_crossr, hds->w_lcover_crossr, 
	  hds->now.cover_crossr, hds->old1.cover_crossr, hds->old2.cover_crossr, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_mirr, hds->w_lcover_mirr, 
	  hds->now.cover_mirr, hds->old1.cover_mirr, hds->old2.cover_mirr, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_lens1, hds->w_lcover_lens1, 
	  hds->now.cover_lens1, hds->old1.cover_lens1, hds->old2.cover_lens1, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_lens2, hds->w_lcover_lens2, 
	  hds->now.cover_lens2, hds->old1.cover_lens2, hds->old2.cover_lens2, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_lens3, hds->w_lcover_lens3, 
	  hds->now.cover_lens3, hds->old1.cover_lens3, hds->old2.cover_lens3, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_cam1, hds->w_lcover_cam1, 
	  hds->now.cover_cam1, hds->old1.cover_cam1, hds->old2.cover_cam1, 
	  COLOR_GRAY2);
  set_oc2(hds->w_cover_cam2, hds->w_lcover_cam2, 
	  hds->now.cover_cam2, hds->old1.cover_cam2, hds->old2.cover_cam2, 
	  COLOR_GRAY2);

  hds->old2=hds->old1;
  //cp_stat(&hds->old2, &hds->old1);
  
  hds->old1=hds->now;
  //cp_stat(&hds->old1, &hds->now);


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


void cp_stat(stat_param *dest, stat_param *src){
  dest->slit_width=src->slit_width;
  dest->slit_length=src->slit_length;

  dest->cam_rotate=src->cam_rotate;
  dest->cam_x=src->cam_x;
  dest->cam_y=src->cam_y;
  dest->cam_z=src->cam_z;

  dest->shutter_shutter=src->shutter_shutter;
  dest->shutter_hu=src->shutter_hu;
  dest->shutter_hl=src->shutter_hl;

  dest->col_col=src->col_col;
  dest->col_echelle=src->col_echelle;
  dest->col_cross=src->col_cross;
  dest->col_scan=src->col_scan;

  dest->temp_i2o=src->temp_i2o;
  dest->temp_i2i=src->temp_i2i;
  dest->temp_nr1=src->temp_nr1;
  dest->temp_nr2=src->temp_nr2;
  dest->temp_ccd=src->temp_ccd;

  dest->option_i2=src->option_i2;
  dest->option_lm=src->option_lm;

  dest->is_unit=src->is_unit;
  dest->is_slic=src->is_slic;
  dest->is_width=src->is_width;

  dest->cover_colb=src->cover_colb;
  dest->cover_colr=src->cover_colr;
  dest->cover_crossb=src->cover_crossb;
  dest->cover_crossr=src->cover_crossr;
  dest->cover_mirr=src->cover_mirr;
  dest->cover_lens1=src->cover_lens1;
  dest->cover_lens2=src->cover_lens2;
  dest->cover_lens3=src->cover_lens3;
  dest->cover_cam1=src->cover_cam1;
  dest->cover_cam2=src->cover_cam2;

  dest->setting=src->setting;

  strcpy(dest->slit_filter1, src->slit_filter1);
  strcpy(dest->slit_filter2, src->slit_filter2);

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


void set_oc(GtkWidget *widget, int oc_status){
  switch(oc_status){
  case 0:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#000000\">Close</span>");
    break;

  case 1:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#7F7FFF\">Open</span>");
    break;

  default:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#888888\"><i>Unknown</i></span>");
    break;
  }
}


void set_oc2(GtkWidget *widget, GtkWidget *lwidget, 
	     int oc_status, int oc_old, int oc_old2,
	     gint lcolor)
{
  if(oc_status!=oc_old){
    if(oc_status==0){
      gtk_label_set_markup(GTK_LABEL(widget),"Close");
      change_color(widget, COLOR_BLACK, FALSE, FALSE);
    }
    else if(oc_status==1){
      gtk_label_set_markup(GTK_LABEL(widget),"Open");
      change_color(widget, COLOR_CYAN, FALSE, FALSE);
    }
    else{
      gtk_label_set_markup(GTK_LABEL(widget),"Unknown");
      change_color(widget, COLOR_GRAY1, FALSE, TRUE);
    }

    change_color(lwidget, COLOR_RED, TRUE, FALSE);
  }
  else if(oc_old!=oc_old2){
    change_color(lwidget, lcolor, TRUE, FALSE);
  }
}


void set_br(GtkWidget *widget, int br_status){
  switch(br_status){
  case 0:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#FF0000\">Red</span>");
    break;

  case 1:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#0000FF\">Blue</span>");
    break;

  case 2:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#00FF00\">Mirror</span>");
    break;

  default:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#888888\"><i>Unknown</i></span>");
    break;
  }
}


void set_io(GtkWidget *widget, int io_status){
  switch(io_status){
  case 0:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#000000\">Out</span>");
    break;

  case 1:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#7F7FFF\">In</span>");
    break;

  default:
    gtk_label_set_markup(GTK_LABEL(widget),
			 "<span color=\"#888888\"><i>Unknown</i></span>");
    break;
  }
}


void gui_init(hds_param *hds){
  GtkWidget *frame, *frame2;
  GtkWidget *table, *table1, *table2;
  GtkWidget *label, *button, *hbox;



  // Main Window 
  hds->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(hds->w_top, "destroy",
		   G_CALLBACK(gtk_main_quit),NULL);
  gtk_container_set_border_width(GTK_CONTAINER(hds->w_top),1);
  gtk_window_set_title(GTK_WINDOW(hds->w_top),"HDS monitor");

  ////////////////// top table //////////////////////
  table = gtkut_table_new(4,7,FALSE,5,5,5);
  gtk_container_add(GTK_CONTAINER(hds->w_top), table);

  // 更新時間
  hds->w_update_time = gtkut_label_new("Update Time");
  gtkut_pos(hds->w_update_time, POS_START, POS_CENTER);
  gtkut_table_attach(table, hds->w_update_time,0,1,0,1,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // OBS-mode
  hds->w_mode_obs = gtk_check_button_new_with_label("OBS mode");
  gtk_widget_set_sensitive(hds->w_mode_obs,FALSE);
  gtkut_table_attach(table, hds->w_mode_obs,1,3,0,1,
		     GTK_SHRINK, 0, 0);

  // Driving
  hds->w_status_driving = gtkut_label_new("<span color=\"#FF0000\"><b>    Driving!!    </b></span>");
  gtkut_pos(hds->w_status_driving, POS_END, POS_CENTER);
  gtkut_table_attach(table, hds->w_status_driving,3,4,0,1,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox,4,5,0,1, 
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL, "edit-find");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  g_signal_connect(button,"pressed", G_CALLBACK(call_efs), (gpointer)hds);
  gtk_widget_set_tooltip_text(button,"Echelle Format Simulator");


  /////////////////////// slit frame /////////////////////
  hds->w_slit_frame = gtkut_frame_new("<b>Slit</b>");
  gtkut_table_attach(table, hds->w_slit_frame,0,2,1,2,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // slit table
  table1 = gtkut_table_new(4,2,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(hds->w_slit_frame), table1);

  // slit width
  hds->w_lslit_width = gtkut_label_new("<span color=\"#880000\"><b>Width</b></span> ");
  gtkut_pos(hds->w_lslit_width, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lslit_width,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_slit_width = gtkut_label_new("--.----[arcsec]");
  gtkut_pos(hds->w_slit_width, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_slit_width,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,0,1,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bslit_width = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[&#xB5;m]</span>");
  gtkut_pos(hds->w_bslit_width, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bslit_width,3,4,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // slit length
  hds->w_lslit_length = gtkut_label_new("<span color=\"#880000\"><b>Length</b></span> ");
  gtkut_pos(hds->w_lslit_length, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lslit_length,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_slit_length = gtkut_label_new("--.----[arcsec]");
  gtkut_pos(hds->w_slit_length, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_slit_length,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,1,2,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bslit_length = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[&#xB5;m]</span>");
  gtkut_pos(hds->w_bslit_length, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bslit_length,3,4,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  
  ////////////////////////////// camera frame /////////////////////////
  frame = gtkut_frame_new("<b>Camera</b>");
  gtkut_table_attach(table, frame,0,2,2,4,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // camera table
  table1 = gtkut_table_new(4,4,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  // camera rotate
  hds->w_lcam_rotate = gtkut_label_new("<span color=\"#444444\"><b>Rotate</b></span> ");
  gtkut_pos(hds->w_lcam_rotate, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcam_rotate,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_widget_set_tooltip_text(hds->w_lcam_rotate,"-1[deg]=3600[sec] usually");

  hds->w_cam_rotate = gtkut_label_new("--.----[deg]");
  gtkut_pos(hds->w_cam_rotate, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_cam_rotate,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,0,1,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bcam_rotate = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[sec]</span>");
  gtkut_pos(hds->w_bcam_rotate, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bcam_rotate,3,4,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // camera Z
  hds->w_lcam_z = gtkut_label_new("<span color=\"#880000\"><b>Z</b></span> ");
  gtkut_pos(hds->w_lcam_z, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcam_z,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cam_z = gtkut_label_new("----[&#xB5;m]");
  gtkut_pos(hds->w_cam_z, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_cam_z,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // camera X
  hds->w_lcam_x = gtkut_label_new("<span color=\"#444444\"><b>X</b></span> ");
  gtkut_pos(hds->w_lcam_x, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcam_x,0,1,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_widget_set_tooltip_text(hds->w_lcam_x,"-0.014[deg]=-50[sec] usually");

  hds->w_cam_x = gtkut_label_new("--.----[deg]");
  gtkut_pos(hds->w_cam_x, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_cam_x,1,2,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
 gtkut_table_attach(table1, label,2,3,2,3,
		    GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bcam_x = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[sec]</span>");
  gtkut_pos(hds->w_bcam_x, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bcam_x,3,4,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // camera Y
  hds->w_lcam_y = gtkut_label_new("<span color=\"#444444\"><b>Y</b></span> ");
  gtkut_pos(hds->w_lcam_y, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcam_y,0,1,3,4,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_widget_set_tooltip_text(hds->w_lcam_y,"-0.069[deg]=-250[sec] usually");

  hds->w_cam_y = gtkut_label_new("--.----[deg]");
  gtkut_pos(hds->w_cam_y, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_cam_y,1,2,3,4,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,3,4,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bcam_y = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[sec]</span>");
  gtkut_pos(hds->w_bcam_y, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bcam_y,3,4,3,4,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);


  //////////////////////// Shutter frame ///////////////////////
  frame = gtkut_frame_new("<b>Shutter</b>");
  gtkut_table_attach(table, frame,0,2,4,5,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // Shutter table
  table1 = gtkut_table_new(4,2,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  // Shutter
  hds->w_lshutter_shutter = gtkut_label_new("<span color=\"#880000\"><b>Main Shutter</b></span> ");
  gtkut_pos(hds->w_lshutter_shutter, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lshutter_shutter,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_shutter_shutter = gtkut_label_new("------");
  gtkut_pos(hds->w_shutter_shutter, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_shutter_shutter,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Hartmann ShutterSlit Filter
  hds->w_lshutter_h = gtkut_label_new("<span color=\"#444444\"><b>Hartmann (Up/Low)</b></span> ");
  gtkut_pos(hds->w_lshutter_h, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lshutter_h,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_shutter_h = gtkut_label_new("------");
  gtkut_pos(hds->w_shutter_h, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_shutter_h,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("/");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_bshutter_h = gtkut_label_new("------");
  gtkut_pos(hds->w_bshutter_h, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bshutter_h,3,4,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);


  ///////////////////////// Wavelength frame /////////////////////////////
  hds->w_col_frame = gtkut_frame_new("<b>Wavelength setting</b>");
  gtkut_table_attach(table, hds->w_col_frame,2,5,1,3,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // Wavelength table
  table1 = gtkut_table_new(4,6,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(hds->w_col_frame), table1);

  // Slit Filter
  hds->w_lcol_filter = gtkut_label_new("<span color=\"#880000\"><b>Slit Filter</b></span> ");
  gtkut_pos(hds->w_lcol_filter, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcol_filter,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_col_filter = gtkut_label_new("------");
  gtkut_pos(hds->w_col_filter, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_col_filter,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("/");
  gtkut_pos(label, POS_CENTER, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_bcol_filter = gtkut_label_new("------");
  gtkut_pos(hds->w_bcol_filter, POS_START, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bcol_filter,3,4,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Echelle scan
  hds->w_lcol_echelle = gtkut_label_new("<span color=\"#444444\"><b>Echelle</b></span> ");
  gtkut_pos(hds->w_lcol_echelle, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcol_echelle,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_col_echelle = gtkut_label_new("--.----[deg]");
  gtkut_pos(hds->w_col_echelle, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_col_echelle,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,1,2,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bcol_echelle = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[sec]</span>");
  gtkut_pos(hds->w_bcol_echelle, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bcol_echelle,3,4,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);


  // Collimator
  hds->w_lcol_col = gtkut_label_new("<span color=\"#444444\"><b>Collimator</b></span> ");
  gtkut_pos(hds->w_lcol_col, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcol_col,0,1,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_col_col = gtkut_label_new("------");
  gtkut_pos(hds->w_col_col, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_col_col,1,2,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_colv = gtkut_label_new("(<i>ideal pos = ------</i>)");
  gtkut_pos(hds->w_colv, POS_CENTER, POS_CENTER);
  gtkut_table_attach(table1, hds->w_colv,1,4,3,4,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Cross Disperser
  hds->w_lcol_cross = gtkut_label_new("<span color=\"#444444\"><b>Cross</b></span> ");
  gtkut_pos(hds->w_lcol_cross, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcol_cross,0,1,4,5,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_col_cross = gtkut_label_new("------");
  gtkut_pos(hds->w_col_cross, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_col_cross,1,2,4,5,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Cross scan
  hds->w_lcol_scan = gtkut_label_new("<span color=\"#880000\"><b>Cross Scan</b></span> ");
  gtkut_pos(hds->w_lcol_scan, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_lcol_scan,0,1,5,6,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_col_scan = gtkut_label_new("--.----[deg]");
  gtkut_pos(hds->w_col_scan, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_col_scan,1,2,5,6,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("<span color=\"#888888\"> = </span>");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,5,6,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  hds->w_bcol_scan = gtkut_label_new("<span color=\"#888888\" size=\"smaller\">-----[sec]</span>");
  gtkut_pos(hds->w_bcol_scan, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_bcol_scan,3,4,5,6,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);


  //////////////////// i2 frame //////////////////////////
  frame = gtkut_frame_new("<b>I2-cell</b>");
  gtkut_table_attach(table, frame,0,1,5,6,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // i2 table
  table1 = gtkut_table_new(1,3,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  // i2 mode
  hds->w_mode_i2 = gtk_check_button_new_with_label("mode ");
  gtkut_table_attach(table1, hds->w_mode_i2,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_widget_set_sensitive(hds->w_mode_i2,FALSE);

  // i2 position
  hds->w_loption_i2 = gtkut_label_new("<span color=\"#444444\"><b>Pos.</b></span> ");
  gtkut_pos(hds->w_loption_i2, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_loption_i2,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_option_i2 = gtkut_label_new("---");
  gtkut_pos(hds->w_option_i2, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_option_i2,2,3,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);



  /////////////////////// lm frame  /////////////////////////////
  frame = gtkut_frame_new("<b>Light Monitor</b>");
  gtkut_table_attach(table, frame,1,3,5,6,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // lm table
  table1 = gtkut_table_new(1,3,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  // lm mode
  hds->w_mode_lm = gtk_check_button_new_with_label("mode ");
  gtkut_table_attach(table1, hds->w_mode_lm,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);
  gtk_widget_set_sensitive(hds->w_mode_lm,FALSE);

  // lm position
  hds->w_loption_lm = gtkut_label_new("<span color=\"#444444\"><b>Pos.</b></span> ");
  gtkut_pos(hds->w_loption_lm, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_loption_lm,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_option_lm = gtkut_label_new("---");
  gtkut_pos(hds->w_option_lm, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_option_lm,2,3,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);


  ////////////////////////// IS frame  ///////////////////////////
  frame = gtkut_frame_new("<b>Image Slicer</b>");
  gtkut_table_attach(table, frame,3,5,5,6,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // IS table
  table1 = gtkut_table_new(1,1,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  // IS unit
  hds->w_mode_is = gtkut_label_new("None");
  gtkut_pos(hds->w_mode_is, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_mode_is,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);



  
  /////////////////// temp frame /////////////////////////////
  frame = gtkut_frame_new("<b>Temperature</b>");
  gtkut_table_attach(table, frame,2,5,3,5,
		     GTK_SHRINK, GTK_SHRINK, 0, 0);

  // temp table
  table1 = gtkut_table_new(4,4,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  // I2-cell temp
  hds->w_ltemp_i2 = gtkut_label_new("<span color=\"#444444\"><b>I2-cell (In/Out)</b></span> ");
  gtkut_pos(hds->w_ltemp_i2, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_ltemp_i2,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_temp_i2 = gtkut_label_new("--.---");
  gtkut_pos(hds->w_temp_i2, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_temp_i2,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("/");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_btemp_i2 = gtkut_label_new("--.---");
  gtkut_pos(hds->w_btemp_i2, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_btemp_i2,3,4,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Nasmyth Room temp
  hds->w_ltemp_nr = gtkut_label_new("<span color=\"#444444\"><b>Ns.Room (1/2)</b></span> ");
  gtkut_pos(hds->w_ltemp_nr, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_ltemp_nr,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_temp_nr = gtkut_label_new("--.---");
  gtkut_pos(hds->w_temp_nr, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_temp_nr,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  label = gtkut_label_new("/");
  gtkut_pos(label, POS_END, POS_CENTER);
  gtkut_table_attach(table1, label,2,3,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_btemp_nr = gtkut_label_new("--.---");
  gtkut_pos(hds->w_btemp_nr, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_btemp_nr,3,4,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // CCD Detector temp
  hds->w_ltemp_ccd = gtkut_label_new("<span color=\"#444444\"><b>Detector</b></span> ");
  gtkut_pos(hds->w_ltemp_ccd, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_ltemp_ccd,0,1,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_temp_ccd = gtkut_label_new("---.-");
  gtkut_pos(hds->w_temp_ccd, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_temp_ccd,1,3,2,3,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_btemp_ccd = gtkut_label_new("(----------)");
  gtkut_pos(hds->w_btemp_ccd, POS_END, POS_CENTER);
  gtkut_table_attach(table1, hds->w_btemp_ccd,0,4,3,4,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);



  //////////////////// cover frame  //////////////////////////
  frame = gtkut_frame_new("<b>Cover</b>");
  gtkut_table_attach(table, frame,0,5,6,7,
		     GTK_SHRINK, GTK_SHRINK, FALSE, FALSE);

  // cover table
  table1 = gtkut_table_new(4,1,FALSE,3,3,3);
  gtk_container_add(GTK_CONTAINER(frame), table1);

  //////////// Collimator cover 
  frame2 = gtkut_frame_new("Collimator");
  gtkut_table_attach(table1, frame2,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Collimator cover table
  table2 = gtkut_table_new(2,2,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame2), table2);

  // Col Blue
  hds->w_lcover_colb = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">Blue</span>");
  gtkut_table_attach(table2, hds->w_lcover_colb,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_colb = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_colb,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Col Red
  hds->w_lcover_colr = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">Red</span>");
  gtkut_table_attach(table2, hds->w_lcover_colr,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_colr = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_colr,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);


  ////////////////// Cross cover 
  frame2 = gtkut_frame_new("Cross Disperser");
  gtkut_table_attach_defaults(table1, frame2,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Cross cover table
  table2 = gtkut_table_new(3,2,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame2), table2);

  // Cross Blue
  hds->w_lcover_crossb = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">Blue</span>");
  gtkut_table_attach(table2, hds->w_lcover_crossb,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_crossb = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_crossb,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Cross Red
  hds->w_lcover_crossr = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">Red</span>");
  gtkut_table_attach(table2, hds->w_lcover_crossr,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_crossr = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_crossr,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Cross Mirror
  hds->w_lcover_mirr = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">Mirror</span>");
  gtkut_table_attach(table2, hds->w_lcover_mirr,2,3,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_mirr = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_mirr,2,3,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  
  /////////////////// Lens cover 
  frame2 = gtkut_frame_new("Camera Lens");
  gtkut_table_attach(table1, frame2,2,3,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Lens cover table
  table2 = gtkut_table_new(3,2,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame2), table2);

  // Lens 1
  hds->w_lcover_lens1 = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">1</span>");
  gtkut_table_attach(table2, hds->w_lcover_lens1,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_lens1 = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_lens1,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Lens 2
  hds->w_lcover_lens2 = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">2</span>");
  gtkut_table_attach(table2, hds->w_lcover_lens2,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_lens2 = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_lens2,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Lens 3
  hds->w_lcover_lens3 = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">3</span>");
  gtkut_table_attach_defaults(table2, hds->w_lcover_lens3,2,3,0,1);

  hds->w_cover_lens3 = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_lens3,2,3,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  /////////////// Camera cover 
  frame2 = gtkut_frame_new("Camera Mirror");
  gtkut_table_attach(table1, frame2,3,4,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Camera cover table
  table2 = gtkut_table_new(2,2,FALSE,0,0,0);
  gtk_container_add(GTK_CONTAINER(frame2), table2);

  // Cam 1
  hds->w_lcover_cam1 = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">1</span>");
  gtkut_table_attach(table2, hds->w_lcover_cam1,0,1,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_cam1 = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_cam1,0,1,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  // Cam 2
  hds->w_lcover_cam2 = gtkut_label_new("<span color=\"#444444\" size=\"smaller\">2</span>");
  gtkut_table_attach(table2, hds->w_lcover_cam2,1,2,0,1,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  hds->w_cover_cam2 = gtkut_label_new("-----");
  gtkut_table_attach(table2, hds->w_cover_cam2,1,2,1,2,
		     GTK_FILL|GTK_EXPAND, GTK_SHRINK, 0, 0);

  gtk_widget_show_all(hds->w_top);
}



void change_color(GtkWidget *widget, gint color, 
		  gboolean flag_bold, gboolean flag_slant){
  const gchar *str;
  gchar *tmp;

  str=gtk_label_get_text(GTK_LABEL(widget));

  if(str){
    if(flag_bold){
      if(flag_slant){
	tmp=g_strdup_printf("<span color=\"%s\"><b><i>%s</i></b></span>", 
			    color_str[color], str);
      }
      else{
	tmp=g_strdup_printf("<span color=\"%s\"><b>%s</b></span>", 
			    color_str[color], str);
      }
    }
    else{
      if(flag_slant){
	tmp=g_strdup_printf("<span color=\"%s\"><i>%s</i></span>", 
			    color_str[color], str);
      }
      else{
	tmp=g_strdup_printf("<span color=\"%s\">%s</span>", 
			    color_str[color], str);
      }
    }
    
    gtk_label_set_markup(GTK_LABEL(widget), tmp);
    g_free(tmp);
  }
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

  hds->efs_mode=EFS_PLOT_EFS;
  hds->efs_bin=BIN_11;
  hds->bin_x=1;
  hds->bin_y=1;

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
    g_print("!!! DATA reading error...  LINE_NUMBER=%d\n",read_line);
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
				       

// Home Dir の取得
char* gethome(void){
    uid_t uid;
    struct passwd *pw;
    
    uid=getuid();
    pw=getpwuid(uid);

    return(pw->pw_dir);
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
  
  hds=g_malloc0(sizeof(hds_param));
  
  gtk_init(&argc, &argv);


  param_read(hds);

  init_hds(&hds->old1);
  init_hds(&hds->old2);
  file_search(hds); /* 最初の一回 */

  strcpy(rcfile, gethome());
  strcat(rcfile, "/.hdsmon/hdsmonrc");
  gui_init(hds);
  hds->init_status=FALSE;

  timer=g_timeout_add(INTERVAL, 
		      (GSourceFunc)time_func, 
		      (gpointer)hds);
  gtk_main();
  
}

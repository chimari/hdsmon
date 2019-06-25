#define VERSION 0.5

#undef DEBUG
#undef DEBUG2

#define USE_GTK3

#include<gtk/gtk.h>
#include <gio/gio.h>
#include <cairo.h>

#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<time.h>
#include<unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include <string.h>
#include <math.h>

#include "gtkut.h"

// reference status file(full-path)
//#define STATUSDATA "/home/taji/HDS/monitor/STATUSDATA"
#define STATUSDATA "/var/hds/STATUSDATA"
#define HDSPARAM "/etc/HDS/HDSparam.dat"

#define BUFFSIZE 256

#define LINE_NUMBER 47

// update interval [msec]
#define INTERVAL 2e3

// Cross Scan Error [arcsec]
#define E_SCAN 15

// Wavelength Standard Setup
enum{
 NOSTD ,
 STDUB ,
 STDUA ,
 STDBA ,
 STDBC ,
 STDYA ,
 STDI2B,  
 STDYD ,
 STDYB ,
 STDI2A ,
 STDYC ,
 STDRA ,
 STDRB ,
 STDNIRC ,
 STDNIRB ,
 STDNIRA ,
 NUM_SET }StandardSetup;


static gdouble ideal_colv0[]={
  +1.441, // Slit
  -1.062, // IS1
  -1.062, // IS2
  -2.292, // IS3 
};

static gdouble ideal_colv1[]={
  +2.106, // Slit
  -0.401, // IS1 
  -0.401, // IS2 
  -1.632  // IS3 
};


static gchar *setting_name[NUM_SET]={
  "Non-Standard", 
  "Ub", 
  "Ua", 
  "Ba", 
  "Bc", 
  "Ya", 
  "I2b",
  "Yd", 
  "Yb",
  "Yc", 
  "I2a", 
  "Ra", 
  "Rb", 
  "NIRc",
  "NIRb", 
  "NIRa"
};

static gint orig_cross_val[NUM_SET] = {
  -1,   //NOSTD ,  
  17820,//STDUB ,   
  17100,//STDUA ,   
  19260,//STDBA ,   
  19890,//STDBC ,   
  21960,//STDYA ,   
  14040,//STDI2B,  
  15480,//STDYD ,   
  15730,//STDYB ,   
  16500,//STDI2A ,  
  18000,//STDYC ,   
  18455,//STDRA ,   
  19080,//STDRB ,   
  21360,//STDNIRC , 
  22860,//STDNIRB , 
  25200	//STDNIRA ,
};

static gchar *setting_tip[NUM_SET]={
  "(non-standard setting)",
  "Max 5[arcsec] for StdUb",
  "Max 5[arcsec] for StdUa",
  "Max 6[arcsec] for StdBa",
  "Max 6[arcsec] for StdBc",
  "Max 7[arcsec] for StdYa",
  "Max 4[arcsec] for StdI2b"
  "Max 4[arcsec] for StdYd",
  "Max 4[arcsec] for StdYb",
  "Max 5[arcsec] for StdYc",
  "Max 7[arcsec] for StdI2a",
  "Max 7[arcsec] for StdRa", 
  "Max 8[arcsec] for StdRb",
  "Max 10[arcsec] for StdNIRc",
  "Max 10[arcsec] for StdNIRb",
  "Max 15[arcsec] for StdNIRa"
};

static gchar *filter_tip[NUM_SET]={
  "(non-standard setting)",
  "Free/Free for StdUb",
  "Free/Free for StdUa",
  "Free/Free for StdBa",
  "Free/Free for StdBc",
  "Free/Free for StdYa",
  "Free/Free for StdI2b",
  "Free/KV370 for StdYd",
  "Free/KV370 for StdYb",
  "KV480/Free for StdYc",
  "Free/SC-46 for StdI2a",
  "Free/SC-46 for StdRa", 
  "Free/SC-46 for StdRb",
  "OG530/Free for StdNIRc",
  "OG530/Free for StdNIRb",
  "OG530/Free for StdNIRa"
};


// Color for GUI
/*
GdkColor red   = {0, 0xffff, 0x0000, 0x0000};
GdkColor green = {0, 0x0000, 0xffff, 0x0000};
GdkColor blue  = {0, 0x0000, 0x0000, 0xffff};
GdkColor black = {0, 0x0000, 0x0000, 0x0000};
GdkColor white = {0, 0xffff, 0xffff, 0xffff};
GdkColor gray1 = {0, 0x7fff, 0x7fff, 0x7fff};
GdkColor gray2 = {0, 0x4fff, 0x4fff, 0x4fff};
GdkColor brown = {0, 0x8fff, 0x2fff, 0x2fff};
GdkColor cyan  = {0, 0x7fff, 0x7fff, 0xffff};
*/

// Font for GUI
//#define FONT_FRAME "*-helvetica-bold-r-normal--12-*-*-*-*-*-*-*"
//#define FONT_LABEL "-*-times-bold-r-*-*-12-*-*-*-*-*-*-*"
//#define FONT_SMALL "*-helvetica-medium-r-normal--10-*-*-*-*-*-*-*"

enum{
  COLOR_WHITE,
  COLOR_BLACK,
  COLOR_RED,
  COLOR_BLUE,
  COLOR_GRAY1,
  COLOR_GRAY2,
  COLOR_BROWN,
  COLOR_GREEN,
  COLOR_CYAN,  
  NUM_COLOR
};

static gchar* color_str[]={
  "#FFFFFF", //COLOR_WHITE,
  "#000000", //COLOR_BLACK,
  "#FF0000", //COLOR_RED,
  "#0000FF", //COLOR_BLUE,
  "#888888", //COLOR_GRAY1,
  "#444444", //COLOR_GRAY2,
  "#880000", //COLOR_BROWN,
  "#00FF00", //COLOR_GREEN,
  "#7F7FFF"  //COLOR_CYAN,  
};

typedef struct _Linepara Linepara;
struct _Linepara{
  gchar *name;
  gdouble  wave;
};

static const Linepara line_data[]={
  {"Be II",       3131.07},
  {"[OII]  3726", 3726.03},
  {"Ca K",        3933.68},
  {"Ca H",        3968.49},
  {"H delta",     4101.75},
  {"H gamma",     4340.48}, 
  {"HeII   4686", 4685.68}, 
  {"H beta",      4861.33},
  {"[OIII] 4959", 4958.92},
  {"[OIII] 5007", 5006.84},
  {"Mg b1",       5183.62},
  {"Na D1",       5895.94},
  {"[OI]   6300", 6300.30},
  {"H alpha",     6562.82},
  {"Li I",        6707.80},
  {"Ca II",       8498.06},
  {"Ca II",       8542.14},
  {"Ca II",       8662.17},
  {NULL, -1}
};

#define MAX_LINE 20
#define EFS_WIDTH 800
#define EFS_HEIGHT 600
enum{ EFS_PLOT_EFS, EFS_PLOT_FSR} EFSMode;

enum{
  BIN_11,
  BIN_21,
  BIN_22,
  BIN_24,
  BIN_41,
  BIN_44,
  NUM_BIN
};

static gchar* bin_name[]={
  "1x1 bin",
  "2x1 bin",
  "2x2 bin",
  "2x4 bin",
  "4x1 bin",
  "4x4 bin"
};

typedef struct _stat_param stat_param;
struct _stat_param{
  float slit_width, slit_length;
  float cam_rotate,cam_z,cam_x,cam_y;
  int shutter_shutter, shutter_hu, shutter_hl;
  int col_col, col_cross;
  float col_echelle, col_scan;
  float temp_i2i, temp_i2o, temp_ccd, temp_nr1, temp_nr2;
  int option_i2,option_lm;
  int is_unit, is_slic;
  float is_width;
  int cover_lens1, cover_lens2, cover_lens3, cover_cam1, cover_cam2,
    cover_colr, cover_colb, cover_crossr, cover_crossb, cover_mirr;
  int setting;
  char slit_filter1[32],slit_filter2[32];
  int mode_obs, mode_i2, mode_lm;
};


typedef struct _hds_param hds_param;
struct _hds_param{
  stat_param now;
  stat_param old1;
  stat_param old2;

  char update_time[32];
  int status_driving;
  char temp_time[32];

  GtkWidget *w_top,  *w_top_tbl, 
    *w_update_time, *w_mode_obs, *w_status_driving,
    *w_slit_frame, *w_slit_tbl, *w_lslit_width, *w_lslit_length, 
    *w_slit_width, *w_slit_length,
    *w_bslit_width, *w_bslit_length, 
    *w_cam_frame, *w_cam_tbl, *w_lcam_rotate, *w_lcam_z, *w_lcam_x, *w_lcam_y, 
    *w_cam_rotate, *w_cam_z, *w_cam_x, *w_cam_y,
    *w_bcam_rotate, *w_bcam_x, *w_bcam_y,
    *w_col_frame, *w_col_tbl, 
    *w_lcol_scan, *w_lcol_echelle, *w_lcol_filter, *w_lcol_col, *w_lcol_cross,
    *w_col_scan, *w_col_echelle, *w_col_filter,  *w_col_col, *w_col_cross,
    *w_bcol_scan, *w_bcol_echelle ,*w_bcol_filter,
    *w_i2_frame, *w_i2_tbl, *w_loption_i2, *w_mode_i2, 
    *w_option_i2,
    *w_lm_frame, *w_lm_tbl, *w_loption_lm, *w_mode_lm, 
    *w_option_lm,
    *w_is_frame, *w_is_tbl, *w_mode_is,
    *w_temp_frame, *w_temp_tbl, 
    *w_ltemp_i2, *w_ltemp_nr, *w_ltemp_ccd, 
    *w_temp_i2, *w_temp_nr, *w_temp_ccd, 
    *w_btemp_i2, *w_btemp_nr, *w_btemp_ccd,
    *w_shutter_frame, *w_shutter_tbl, 
    *w_lshutter_shutter, *w_lshutter_h,
    *w_shutter_shutter, *w_shutter_h,
    *w_bshutter_h,
    *w_cover_frame, *w_cover_tbl, 
    *w_cover_col_frame, *w_cover_col_tbl, 
    *w_lcover_colb, *w_lcover_colr,
    *w_cover_colb, *w_cover_colr,
    *w_cover_cross_frame, *w_cover_cross_tbl, 
    *w_lcover_crossb, *w_lcover_crossr, *w_lcover_mirr,
    *w_cover_crossb, *w_cover_crossr, *w_cover_mirr,
    *w_cover_lens_frame, *w_cover_lens_tbl, 
    *w_lcover_lens1, *w_lcover_lens2, *w_lcover_lens3,
    *w_cover_lens1, *w_cover_lens2, *w_cover_lens3,
    *w_cover_cam_frame, *w_cover_cam_tbl, 
    *w_lcover_cam1, *w_lcover_cam2,
    *w_cover_cam1, *w_cover_cam2,
    *w_colv;
  gboolean init_status;
  gint cross_val[NUM_SET];
  gint ec0_b, ec0_r;
  gint dcr_b, dcr_r;
  gint efs_mode;
  gint efs_bin, bin_x, bin_y;
  gchar *camz_b, *camz_r;

  GtkWidget *efs_main, *efs_dw;

  gint filnum, filnum_old;
  gint isnum, isnum_old;
};

GtkWidget* gtkut_frame_new();
GtkWidget* gtkut_label_new();

gint time_func();
gint time_func0();
void file_search();
void update_gui();
void gui_init();

void  init_hds();
void cp_stat();
char *get_param();
int   get_status();
int   get_status2();
int   get_status_is();
void  set_oc();
void  set_oc2();
int   get_oc();
int  *get_oc2();
void  set_br();
int   get_br();
void  set_io();
int   get_io();
int   get_setting();
void  skip_line();
char *gethome();
#ifdef DEBUG
void  debug_print();
#endif

void change_font();
void change_color();
void change_color_bg();
void change_color_insensitive();
char *capiterize();
gint absvalue();


void call_efs();


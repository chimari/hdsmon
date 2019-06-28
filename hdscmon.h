// #define DEBUG

#include<glib.h>
#include<math.h>

#define DELTA_CROSS (60./3600.)

// reference status file(full-path)
//#define STATUSDATA "/home/taji/HDS/monitor/STATUSDATA"
//#define STATUSDATA "/home/hdsuser/java/myprojects/hds/STATUSDATA"
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
  gboolean init_status;

  gint cross_val[NUM_SET];
  gint ec0_b, ec0_r;
  gint dcr_b, dcr_r;
  gchar *camz_b, *camz_r;
};



gint time_func();
gint time_func0();
void file_search();
void update_gui();
void gui_init();

void  init_hds();
char *get_param();
int   get_status_is();
int   get_status();
int   get_status2();
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



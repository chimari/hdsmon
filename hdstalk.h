// #define DEBUG

// reference status file(full-path)
//#define STATUSDATA "./STATUSDATA"
//#define STATUSDATA "/home/hdsuser/java/myprojects/hds/STATUSDATA"
#define STATUSDATA "/var/hds/STATUSDATA"

#define BUFFSIZE 256

#define D_CROSS (140./60./60.)

#define LINE_NUMBER 47

// update interval [msec]
#define INTERVAL 2e3

// Cross Scan Error [deg]
#define E_SCAN 0.01

//
enum{COL_RED,COL_BLUE,COL_MIRROR} Color_mode;

// Wavelength Standard Setup
enum{
 NOSTD ,
 STDUA ,
 STDUB ,
 STDBA ,
 STDBC ,
 STDYA ,
 STDYB ,
 STDYC ,
 STDYD ,
 STDRA ,
 STDRB ,
 STDNIRA ,
 STDNIRB ,
 STDNIRC ,
 STDI2A ,
   STDI2B }StandardSetup;

static char setting_name[20][20]=
  {"non-std", "StdUa", "StdUb", "StdBa", "StdBc", "StdYa", "StdYb",
   "StdYc", "StdYd", "StdRa", "StdRb", "StdNIRa", "StdNIRb", "StdNIRc",
   "StdI2a", "StdI2b"};

static char setting_talk[20][40]=
  {"non-standard", "Sandard U-A", "Standard U-B", "Standard B-A", "Standard B-C", "Standard Y-A", "Standard Y-B",
   "Standard Y-C", "Standard Y-D", "Standard R-A", "Standard R-B", "Standard Near Infrared-A", "Standard Near Infrared-B", "Standard Near Infrared C",
   "Standard I2-A", "Standard I2-B"};

static char setting_tip[20][64]=
  {"(non-standard setting)",
   "Max 5[arcsec] for StdUa",
   "Max 5[arcsec] for StdUb",
   "Max 6[arcsec] for StdBa",
   "Max 6[arcsec] for StdBc",
   "Max 7[arcsec] for StdYa",
   "Max 4[arcsec] for StdYb",
   "Max 5[arcsec] for StdYc",
   "Max 4[arcsec] for StdYd",
   "Max 7[arcsec] for StdRa", 
   "Max 8[arcsec] for StdRb",
   "Max 15[arcsec] for StdNIRa",
   "Max 10[arcsec] for StdNIRb",
   "Max 10[arcsec] for StdNIRc",
   "Max 7[arcsec] for StdI2a",
   "Max 4[arcsec] for StdI2b"};

static char filter_tip[20][64]=
  {"(non-standard setting)",
   "Free/Free for StdUa",
   "Free/Free for StdUb",
   "Free/Free for StdBa",
   "Free/Free for StdBc",
   "Free/Free for StdYa",
   "Free/KV370 for StdYb",
   "KV480/Free for StdYc",
   "Free/KV370 for StdYd",
   "Free/SC-46 for StdRa", 
   "Free/SC-46 for StdRb",
   "OG530/Free for StdNIRa",
   "OG530/Free for StdNIRb",
   "OG530/Free for StdNIRc",
   "Free/SC-46 for StdI2a",
   "Free/Free for StdI2b"};


typedef struct _hds_param hds_param;
struct _hds_param{
  char update_time[32];
  int status_driving;
  int mode_obs, mode_i2, mode_lm;
  int is_unit, is_slic;
  float is_width;
  float slit_width, slit_length;
  char slit_filter1[32],slit_filter2[32];
  int shutter_shutter, shutter_hu, shutter_hl;
  int col_col, col_cross;
  float col_echelle, col_scan;
  float cam_rotate,cam_z,cam_x,cam_y;
  int cover_lens1, cover_lens2, cover_lens3, cover_cam1, cover_cam2,
    cover_colr, cover_colb, cover_crossr, cover_crossb, cover_mirr;
  int option_i2,option_lm;
  float temp_i2i, temp_i2o, temp_ccd, temp_nr1, temp_nr2;
  char temp_time[32];
  gboolean init_status;
  int setting;
};




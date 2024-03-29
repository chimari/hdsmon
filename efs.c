// imported from
//    hoe : Subaru HDS OPE file Editor
//        efs.c :  Echelle Format Simulator
//                                           2018.02.14  A.Tajitsu

#include"hdsmon.h"
#include<math.h>


#define BUFFER_SIZE 1024

enum{CROSS_RED, CROSS_BLUE} cross_color;
enum{COL_RED, COL_BLUE} col_color;


void efs();

void close_efs();
void create_efs_dialog();
#ifdef USE_GTK3
gboolean draw_efs_cb();
#else
gboolean expose_efs_cairo();
#endif
gboolean configure_efs_cb();
gboolean draw_efs_cairo();
gdouble nx();
gdouble ny();
gdouble ny2();

static void refresh_efs();
void cc_get_efs_mode();
void cc_get_efs_bin();

// global arguments
#ifdef USE_GTK3
GdkPixbuf *pixbuf_efs=NULL;
#else
GdkPixmap *pixmap_efs=NULL;
#endif
int mout[9];
double wlout[9];
double ypix1[4][51], ypix2[4][51];
int n1end,  n2end;
double wl[MAX_LINE+1],xline[1000][4],yline1[1000][4],yline2[1000][4];
double xpt[1000],ypt[1000];
double wlfmax[201],wlfmin[201],xfmax[201],xfmin[201],yfmax[201],yfmin[201];
gchar line_name[MAX_LINE+1][BUFFER_SIZE];

gboolean flagEFS=FALSE;



void efs(int ncross,double theta_E_mes, double theta_C,double det_rot, int nlines){

  int    order1[51],order2[51];
  int    ypixmin1[51],ypixcen1[51],ypixmax1[51];
  double wlnmmin1[51],wlnmcen1[51],wlnmmax1[51];
  int    ypixmin2[51],ypixcen2[51],ypixmax2[51];
  double wlnmmin2[51],wlnmcen2[51],wlnmmax2[51];
  double wl0[201],wlmax[201],wlmin[201],wlb[201];
  double y0[201],ymax[201],ymin[201];
  double fsr[201];

  double rad, a_E, theta_E, a_C, a_CB, a_CR, cos_gam_C,cos_eps_E;
  double ccdgap, mccdgap, ccdmin, ccdmax, mccdmax, ccdsize, pixsize;
  double delta_rot, delta_pos, d_wly1, d_wly2, d_wlx0;
  double fcam;
  int   mmin1, mmin2, mmax1, mmax2, mmid1, mmid2;
  double wlmax1, wlmax2, wlmin1, wlmin2, wlmid1, wlmid2, wl_pix1, wl_pix2;
  int   n1, n2;

  double beta_E, beta_C, wl_cent;
  int  i,j,m;
  double deltawl , fsrpr;
  double d_wlx, wlpr1, wlpr2, wlpr3, wlpr4, wlpr5, ald, cdmin, cdcen, cdmax;
  double aordsep, wl1, wl2048, wl4096, yline;
  int yintmin, yintmax, yint0;
  double rough_x;


  rad=3.14159/180.0;
  a_E=1./316.0;
  theta_E=71.26;
  a_CB=1./4000.;
  a_CR=1./2500.;
  //cos_gam_C=0.92388;
  cos_gam_C=cos((22.5-0./3600.)*rad);
  //cos_eps_E=0.99452;
  cos_eps_E=cos((6-0./3600.0)*rad); //After EQ
  ccdgap=43.;
  mccdgap=-ccdgap;
  ccdmin=ccdgap;
  ccdmax=2048.+ccdgap;
  mccdmax=-ccdmax;
  ccdsize=5.5296;
  pixsize=0.00135;
  delta_rot=2048.*sin(rad*det_rot);
  //  +delta_rot for y=4096, -delta_rot for y=1
  delta_pos=20.;            // correction on detector position
  d_wly1=1.00;  
  d_wly2=1.00;
  d_wlx0=0.00;

  // parameters 
  fcam=77.0;

  mmin1=200;
  mmax1=0;
  mmin2=200;
  mmax2=0;
  mmid1=0;
  mmid2=0;
 
  wlmin1=1000.;
  wlmin2=1000.;
  wlmax1=0.;
  wlmax2=0.;
  wlmid1=0.;
  wlmid2=0.; 
  wl_pix1=0.; 
  wl_pix2=0.; 
  n1=1;
  n2=1;

  // variables 
  //      wl_cent,deltawl
  //	 m,m_max,m_min
  //	 yintmin,yint0,yintmax
  //	double wl1,wl2048,wl4096
  //   begin

  if(ncross==CROSS_BLUE){
    a_C=a_CB;
  }
  else{
    a_C=a_CR;
  }
   
  beta_E=theta_E;
  beta_C=theta_C-22.5;
  wl_cent=2.0*a_C*sin(rad*theta_C)*cos_gam_C;
  for(i=1;i<=nlines;i++){
    for(j=1;j<=3;j++){
      xline[i][j]=-10000.0;
      yline1[i][j]=-10000.0;
      yline2[i][j]=-10000.0;
    }
  }
  m=199;

  while(m>50){
    wlb[m]=2.0*a_E*cos_eps_E*sin(rad*theta_E)/(double)m;
    wl0[m]=wlb[m]+a_E*cos_eps_E*cos(rad*theta_E)
      *(tan(2.*rad*theta_E_mes))/(double)m;
    deltawl=a_E*cos_eps_E*cos(rad*beta_E)*0.5*ccdsize/(fcam*(double)m);
    wlmax[m]=wl0[m]+deltawl*d_wly1;
    wlmin[m]=wl0[m]-deltawl*d_wly2;
    if(wl0[m]>wl_cent){
      d_wlx=1.0+d_wlx0;
    }
    else{
      d_wlx=1.0-d_wlx0;
    }
    y0[m]=fcam*(wl0[m]*d_wlx-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize + delta_pos;
    if(wlmax[m]>wl_cent){
      d_wlx=1.0+d_wlx0;
    }
    else{
      d_wlx=1.0-d_wlx0;
    }
    ymax[m]=fcam*(wlmax[m]*d_wlx-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize-delta_rot + delta_pos +20.;
    if(wlmin[m]>wl_cent){
      d_wlx=1.0+d_wlx0;
    }
    else{
      d_wlx=1.0-d_wlx0;
    }
    ymin[m]=fcam*(wlmin[m]*d_wlx-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize+delta_rot + delta_pos;

    //*** Calculation of free spectrum range ***
    fsr[m]=wlb[m]/(double)m;
    wlfmax[m]=wlb[m] + 0.5*fsr[m];
    wlfmin[m]=wlb[m] - 0.5*fsr[m];
    xfmax[m]=0.5*wlb[m]*fcam/(a_E*cos_eps_E*cos(rad*beta_E))/pixsize;
    xfmin[m]=-xfmax[m];
    yfmax[m]=fcam*(wlfmax[m]-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize -delta_rot + delta_pos;
    yfmin[m]=fcam*(wlfmin[m]-wl_cent)/(a_C*cos(rad*beta_C))
      /pixsize +delta_rot + delta_pos;
    wlpr1=1.0e+7*wlb[m];     // [nm] 
    wlpr2=1.0e+7*wlfmin[m];  // [nm] 
    wlpr3=1.0e+7*wlfmax[m];  // [nm] 
    fsrpr=1.0e+7*fsr[m];     // [nm] 
    wlpr4=1.0e+7*wlmin[m];   // [nm] 
    wlpr5=1.0e+7*wlmax[m];   // [nm] 
    ald=2.0e+6*deltawl/ccdsize;  // [nm/mm]
    cdmin=(yfmin[m]-yfmin[199])*pixsize*10.; // [mm]
    cdcen=(y0[m]-yfmin[199])*pixsize*10.;    // [mm]
    cdmax=(yfmax[m]-yfmin[199])*pixsize*10.; // [mm]
    aordsep=cdmax-cdmin;                     // [mm]

    if( (ymax[m]>ccdmax) || 
	((ymin[m]<ccdgap)&&(ymax[m]>mccdgap)) ||
	(ymin[m]<mccdmax)){
    }
    else{
      yintmax = (int)(ymax[m]);
      yint0   = (int)(y0[m]);
      yintmin = (int)(ymin[m]);
      wl1    = 1.0e+7*wlmin[m];
      wl2048 = 1.0e+7*wl0[m];
      wl4096 = 1.0e+7*wlmax[m];
      for(i=1;i<=nlines;i++){
	if((wl[i]>=wl1)&&(wl[i]<=wl4096)){
	  j=1;
	  if(xline[i][j]>0.0){
	    j=2;
	    if(xline[i][j]>0.0){
	      j=3;
	      xline[i][j]=2048.0+2048.*(1.0e-7*(wl[i]-wl2048)/deltawl)*1.02;//!!!!
	    }
	    else{
	      xline[i][j]=2048.0+2048.*(1.0e-7*(wl[i]-wl2048)/deltawl)*1.02;
	    }
	  }
	  else{ 
	    xline[i][j]=2048.0+2048.*(1.0e-7*(wl[i]-wl2048)/deltawl)*1.02;
	  }
	  yline=fcam*(1.0e-7*wl[i]-wl_cent)/
	    (a_C*cos(rad*beta_C))/pixsize+delta_pos
	    -delta_rot*(xline[i][j]-2048.)/2048.;
	  if(yline>-ccdgap){
	    yline1[i][j]=2048.0+ccdgap-yline;
	    //yline1[i][j]=2048.0-yline;
	    yline2[i][j]=-90000.;
	  }
	  else{
	    yline1[i][j]=-90000.;
	    yline2[i][j]=-yline-ccdgap;
	  }
	}
      }
      
      if(ymin[m]>-ccdgap){
	order1[n1]=m;
	ypixmin1[n1]=2048+(int)(ccdgap)-yintmin;
	ypixcen1[n1]=2048+(int)(ccdgap)-yint0;
	ypixmax1[n1]=2048+(int)(ccdgap)-yintmax;
	wlnmmin1[n1]=wl1;
	wlnmcen1[n1]=wl2048;
	wlnmmax1[n1]=wl4096;
	//            write(6,101)order1[n1],ypixmin1[n1],ypixcen1[n1],
	//&       ypixmax1[n1],wlnmmin1[n1],wlnmcen1[n1],wlnmmax1[n1]
	// 101        format(i5,3i5,3f10.3)
	ypix1[1][n1]=(double)(ypixmin1[n1]);
	ypix1[2][n1]=(double)(ypixcen1[n1]);
	ypix1[3][n1]=(double)(ypixmax1[n1]);
	n1=n1+1;
	if(m<mmin1) mmin1=m;
	if(wl1<wlmin1) wlmin1=wl1;
	if(m>mmax1) mmax1=m;
	if(wl4096>wlmax1) wlmax1=wl4096;
	if(yint0<1024){
	  mmid1=m ;
	  wlmid1=wl2048;
	  wl_pix1=(wl4096-wl1)/4096.;
	}
      }
      else {
	order2[n2]=m;
	ypixmin2[n2]=-(int)(ccdgap)-yintmin;
	ypixcen2[n2]=-(int)(ccdgap)-yint0;  
	ypixmax2[n2]=-(int)(ccdgap)-yintmax;
	wlnmmin2[n2]=wl1;
	wlnmcen2[n2]=wl2048;
	wlnmmax2[n2]=wl4096;
	//      write(6,101)order2[n2],ypixmin2[n2],ypixcen2[n2],
	//&       ypixmax2[n2],wlnmmin2[n2],wlnmcen2[n2],wlnmmax2[n2]
	ypix2[1][n2]=(double)(ypixmin2[n2]);
	ypix2[2][n2]=(double)(ypixcen2[n2]);
	ypix2[3][n2]=(double)(ypixmax2[n2]);
	n2=n2+1;
	if(m<mmin2) mmin2=m;
	if(wl1<wlmin2) wlmin2=wl1;
	if(m>mmax2) mmax2=m;
	if(wl4096>wlmax2) wlmax2=wl4096;
	if(yint0<-1024){
	  mmid2=m; 
	  wlmid2=wl2048;
	  wl_pix2=(wl4096-wl1)/4096.;
	}
      }
    }
     m--;
  }

  // 22   continue

  n1end=n1-1;
  n2end=n2-1;
  mout[1]=mmin1;
  mout[2]=mmid1;
  mout[3]=mmax1;
  mout[4]=mmin2;
  mout[5]=mmid2;
  mout[6]=mmax2;
  wlout[1]=wlmin1;
  wlout[2]=wlmid1;
  wlout[3]=wlmax1;
  wlout[4]=wl_pix1;
  wlout[5]=wlmin2;
  wlout[6]=wlmid2;
  wlout[7]=wlmax2;
  wlout[8]=wl_pix2;
}


// Create EFS Window
void close_efs(GtkWidget *w, gpointer gdata)
{
  hds_param *hds=(hds_param *)gdata;

  gtk_widget_destroy(GTK_WIDGET(hds->efs_main));
  flagEFS=FALSE;
}


void go_efs(hds_param *hds){
  if(flagEFS){
    gdk_window_raise(gtk_widget_get_window(hds->efs_main));
    draw_efs_cairo(hds->efs_dw,hds);
    return;
  }
  else{
    flagEFS=TRUE;
  }
  
  create_efs_dialog(hds);
}

void call_efs(GtkWidget *w, gpointer gdata){
  hds_param *hds=(hds_param *)gdata;

  go_efs(hds);
}


void create_efs_dialog(hds_param *hds)
{
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox1;
  GtkWidget *frame, *check, *label, *button;
  GSList *group=NULL;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;


  hds->efs_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hds->efs_main), "HDS Monitor : Echelle Format Simulator");
  
  g_signal_connect(hds->efs_main,
		   "destroy",
		   G_CALLBACK(close_efs), 
		   (gpointer)hds);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hds->efs_main), vbox);

  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);


  frame = gtkut_frame_new ("<b>Mode</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Echelle Format Simulator",
		       1, EFS_PLOT_EFS, -1);
    if(hds->efs_mode==EFS_PLOT_EFS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Free Spectral Range",
		       1, EFS_PLOT_FSR, -1);
    if(hds->efs_mode==EFS_PLOT_FSR) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",
		      G_CALLBACK(cc_get_efs_mode),
		      (gpointer)hds);
  }

  frame = gtkut_frame_new ("<b>CCD</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  {
    gint i_bin;
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    for(i_bin=0; i_bin<NUM_BIN; i_bin++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, bin_name[i_bin],
			 1, i_bin, -1);
      if(hds->efs_bin==i_bin) iter_set=iter;
    }
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",
		      G_CALLBACK(cc_get_efs_bin),
		      (gpointer)hds);
  }

  frame = gtkut_frame_new ("<b>Act.</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (refresh_efs), (gpointer)hds);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Refresh");
#endif


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"window-close");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_efs), (gpointer)hds);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Quit");
#endif

 
  // Drawing Area
  hds->efs_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hds->efs_dw, EFS_WIDTH, EFS_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), hds->efs_dw, TRUE, TRUE, 0);
  gtk_widget_set_app_paintable(hds->efs_dw, TRUE);
  gtk_widget_show(hds->efs_dw);

#ifdef USE_GTK3
  g_signal_connect(hds->efs_dw, 
		   "draw", 
		   G_CALLBACK(draw_efs_cb),
		   (gpointer)hds);
#else
  g_signal_connect(hds->efs_dw, 
		   "expose-event", 
		   G_CALLBACK(expose_efs_cairo),
		   (gpointer)hds);
#endif
  g_signal_connect(hds->efs_dw, 
		   "configure-event", 
		   G_CALLBACK(configure_efs_cb),
		   (gpointer)hds);

  gtk_widget_show_all(hds->efs_main);

  gdk_window_raise(gtk_widget_get_window(hds->efs_main));

  draw_efs_cairo(hds->efs_dw,hds); 
}

#ifdef USE_GTK3
gboolean draw_efs_cb(GtkWidget *widget,
		     cairo_t *cr, 
		     gpointer userdata){
  hds_param *hds=(hds_param *)userdata;
  if(!pixbuf_efs) draw_efs_cairo(widget,hds);
  gdk_cairo_set_source_pixbuf(cr, pixbuf_efs, 0, 0);
  cairo_paint(cr);
  return(TRUE);
}
#else
gboolean expose_efs_cairo(GtkWidget *widget,
			  GdkEventExpose *event, 
			  gpointer userdata){
  hds_param *hds=(hds_param *)userdata;
  if(!pixmap_efs) draw_efs_cairo(hds->efs_dw,hds);
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    GtkStyle *style=gtk_widget_get_style(widget);
    gtk_widget_get_allocation(widget,allocation);

    gdk_draw_drawable(gtk_widget_get_window(widget),
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_efs,
		      0,0,0,0,
		      allocation->width,
		      allocation->height);
    g_free(allocation);
  }

  return (TRUE);
}
#endif

gboolean configure_efs_cb(GtkWidget *widget,
			  GdkEventConfigure *event, 
			  gpointer userdata){
  hds_param *hds=(hds_param *)userdata;
  draw_efs_cairo(widget,hds);
  return(TRUE);
}

gboolean draw_efs_cairo(GtkWidget *widget, hds_param *hds){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint from_set, to_rise;
  double dx,dy,lx,ly;
  int width, height;

  if(!flagEFS) return (FALSE);

  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget,allocation);
    
    width= allocation->width;
    height= allocation->height;
    g_free(allocation);
  }

  if(width<=1){
    gtk_window_get_size(GTK_WINDOW(widget), &width, &height);
  }
  dx=width*0.1;
  dy=height*0.1;
  lx=width*0.8;
  ly=height*0.8;

#ifdef USE_GTK3
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				       width, height);
  
  cr = cairo_create(surface);
#else
  if(pixmap_efs) g_object_unref(G_OBJECT(pixmap_efs));
  pixmap_efs = gdk_pixmap_new(gtk_widget_get_window(widget),
			      width,
			      height,
			      -1);
  
  cr = gdk_cairo_create(pixmap_efs);
#endif
  
  cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);

  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);


  // from efs_main
  {
    int  i, line_max;
    char buf[BUFFER_SIZE],buf2[BUFFER_SIZE];
    int  dummy;
    char *c;
    char tmp[BUFFER_SIZE];
    double rad;
    double cos_gam_C;
    double a_C,a_CB, a_CR;
    double theta_CB0, theta_CR0;
    double fcam;
    double pixsize;
    double d_det_rot, d_theta_CB, d_theta_CR, d_theta_E_mes;
    int   ncross;
    char  cross[2];
    int   howto;
    
    int nlines;
    double wlc, theta_C, wl_center, theta_E_mes, det_rot, aaa, theta_C0;
    char th_ec[30],crossrot[30];
    
    
    int nonstd_iset=-1;
    
    rad=3.14159/180.0;
    //cos_gam_C=0.92388;   // cos(gamma_c)=cos(22.5deg)
    cos_gam_C=cos((22.5-0./3600.)*rad);
    a_CB=1./4000;
    a_CR=1./2500;
    theta_CB0=4.76;
    theta_CR0=5.00;
    fcam=77.0;
    pixsize=0.00135;
    //d_det_rot=0.1;
    d_det_rot=0.1+2800./3600.; // After EQ
    //d_theta_CB=0.06;
    d_theta_CB=0.06-20./3600.;  // After EQ
    //d_theta_CR=0.11;
    d_theta_CR=0.11-20./3600.;  // After EQ
    //d_theta_E_mes=0.0;
    //d_theta_E_mes=550.0/3600.0;  // After EQ
    //d_theta_E_mes=600.0/3600.0;  // After EQ on 2010.4
    d_theta_E_mes=590.0/3600.0;  // After EQ on 2011.10
    
    

    // Read Line List
    for(i=0;i<=MAX_LINE;i++){
      wl[i]=0.0;
    }

    line_max=0;
    for(i=0;i<MAX_LINE;i++){
      if((line_data[i].name)&&(line_data[i].wave>0)){
	strcpy(line_name[line_max+1],line_data[i].name);
	wl[line_max+1]=(double)line_data[i].wave/10.;
	line_max++;
      }
      else{
	break;
      }
    }
    
    nlines=line_max;
    
    // Cross Disperser
    if(hds->now.col_col==COL_BLUE){
      a_C=a_CB;
      ncross=CROSS_BLUE;
    }
    else{
      a_C=a_CR;
      ncross=CROSS_RED;
    }
    
    // Cross Angle(=-1), WaveLength(=1)
    howto=-1;

    if(hds->now.col_col==COL_BLUE){
      theta_C=(gdouble)hds->now.col_scan-(gdouble)hds->dcr_b/60./60.;
    }
    else{
      theta_C=(gdouble)hds->now.col_scan-(gdouble)hds->dcr_r/60./60.;
    }
    theta_E_mes=(gdouble)hds->now.col_echelle;
    det_rot=(double)hds->now.cam_rotate;

    sprintf(th_ec, "Echelle=%.4lf[deg]",theta_E_mes);
    theta_E_mes=theta_E_mes+d_theta_E_mes;
    theta_E_mes=theta_E_mes-0.425;
    
    
    det_rot= det_rot - d_det_rot;
    
    if(hds->now.col_col==COL_BLUE){
      sprintf(crossrot,"Cross-rot=%.4lf[deg]",theta_C+(gdouble)hds->dcr_b/60./60.);
    }else{
      sprintf(crossrot,"Cross-rot=%.4lf[deg]",theta_C+(gdouble)hds->dcr_r/60./60.);
    }

    if(ncross==CROSS_BLUE){
      theta_C=theta_C-d_theta_CB;
    }
    else{
      theta_C=theta_C-d_theta_CR;
    }



    efs(ncross, theta_E_mes,theta_C, det_rot, nlines);
    

    // plot
    {
      char tmp[BUFFER_SIZE];
      double xmin0, xmax0, ymin0, ymax0;
      int   i,n,j;
      double x[4],y1[4],y2[4],y[4];
      double old_y2;
      double min_sep;
      float lenx[5],leny[5];
      double y1_1, y1_3, y3_1, y3_3, y2_1, y2_3, y4_1, y4_3, y2_2, y1_2;
      double xt_title, yt_title;
      double xt_cr, xt_crs, yt_cr, xt_ec, yt_ec, xt_det;
      int   i1, i2;
      double xccd_min, xccd_max, yccd1_min, yccd1_max, yccd2_min, yccd2_max;
      double xt_wave, xt_ord, yt_ord;
      int   imod;
      double xt_o, yt_o, xt_w, yt_w,ytext;
      char  order[10], wave[10], text[15][10];
      double fcam=77.0;
      double rad=3.14159/180.0;
      double pixsize=0.00135;
      // double d_det_rot=0.1;
      double d_det_rot=0.1+2800./3600.;  // After EQ
      double slit_pix;
      char line_txt[21][256];
      int line_flag[21];
      gdouble ccdgap=43.;
      gdouble rx, ry, xd, yd;
      gdouble yobj1, yobj2;
      
      
      // ### PLOT ###
      
      if(hds->efs_mode==EFS_PLOT_EFS){
	xmin0=000.;
	ymin0=-500.;
	xmax0=6500.;
	ymax0=5000.;
	
	rx=(gdouble)width/(xmax0-xmin0); 
	ry=(gdouble)height/(ymax0-ymin0);
	xd=dx/2;
	yd=height-dy;
      }
      else {// FSR
	if(ncross==CROSS_RED){
	  xmax0=xfmax[56]+100.;
	  ymax0=yfmax[56]+100;
	  xmin0=xfmin[56]-100.;
	  ymin0=yfmin[160]-100;
	}
	else{
	  xmax0=xfmax[100]+1000.;
	  ymax0=yfmax[100]+100;
	  xmin0=xfmin[100]-1000.;
	  ymin0=yfmin[199]-100;
	}
	rx=(gdouble)width/(xmax0-xmin0)*0.5; 
	ry=(gdouble)height/(ymax0-ymin0)*0.85;
	xd=dx*5;
	yd=height-dy/2.;
      }
      
    
      for(i=1;i<=8;i++){
	sprintf(text[i],"%.3lf",wlout[i]);
      }	
      for(i=1;i<=6;i++){
	sprintf(text[i+8],"%d",mout[i]);
      }	
      
      if(hds->efs_mode==EFS_PLOT_EFS){
	// Slit Length
	switch(hds->now.is_unit){
	case 1:
	  slit_pix=(8.4)/0.138;
	  break;
	case 2:
	  slit_pix=(4.8)/0.138;
	  break;
	case 3:
	  slit_pix=(5.0)/0.138;
	  break;
	default:
	  slit_pix=(double)hds->now.slit_length/0.138;
	  break;
	}
	
	cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	
	// CCD Chip
	{
	  
	  cairo_rectangle(cr,nx(0.,rx,xd),ny(2005.,ry,yd),4096.*rx,2048.*ry);
	  cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	  cairo_set_line_width(cr,2.0);
	  cairo_stroke(cr);
	  cairo_rectangle(cr,nx(0.,rx,xd),ny(2005.,ry,yd),4096.*rx,2048.*ry);
	  cairo_set_source_rgba(cr, 0.95, 0.95, 1.0, 1.0);
	  cairo_fill(cr);
	  
	  cairo_rectangle(cr,nx(0.,rx,xd),ny(4139.,ry,yd),4096.*rx,2048.*ry);
	  cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	  cairo_set_line_width(cr,2.0);
	  cairo_stroke(cr);
	  cairo_rectangle(cr,nx(0.,rx,xd),ny(4139.,ry,yd),4096.*rx,2048.*ry);
	  cairo_set_source_rgba(cr, 1.0, 0.95, 0.95, 1.0);
	  cairo_fill(cr);

	  // 0.15um pitch (Just for CCD replacement test)
	  /*
	    cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(2005.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
	    cairo_set_line_width(cr,2.0);
	    cairo_stroke(cr);
	    cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(2005.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	    cairo_set_source_rgba(cr, 0.95, 0.95, 1.0, 0.5);
	    cairo_fill(cr);
	    
	    cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(4139.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
	    cairo_set_line_width(cr,2.0);
	    cairo_stroke(cr);
	    cairo_rectangle(cr,nx(0.,rx*0.15/0.135,xd),ny(4139.,ry*0.15/0.135,yd),4096.*rx*0.15/0.135,2048.*ry*0.15/0.135);
	    cairo_set_source_rgba(cr, 1.0, 0.95, 0.95, 0.5);
	    cairo_fill(cr);
	  */
	}
	
	x[0]=1.;
	x[1]=2048.;
	x[2]=4096.;
	for(n=1;n<=n1end;n++){
	  y1[0]=4096-ypix1[1][n]+43.;
	  y1[1]=4096-ypix1[2][n]+43.;
	  y1[2]=4096-ypix1[3][n]+43.;
	  if(n==1)     y1_1=y1[0];
	  if(n==1)     y3_1=y1[2];
	  if(n==n1end) y1_3=y1[0];
	  if(n==n1end) y3_3=y1[2];
	  
	  lenx[0]=x[0];
	  lenx[1]=x[0];
	  lenx[2]=x[2];
	  lenx[3]=x[2];
	  
	  leny[0]=y1[0]-slit_pix/2;
	  leny[1]=y1[0]+slit_pix/2;
	  leny[2]=y1[2]+slit_pix/2;
	  leny[3]=y1[2]-slit_pix/2;
	  
	  cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.2);
	  cairo_set_line_width(cr,slit_pix*ry);
	  cairo_move_to(cr,nx(x[0],rx,xd),ny(y1[0],ry,yd));
	  cairo_line_to(cr,nx(x[2],rx,xd),ny(y1[2],ry,yd));
	  cairo_stroke(cr);
	  
	  cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
	  cairo_set_line_width(cr,1.0);
	  cairo_move_to(cr,nx(x[0],rx,xd),ny(y1[0],ry,yd));
	  cairo_line_to(cr,nx(x[2],rx,xd),ny(y1[2],ry,yd));
	  cairo_stroke(cr);
	  
	}
	
	for(n=1;n<=n2end;n++){
	  y2[0]=2048-ypix2[1][n]-43.;
	  y2[1]=2048-ypix2[2][n]-43.;
	  y2[2]=2048-ypix2[3][n]-43.;
	  if(n==1)     y2_1=y2[0];
	  if(n==n2end) y2_3=y2[0];
	  if(n==1)     y4_1=y2[2];
	  if(n==n2end) y4_3=y2[2];
	  
	  lenx[0]=x[0];
	  lenx[1]=x[0];
	  lenx[2]=x[2];
	  lenx[3]=x[2];
	  
	  leny[0]=y2[0]-slit_pix/2;
	  leny[1]=y2[0]+slit_pix/2;
	  leny[2]=y2[2]+slit_pix/2;
	  leny[3]=y2[2]-slit_pix/2;
	  
	  cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.2);
	  cairo_set_line_width(cr,slit_pix*ry);
	  cairo_move_to(cr, nx(x[0],rx,xd),ny(y2[0],ry,yd));
	  cairo_line_to(cr, nx(x[2],rx,xd),ny(y2[2],ry,yd));
	  cairo_stroke(cr);
	  cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
	  cairo_set_line_width(cr,1.0);
	  cairo_move_to(cr, nx(x[0],rx,xd),ny(y2[0],ry,yd));
	  cairo_line_to(cr, nx(x[2],rx,xd),ny(y2[2],ry,yd));
	  cairo_stroke(cr);
	  
	  if(n==1){
	    old_y2=y2[2];
	  }
	  else if(n==2){
	    min_sep=((y2[2]-slit_pix/2)-(old_y2+slit_pix/2))
	      /(gdouble)hds->bin_x;
	  }
	}      
	cairo_set_font_size (cr, 10.0);
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	switch(hds->now.is_unit){
	case 1:
	  sprintf(tmp,"Minimum order gap is %.2lf pix w/0\".30x5 Image Slicer",
		  min_sep);
	  break;
	case 2:
	  sprintf(tmp,"Minimum order gap is %.2lf pix w/0\".45x3 Image Slicer",
		  min_sep);
	  break;
	case 3:
	  sprintf(tmp,"Minimum order gap is %.2lf pix w/0\".20x3 Image Slicer",
		  min_sep);
	  break;
	default:
	  sprintf(tmp,"Minimum order gap is %.2lf pix w/%.2f\" slit length.",
		  min_sep,(float)hds->now.slit_length);
	}
	if (min_sep<5){
	  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	}
	else{
	  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.8);
	}
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr, dx, height-extents.height);
	cairo_show_text(cr, tmp);
	
	//      badcolumn in R
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.7);
	cairo_set_line_width(cr,1.5);
	
	cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1106.+2091.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1106.+2091.,ry,yd));
	cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1111.+2091.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1111.+2091.,ry,yd));
	
	cairo_move_to(cr, nx(1446.,rx,xd),ny(2048.- 938.+2091.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 938.+2091.,ry,yd));
	cairo_move_to(cr, nx(1446.,rx,xd),ny(2048.- 943.+2091.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 943.+2091.,ry,yd));
	
	//      badcolumn in B
	cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-  91.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-  91.-43.,ry,yd));
	cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.- 128.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 128.-43.,ry,yd));
	
	cairo_move_to(cr, nx(2244.,rx,xd),ny(2048.- 359.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.- 359.-43.,ry,yd));
	
	cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1721.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1721.-43.,ry,yd));
	cairo_move_to(cr, nx(   0.,rx,xd),ny(2048.-1742.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1742.-43.,ry,yd));
	
	cairo_move_to(cr, nx(2711.,rx,xd),ny(2048.-1921.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1921.-43.,ry,yd));
	cairo_move_to(cr, nx(3118.,rx,xd),ny(2048.-1958.-43.,ry,yd));
	cairo_line_to(cr, nx(4096.,rx,xd),ny(2048.-1958.-43.,ry,yd));
	
	cairo_stroke(cr);
	
	
	// Wavelength
	y2_2=1024.;
	y1_2=3072.;
	
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 11.0);
	cairo_set_line_width(cr, 2.0);
	
	cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(-43.,ry,yd)-10.);
	cairo_text_path(cr, text[5]);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(-43.,ry,yd)-10.);
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,text[5]);
	
	cairo_move_to(cr,nx(1750.,rx,xd),ny(y2_2,ry,yd));
	cairo_text_path(cr, text[6]);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(1750.,rx,xd),ny(y2_2,ry,yd));
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,text[6]);
	
	cairo_text_extents (cr, text[7], &extents);
	cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		      ny(2005.,ry,yd)+extents.height+10.);
	cairo_text_path(cr, text[7]);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		      ny(2005.,ry,yd)+extents.height+10.);
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,text[7]);
	
	cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(2091.,ry,yd)-10.);
	cairo_text_path(cr, text[1]);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(0.,rx,xd)+10.,ny(2091.,ry,yd)-10.);
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,text[1]);
	
	cairo_move_to(cr,nx(1750.,rx,xd),ny(y1_2,ry,yd));
	cairo_text_path(cr, text[2]);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(1750.,rx,xd),ny(y1_2,ry,yd));
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,text[2]);
	
	cairo_text_extents (cr, text[3], &extents);
	cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		      ny(4139.,ry,yd)+extents.height+10.);
	cairo_text_path(cr, text[3]);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(4096.,rx,xd)-10.-extents.width,
		      ny(4139.,ry,yd)+extents.height+10.);
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,text[3]);
	
	
	y1_2=3000.;
	y2_2=1000.;
	
	// ORDER
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	
	cairo_text_extents (cr, text[14], &extents);
	cairo_move_to(cr,xd-extents.width-5,ny(y2_1,ry,yd));
	cairo_show_text(cr,text[14]);
	
	cairo_text_extents (cr, text[12], &extents);
	cairo_move_to(cr,xd-extents.width-5,ny(y2_3,ry,yd));
	cairo_show_text(cr,text[12]);
	
	cairo_text_extents (cr, text[11], &extents);
	cairo_move_to(cr,xd-extents.width-5,ny(y1_1,ry,yd));
	cairo_show_text(cr,text[11]);
	
	cairo_text_extents (cr, text[9], &extents);
	cairo_move_to(cr,xd-extents.width-5,ny(y1_3,ry,yd));
	cairo_show_text(cr,text[9]);
	
	cairo_text_extents (cr, "WAVELENGTH (nm)", &extents);
	cairo_move_to(cr,nx(4096.,rx,xd)-extents.width,ny(4200.,ry,yd));
	cairo_show_text(cr,"WAVELENGTH (nm)");
	
	cairo_text_extents (cr, "ORDER", &extents);
	cairo_move_to(cr,xd-extents.width/2.,ny(4200.,ry,yd));
	cairo_show_text(cr,"ORDER");
	
	cairo_text_extents (cr, "CCD-1", &extents);
	cairo_move_to(cr,nx(0.,rx,xd)+10., ny(4139.,ry,yd)+10.+extents.height);
	cairo_text_path(cr,"CCD-1");
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(0.,rx,xd)+10., ny(4139.,ry,yd)+10.+extents.height);
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,"CCD-1");
	
	cairo_text_extents (cr, "CCD-2", &extents);
	cairo_move_to(cr,nx(0.,rx,xd)+10., ny(2005.,ry,yd)+10.+extents.height);
	cairo_text_path(cr,"CCD-2");
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	cairo_stroke(cr);
	cairo_move_to(cr,nx(0.,rx,xd)+10., ny(2005.,ry,yd)+10.+extents.height);
	cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
	cairo_show_text(cr,"CCD-2");
	
	xt_title=0.;
	yt_title=4700.;
	xt_cr=500.;
	xt_crs=2000.;
	yt_cr=4500.;
	xt_ec=500.;
	yt_ec=4350.;
	xt_det=2000.;
      }
      else {
	if(ncross==CROSS_RED){
	  i1=56;
	  i2=160;
	}
	else{
	  i1=100;
	  i2=199;
	}
	
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_rectangle(cr,nx(xmin0,rx,xd), ny2(ymin0,ry,yd,ymin0),
			(xmax0-xmin0)*rx,(ymin0-ymax0)*ry);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(xmin0,rx,xd), ny2(ymin0,ry,yd,ymin0),
			(xmax0-xmin0)*rx,(ymin0-ymax0)*ry);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_fill(cr);
	
	cairo_set_line_width(cr,1.0);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	
	for(i=i1;i<=i2;i++){
	  x[0]=xfmin[i];
	  y[0]=yfmin[i];
	  x[1]=xfmax[i];
	  y[1]=yfmax[i];
	  //cpgline(2,(float *)x,(float *)y);
	  cairo_move_to(cr,nx(x[0],rx,xd), ny2(y[0],ry,yd,ymin0));
	  cairo_line_to(cr,nx(x[1],rx,xd), ny2(y[1],ry,yd,ymin0));
	  cairo_stroke(cr);
	}
	
	xccd_min=-2050.+fcam*rad*theta_E_mes/pixsize;
	xccd_max=2050.+fcam*rad*theta_E_mes/pixsize;
	yccd1_min=-2091.;
	yccd1_max=-43.;
	yccd2_min=-yccd1_min;
	yccd2_max=-yccd1_max;
	
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd1_max,ry,yd,ymin0),
			(xccd_max-xccd_min)*rx,(yccd1_max-yccd1_min)*ry);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd1_max,ry,yd,ymin0),
			(xccd_max-xccd_min)*rx,(yccd1_max-yccd1_min)*ry);
	cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
	cairo_fill(cr);
	
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd2_max,ry,yd,ymin0),
			(xccd_max-xccd_min)*rx,(yccd2_max-yccd2_min)*ry);
	cairo_set_line_width(cr,2.0);
	cairo_stroke(cr);
	cairo_rectangle(cr,nx(xccd_min,rx,xd),ny2(yccd2_max,ry,yd,ymin0),
			(xccd_max-xccd_min)*rx,(yccd2_max-yccd2_min)*ry);
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
	cairo_fill(cr);
	
	// New Chip 0.15um
	/*
	  cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
	  cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd1_max,ry*0.15/0.135,yd,ymin0),
	  (xccd_max-xccd_min)*rx*0.15/0.135,(yccd1_max-yccd1_min)*ry*0.15/0.135);
	  cairo_set_line_width(cr,2.0);
	  cairo_stroke(cr);
	  cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd1_max,ry*0.15/0.135,yd,ymin0),
	  (xccd_max-xccd_min)*rx*0.15/0.135,(yccd1_max-yccd1_min)*ry*0.15/0.135);
	  cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
	  cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd2_max,ry*0.15/0.135,yd,ymin0),
	  (xccd_max-xccd_min)*rx*0.15/0.135,(yccd2_max-yccd2_min)*ry*0.15/0.135);
	  cairo_set_line_width(cr,2.0);
	  cairo_stroke(cr);
	  cairo_rectangle(cr,nx(xccd_min,rx*0.15/0.135,xd),ny2(yccd2_max,ry*0.15/0.135,yd,ymin0),
	  (xccd_max-xccd_min)*rx*0.15/0.135,(yccd2_max-yccd2_min)*ry*0.15/0.135);
	  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
	  cairo_fill(cr);
	*/
	
	xt_wave=xmax0-500.;
	xt_ord=xmin0-500.;
	yt_ord=ymax0+100.;
	
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 11.0);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	
	cairo_text_extents (cr, "WAVELENGTH (nm)", &extents);
	cairo_move_to(cr,nx(xmax0,rx,xd)-extents.width/2.,ny2(ymax0,ry,yd,ymin0)-extents.height/2.);
	cairo_show_text(cr,"WAVELENGTH (nm)");
	
	cairo_text_extents (cr, "ORDER", &extents);
	cairo_move_to(cr,nx(xmin0,rx,xd)-extents.width/2.,ny2(ymax0,ry,yd,ymin0)-extents.height/2.);
	cairo_show_text(cr,"ORDER");
	
	cairo_text_extents (cr, "CCD-1", &extents);
	cairo_move_to(cr,nx(xccd_min,rx,xd)-extents.width-5.,ny2(1000.,ry,yd,ymin0));
	cairo_show_text(cr,"CCD-1");
	
	cairo_text_extents (cr, "CCD-2", &extents);
	cairo_move_to(cr,nx(xccd_min,rx,xd)-extents.width-5.,ny2(-1000.,ry,yd,ymin0));
	cairo_show_text(cr,"CCD-2");
	
	cairo_set_font_size (cr, 10.0);
	
	for(i=i1;i<=i2;i++){
	  imod=i-(int)(i/10)*10;
	  if(imod==0){
	    xt_o=xfmin[i];
	    yt_o=yfmin[i];
	    xt_w=xfmax[i];
	    yt_w=yfmax[i];
	    
	    sprintf(order,"%d",i);
	    sprintf(wave,"%6.1lf",wlfmax[i]*1.e+7);
	    
	    cairo_text_extents (cr, order, &extents);
	    cairo_move_to(cr,nx(xt_o,rx,xd)-extents.width-5.,ny2(yt_o,ry,yd,ymin0));
	    cairo_show_text(cr,order);
	    
	    cairo_move_to(cr,nx(xt_w,rx,xd),ny2(yt_w,ry,yd,ymin0));
	    cairo_show_text(cr,wave);
	  }
	}     

	xt_cr=xmin0-500.;
	xt_crs=xmin0+3000.;
	yt_cr=ymax0+1000.;
	xt_ec=xt_cr-1000;
	yt_ec=ymax0+600.;
	xt_det=xt_crs;
	xt_title=xmin0-700.;
	yt_title=ymax0+1500.;
      }

      // ### Setting: ###
      if(hds->now.setting==0){  // None Std
	sprintf(tmp,"Non-Standard %dx%dbin", hds->bin_x, hds->bin_y);
      }
      else{
	sprintf(tmp,"Std%s %dx%dbin",
		setting_name[hds->now.setting],
		hds->bin_x, hds->bin_y);
      }
      cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size (cr, 16.0);
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      
      cairo_text_extents (cr, tmp, &extents);
      cairo_move_to(cr,dx,extents.height+5);
      cairo_show_text(cr,tmp);
      
      cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0);
      
      cairo_move_to(cr,dx,extents.height+5);
      
      if(ncross==CROSS_RED){
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	cairo_text_extents (cr, "Cross=RED", &extents);
	cairo_rel_move_to(cr,extents.width/5.,extents.height+5);
	cairo_show_text(cr,"Cross=RED");
      }
      else{
	cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
	cairo_text_extents (cr, "Cross=BLUE", &extents);
	cairo_rel_move_to(cr,extents.width/5.,extents.height+5);
	cairo_show_text(cr,"Cross=BLUE");
      }
      
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      sprintf(tmp," / %s / %s / Cam-rot=%.4f[deg]",
	      crossrot,
	      th_ec,
	      det_rot+d_det_rot);
      cairo_show_text(cr,tmp);
      
      //   plot lines specified
      if(hds->efs_mode==EFS_PLOT_EFS){
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.8, 1.0);
	
	for(i=1;i<=nlines;i++){
	  line_flag[i]=0;
	  sprintf(line_txt[i],"%7.2lfA",wl[i]*10);
	}
	
	for(j=1;j<=3;j++){
	  for(i=1;i<=nlines;i++){
	    xpt[i]=xline[i][j];
	    if(yline1[i][j]>0.0){
	      ypt[i]=4096.-yline1[i][j]+ccdgap;
	      if(xpt[i]>0){
		if(line_flag[i]==0){
		  sprintf(tmp," : CCD1(%4.0lf, %4.0lf)",
			  ((4096+ccdgap)-ypt[i])/(gdouble)hds->bin_x,
			  xpt[i]/(gdouble)hds->bin_y);
		}
		else{
		  sprintf(tmp,"(%4.0lf, %4.0lf)",
			  ((4096+ccdgap)-ypt[i])/(gdouble)hds->bin_x,
			  xpt[i]/(gdouble)hds->bin_y);
		}
		strcat(line_txt[i],tmp);
		line_flag[i]++;
	      }
	    }
	    else if(yline2[i][j]>0.0){
	      ypt[i]=2048.-yline2[i][j]-ccdgap;
	      if(xpt[i]>0){
		if(line_flag[i]==0){
		  sprintf(tmp," : CCD2(%4.0lf, %4.0lf)",
			  ((2048-ccdgap)-ypt[i])/(gdouble)hds->bin_x,
			  xpt[i]/(gdouble)hds->bin_y);
		}
		else{
		  sprintf(tmp,"(%4.0lf, %4.0lf)",
			  ((2048-ccdgap)-ypt[i])/(gdouble)hds->bin_x,
			  xpt[i]/(gdouble)hds->bin_y);
		}
		strcat(line_txt[i],tmp);
		line_flag[i]++;
	      }
	    }
	  }
	  
	  cairo_set_line_width(cr,2.0);
	  
	  for(i=1;i<=nlines;i++){
	    sprintf(tmp,"%d",i);
	    
	    cairo_text_extents (cr, tmp, &extents);
	    cairo_move_to(cr, nx(xpt[i],rx,xd)-extents.width-5.,ny(ypt[i],ry,yd));
	    cairo_text_path(cr,tmp);
	    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
	    cairo_stroke(cr);
	    cairo_move_to(cr, nx(xpt[i],rx,xd)-extents.width-5.,ny(ypt[i],ry,yd));
	    cairo_set_source_rgba(cr, 0.0, 0.0, 0.8, 1.0);
	    cairo_show_text(cr,tmp);
	    cairo_arc(cr, 
		      nx(xpt[i],rx,xd),ny(ypt[i],ry,yd),
		      3,0, 2 * M_PI);
	    cairo_fill(cr);
	    
	  }
	  
	}
	
	ytext=0.;
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 13.0);
	cairo_text_extents (cr, "Line Name", &extents);
	yobj1=extents.height;
	
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 10.0);
	cairo_text_extents (cr, "Pixel Value", &extents);
	yobj2=extents.height;
      
	
	for(i=1;i<=nlines;i++){
	  ytext=ytext+yobj1+7.;
	  sprintf(tmp,"%2d. %s",i,line_name[i]);
	  cairo_move_to(cr,nx(4150.,rx,xd),ny(4700.,ry,yd)+ytext);
	  cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size (cr, 13.0);
	  cairo_show_text(cr,tmp);
	  
	  ytext=ytext+yobj2+4.;
	  cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_NORMAL);
	  cairo_set_font_size (cr, 10.0);
	  cairo_move_to(cr,nx(4250.,rx,xd),ny(4700.,ry,yd)+ytext);
	  if(line_flag[i]!=0){
	    cairo_show_text(cr,line_txt[i]);
	  }
	  else{
	    sprintf(line_txt[i],"%7.2lfA : ---- ",wl[i]*10);
	    cairo_show_text(cr,line_txt[i]);
	  }
	}
      }
    }
  }
    
  cairo_destroy(cr);
  
#ifdef USE_GTK3
  if(pixbuf_efs) g_object_unref(G_OBJECT(pixbuf_efs));
  pixbuf_efs=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
  cairo_surface_destroy(surface);
  gtk_widget_queue_draw(widget);
#else
  {
    GtkStyle *style=gtk_widget_get_style(widget);
    gdk_draw_drawable(gtk_widget_get_window(widget),
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_efs,
		      0,0,0,0,
		      width,
		      height);
  }
  
  //g_object_unref(G_OBJECT(pixmap_efs));
#endif

  return TRUE;
}



gdouble nx(gdouble x, gdouble rx, gdouble xd){
  return(x*rx+xd);
}

gdouble ny(gdouble y, gdouble ry, gdouble yd){
  gdouble rev_y=y*ry;

  return(-rev_y+yd);
}

gdouble ny2(gdouble y, gdouble ry, gdouble yd, gdouble y0){
  gdouble rev_y=(y-y0)*ry;

  return(-rev_y+yd);
}

static void refresh_efs (GtkWidget *widget, gpointer data)
{
  hds_param *hds = (hds_param *)data;

  if(flagEFS){
    draw_efs_cairo(hds->efs_dw,hds);
  }
}



void cc_get_efs_mode (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  hds_param *hds=(hds_param *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hds->efs_mode=n;

    refresh_efs(widget, hds);
  }
}


void cc_get_efs_bin (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  hds_param *hds=(hds_param *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hds->efs_bin=n;

    switch(hds->efs_bin){
    case BIN_21:
      hds->bin_x=2;
      hds->bin_y=1;
      break;

    case BIN_22:
      hds->bin_x=2;
      hds->bin_y=2;
      break;

    case BIN_24:
      hds->bin_x=2;
      hds->bin_y=4;
      break;

    case BIN_41:
      hds->bin_x=4;
      hds->bin_y=1;
      break;

    case BIN_44:
      hds->bin_x=4;
      hds->bin_y=4;
      break;

    default:
      hds->bin_x=1;
      hds->bin_y=1;
      break;
    }

    refresh_efs(widget, hds);
  }
}

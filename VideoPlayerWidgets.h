#ifndef VideoPlayerWidgetsH
#define VideoPlayerWidgetsH

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <sqlite3.h>

#include "AudioMixer.h"
#include "VideoPlayer.h"
#include "BiQuad.h"

typedef struct
{
GtkWidget *vpwindow;
GtkWidget *box1;
GtkWidget *horibox;
GtkWidget *dwgarea;
GtkWidget *button_box;
GtkWidget *button1;
GtkWidget *button2;
GtkWidget *button8;
GtkWidget *button9;
GtkWidget *button10;
GtkWidget *buttonParameters;
GtkAdjustment *hadjustment;
GtkWidget *hscale;
GtkWidget *stackswitcher;
GtkWidget *stack;
GtkWidget *playerbox;
GtkWidget *box2;
GtkWidget *playlistbox;
GtkWidget *button_box2;
GtkWidget *button3;
GtkWidget *button4;
GtkWidget *button6;
GtkWidget *button7;
GtkWidget *listview;
GtkListStore *store;
GtkTreeIter iter;
GtkWidget *scrolled_window;
GtkWidget *statusbar;
gint context_id;
GtkWidget *windoweq;
GtkWidget *eqvbox;
GtkWidget *eqbox;
GtkAdjustment *vadj0;
GtkAdjustment *vadj1;
GtkAdjustment *vadj2;
GtkAdjustment *vadj3;
GtkAdjustment *vadj4;
GtkAdjustment *vadj5;
GtkAdjustment *vadj6;
GtkAdjustment *vadj7;
GtkAdjustment *vadj8;
GtkAdjustment *vadj9;
GtkAdjustment *vadjA;
GtkWidget *vscaleeq0;
GtkWidget *vscaleeq1;
GtkWidget *vscaleeq2;
GtkWidget *vscaleeq3;
GtkWidget *vscaleeq4;
GtkWidget *vscaleeq5;
GtkWidget *vscaleeq6;
GtkWidget *vscaleeq7;
GtkWidget *vscaleeq8;
GtkWidget *vscaleeq9;
GtkWidget *vscaleeqA;
GtkWidget *eqbox0;
GtkWidget *eqbox1;
GtkWidget *eqbox2;
GtkWidget *eqbox3;
GtkWidget *eqbox4;
GtkWidget *eqbox5;
GtkWidget *eqbox6;
GtkWidget *eqbox7;
GtkWidget *eqbox8;
GtkWidget *eqbox9;
GtkWidget *eqboxA;
GtkWidget *eqlabel0;
GtkWidget *eqlabel1;
GtkWidget *eqlabel2;
GtkWidget *eqlabel3;
GtkWidget *eqlabel4;
GtkWidget *eqlabel5;
GtkWidget *eqlabel6;
GtkWidget *eqlabel7;
GtkWidget *eqlabel8;
GtkWidget *eqlabel9;
GtkWidget *eqlabelA;
GtkWidget *eqenable;
GtkWidget *eqautolevel;
GtkWidget *combopreset;

GtkWidget *windowparm;
GtkWidget *parmvbox;
GtkWidget *parmhbox1;
GtkWidget *parmlabel11;
GtkWidget *parmlabel12;
GtkWidget *spinbutton1;
GtkWidget *parmhbox2;
GtkWidget *parmlabel21;
GtkWidget *parmlabel22;
GtkWidget *spinbutton2;
GtkWidget *parmhbox3;
GtkWidget *parmlabel3;
GtkWidget *spinbutton3;
GtkWidget *parmhbox4;
GtkWidget *parmlabel4;
GtkWidget *spinbutton4;
GtkWidget *parmhlevel1;
GtkWidget *levellabel1;
GtkWidget *parmhlevel2;
GtkWidget *levellabel2;
GtkWidget *parmhlevel3;
GtkWidget *levellabel3;
GtkWidget *parmhlevel4;
GtkWidget *levellabel4;
GtkWidget *parmhlevel5;
GtkWidget *levellabel5;
GtkWidget *parmhlevel6;
GtkWidget *levellabel6;
GtkWidget *parmhlevel7;
GtkWidget *levellabel7;

GtkWidget *notebook;
GtkWidget *nbpage1;
GtkWidget *nbpage2;
GtkWidget *nbpage3;

int vpvisible;
int hscaleupd;
int playerWidth, playerHeight;

videoplayer vp;
eqdefaults aedef;
audioequalizer ae;
audiomixer *ax;
int last_id;
pthread_t tid;
int retval0;
cpu_set_t cpu[4];
}vpwidgets;

typedef struct
{
vpwidgets *vpw;
int vqMaxLength, aqMaxLength;
}playlistparams;

void init_playlistparams(playlistparams *plparams, vpwidgets *vpw, int vqMaxLength, int aqMaxLength);
void close_playlistparams(playlistparams *plparams);
void toggle_vp(vpwidgets *vpw, GtkWidget *togglebutton);
void init_videoplayerwidgets(playlistparams *plp, int argc, char** argv, int playWidth, int playHeight, audiomixer *x);
void close_videoplayerwidgets(vpwidgets *vpw);
void press_vp_stop_button(playlistparams *plp);
#endif

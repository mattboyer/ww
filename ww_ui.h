/*
Copyright (C) 2011 Matt Boyer.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the project nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#ifndef WW_UI_H
#define WW_UI_H

#include <gnt.h>
#include <gntws.h>
#include <gntwm.h>
#include <gntwindow.h>
#include <gntbutton.h>
#include <gntlabel.h>

#include "portable_info.h"
#include "ww_config.h"

#define SEC_DAY 86400
#define SEC_HOUR 3600
#define SEC_MINUTE 60

struct main_window_widgets {
	GntWindow* window;
	GntTree* utmp_tree;
	GntTextView* host_text;
	GntTextView* user_text;
	GntButton* quit_button;
	GntBox* user_box;
	GntBox* host_box;
};

void display_user_info(GntWidget*, gpointer, gpointer, struct status*);
void setup_ui(struct status*);
void populate_tree(struct status*, char**);
void dismantle_ui(GntWidget*, struct status*);
void display_host_info(struct status*);
void display_duration(time_t, char*);
void update_max_width(char*, size_t*);

#endif

/* vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list: */

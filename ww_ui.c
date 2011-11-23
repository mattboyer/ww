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

#include "ww_ui.h"
const char* column_titles[]={COLUMN_TITLES};

void display_user_info(GntWidget* originator, gpointer old, gpointer current,
    struct status* status) {

	gpointer selected_row_key = gnt_tree_get_selection_data(
	    GNT_TREE(originator));
	struct abstract_utmpx* session_entry = g_hash_table_lookup(
	    status->sessions, selected_row_key);
	struct abstract_user* user_entry = g_hash_table_lookup(
	    status->users, session_entry->user_name);

	if(!user_entry || !session_entry)
		return;

	// Blank user textview
	gnt_text_view_clear(status->widgets->user_text);

	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    "Full name: ", GNT_TEXT_FLAG_BOLD);
	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    user_entry->full_name, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(status->widgets->user_text);

	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    "UID: ", GNT_TEXT_FLAG_BOLD);
	char* display_uid= calloc(32,sizeof(char));
	sprintf(display_uid, "%d", user_entry->uid);
	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    display_uid, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(status->widgets->user_text);


	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    "GID: ", GNT_TEXT_FLAG_BOLD);
	char* display_gid= calloc(32,sizeof(char));
	sprintf(display_gid, "%d", user_entry->main_gid);
	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    display_gid, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(status->widgets->user_text);


	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    "Groups: ", GNT_TEXT_FLAG_BOLD);

	char* display_groups=calloc(512, sizeof(char));

	GHashTableIter group_iterator;
	gpointer gid;
	char* name;
	g_hash_table_iter_init(&group_iterator, user_entry->group_membership);

	while(g_hash_table_iter_next(&group_iterator, &gid, (gpointer) &name)) {
		if( 0 != strcmp(display_groups, "") )
			strcat(display_groups, ", ");

		char* display_group=calloc(32,sizeof(char));
		sprintf(display_group, "%s(%d)", name, GPOINTER_TO_INT(gid));
		display_groups=strcat(display_groups, display_group);
	}
	gnt_text_view_append_text_with_flags(status->widgets->user_text,
	    display_groups, GNT_TEXT_FLAG_NORMAL);

}

void setup_ui(struct status* st) {

	/* Create main window */
	GntWidget* utmp_window = gnt_window_box_new(FALSE,TRUE);
	char* window_title=calloc(40, sizeof(char));
	sprintf(window_title, "Active UTMP sessions - ww v%d.%02d",
	    WW_VERSION_MAJOR, WW_VERSION_MINOR);
	gnt_box_set_title(GNT_BOX(utmp_window), window_title);
	gnt_box_set_alignment(GNT_BOX(utmp_window), GNT_ALIGN_MID);
	GNT_WIDGET_SET_FLAGS(utmp_window, GNT_WIDGET_NO_SHADOW);
	gnt_box_set_fill(GNT_BOX(utmp_window), TRUE);
	gnt_window_set_maximize(GNT_WINDOW(utmp_window),
	    GNT_WINDOW_MAXIMIZE_X | GNT_WINDOW_MAXIMIZE_Y);


	/* Container for User and Host info boxes */
	GntWidget* info_box = gnt_box_new(FALSE,FALSE);
	gnt_box_set_fill(GNT_BOX(info_box), TRUE);
	gnt_box_set_alignment(GNT_BOX(info_box), GNT_ALIGN_TOP);
	gnt_box_add_widget(GNT_BOX(utmp_window), info_box);


	/* Info on the local system */
	GntWidget* host_info_box = gnt_box_new(TRUE, TRUE);
	gnt_box_set_title(GNT_BOX(host_info_box), "Host Information");
	GntWidget* host_info_text=gnt_text_view_new();
	gnt_text_view_set_flag(GNT_TEXT_VIEW(host_info_text),
	    GNT_TEXT_VIEW_NO_SCROLL | GNT_TEXT_VIEW_TOP_ALIGN);
	gnt_box_add_widget(GNT_BOX(host_info_box), host_info_text);
	GNT_WIDGET_UNSET_FLAGS(GNT_WIDGET(host_info_box), GNT_WIDGET_NO_BORDER);


	/* User info box */
	GntWidget* user_info_box = gnt_box_new(FALSE, TRUE);
	gnt_box_set_title(GNT_BOX(user_info_box), "User Information");
	GntWidget* user_info_text=gnt_text_view_new();
	gnt_text_view_set_flag(GNT_TEXT_VIEW(user_info_text),
	    GNT_TEXT_VIEW_NO_SCROLL | GNT_TEXT_VIEW_TOP_ALIGN);
	gnt_box_add_widget(GNT_BOX(user_info_box), user_info_text);
	GNT_WIDGET_UNSET_FLAGS(GNT_WIDGET(user_info_box), GNT_WIDGET_NO_BORDER);


	/* Connection info box */
	GntWidget* connection_info_box = gnt_box_new(FALSE, TRUE);
	gnt_box_set_title(GNT_BOX(connection_info_box), "Connection Information");
	GntWidget* connection_info_text=gnt_text_view_new();
	gnt_text_view_set_flag(GNT_TEXT_VIEW(connection_info_text),
	    GNT_TEXT_VIEW_NO_SCROLL | GNT_TEXT_VIEW_TOP_ALIGN);
	gnt_box_add_widget(GNT_BOX(connection_info_box), connection_info_text);
	GNT_WIDGET_UNSET_FLAGS(GNT_WIDGET(connection_info_box),
	    GNT_WIDGET_NO_BORDER);
	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(connection_info_text),
	    "Hostname: ", GNT_TEXT_FLAG_BOLD);

	gnt_box_add_widget(GNT_BOX(info_box), host_info_box);
	gnt_box_add_widget(GNT_BOX(info_box), user_info_box);
	//gnt_box_add_widget(GNT_BOX(info_box), connection_info_box);


	/* Tree widget */
	GntWidget* tree = gnt_tree_new_with_columns(NUM_COLUMNS);
	gnt_tree_set_column_titles(GNT_TREE(tree), COLUMN_TITLES);
	gnt_tree_set_show_title(GNT_TREE(tree), TRUE);
	GNT_WIDGET_SET_FLAGS(tree, GNT_WIDGET_NO_BORDER);
	gnt_box_add_widget(GNT_BOX(utmp_window), tree);

/*
	GntWidget* but = gnt_button_new("Exit");
	gnt_box_add_widget(GNT_BOX(utmp_window), but);
*/

	gnt_widget_show(utmp_window);

	st->widgets->window=GNT_WINDOW(utmp_window);
	st->widgets->utmp_tree=GNT_TREE(tree);
	st->widgets->host_text=GNT_TEXT_VIEW(host_info_text);
	st->widgets->user_text=GNT_TEXT_VIEW(user_info_text);

	st->widgets->user_box=GNT_BOX(user_info_box);
	st->widgets->host_box=GNT_BOX(host_info_box);
	//widgets->quit_button=GNT_BUTTON(but);

	g_signal_connect(G_OBJECT(tree), "selection_changed",
	    G_CALLBACK(display_user_info), st);

}

void dismantle_ui(GntWidget* originator, struct status* st) {
	gnt_widget_destroy(GNT_WIDGET(st->widgets->host_text));
	gnt_widget_destroy(GNT_WIDGET(st->widgets->utmp_tree));
	gnt_widget_destroy(GNT_WIDGET(st->widgets->window));
	//gnt_widget_destroy(GNT_WIDGET(st->widgets->quit_button));

	g_hash_table_destroy(st->sessions);
	g_hash_table_destroy(st->users);
	gnt_quit();
}

void update_max_width(char* text, size_t* width) {
	if( strlen(text) > *width )
		*width=strlen(text);
}

void display_host_info(struct status* st) {

	/* Hostname */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "Hostname: ",
	    GNT_TEXT_FLAG_BOLD);
	gnt_text_view_append_text_with_flags(st->widgets->host_text,
	    st->host->host_name, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(st->widgets->host_text);

	/* OS */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "OS: ",
	    GNT_TEXT_FLAG_BOLD);
	gnt_text_view_append_text_with_flags(st->widgets->host_text,
	    st->host->os_type, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(st->widgets->host_text);

	/* Boot time */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "Up since: ",
	    GNT_TEXT_FLAG_BOLD);
	char* runlevel_timestamp;
	struct tm* runlevel_tm=localtime(&st->host->boot_time);
	/* TODO: Let the UI decide between long and short form based on
	 * the width of the terminal, as reported to ncurses.
	 */
	sprint_date(runlevel_tm, &runlevel_timestamp, DATE_FULL);
	gnt_text_view_append_text_with_flags(st->widgets->host_text,
	    runlevel_timestamp, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(st->widgets->host_text);

	/* Uptime */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "Up for: ",
	    GNT_TEXT_FLAG_BOLD);
	char* display_uptime;
	time_t now;
	time(&now);
	sprint_interval((time_t) (now - st->host->boot_time), &display_uptime,
	    INTERVAL_FULL);
	gnt_text_view_append_text_with_flags(st->widgets->host_text, display_uptime,
	    GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(st->widgets->host_text);

	/* Platform */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "Platform: ",
	    GNT_TEXT_FLAG_BOLD);
	gnt_text_view_append_text_with_flags(st->widgets->host_text,
	    st->host->hardware, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(st->widgets->host_text);

	/* Number of distinct users logged in */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "Users: ",
	    GNT_TEXT_FLAG_BOLD);
	char* display_users= calloc(32,sizeof(char));
	sprintf(display_users, "%d", g_hash_table_size(st->users));
	gnt_text_view_append_text_with_flags(st->widgets->host_text, display_users,
	    GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_next_line(st->widgets->host_text);

	/* Number of open sessions */
	gnt_text_view_append_text_with_flags(st->widgets->host_text, "Sessions: ",
	    GNT_TEXT_FLAG_BOLD);
	char* display_sessions= calloc(32,sizeof(char));
	sprintf(display_sessions, "%d", g_hash_table_size(st->sessions));
	gnt_text_view_append_text_with_flags(st->widgets->host_text,
	    display_sessions, GNT_TEXT_FLAG_NORMAL);
}

void populate_tree(struct status* st, char** keys) {

	/* Prepare an array to determine the max width of each column */
	size_t* max_widths = calloc(NUM_COLUMNS, sizeof(size_t));
	unsigned int title_idx;
	for(title_idx=0; title_idx<NUM_COLUMNS; title_idx++) {
		update_max_width((char*) column_titles[title_idx],
		    max_widths+title_idx);
	}

	GHashTableIter session_iterator;
	gpointer index, value;
	g_hash_table_iter_init(&session_iterator, st->sessions);

	while(g_hash_table_iter_next(&session_iterator, &index, &value)) {

		/* Build a GList of columns to display */
		struct abstract_utmpx* entry=(struct abstract_utmpx*) value;
		GList* columns=NULL;
		unsigned int col_index=0;

		/* User name */
		columns=g_list_append(columns, entry->user_name);
		update_max_width(entry->user_name, max_widths+col_index++);

		/* TTY name */
		columns=g_list_append(columns, entry->tty_name);
		update_max_width(entry->tty_name, max_widths+col_index++);

		/* Login PID */
		char* pid_string=calloc(MAX_PID_LENGTH, sizeof(char));
		sprintf(pid_string, "%d", entry->pid);
		columns=g_list_append(columns, pid_string);
		update_max_width(pid_string, max_widths+col_index++);

		/* Login time */
		char* display_login_time;
		struct tm* login_tm=localtime(&entry->login_time);
		sprint_date(login_tm, &display_login_time, DATE_COMPACT);
		columns=g_list_append(columns, display_login_time);
		update_max_width(display_login_time, max_widths+col_index++);

		/* Session duration */
		char* session_duration;
		time_t now;
		time(&now);

		/* TODO: Let the UI decide between long and short form based on
		 * the width of the terminal, as reported to ncurses.
		 */
		sprint_interval((time_t) (now - entry->login_time),
		    &session_duration, INTERVAL_COMPACT);
		columns=g_list_append(columns, session_duration);
		update_max_width(session_duration, max_widths+col_index++);

		/* Command and args of the Most Interesting Process */
#ifdef __sun
		char* process_args=calloc(PRARGSZ, sizeof(char));
#else
		char* process_args=calloc(400, sizeof(char));
#endif
		get_process_info(entry->mip, &process_args);
		if(0==strcmp(process_args, ""))
			sprintf(process_args, "could not get args for %d",
			    entry->mip);
		columns=g_list_append(columns, process_args);
		update_max_width(process_args, max_widths+col_index++);

		/* Add the row to the tree */
		GntTreeRow* r = gnt_tree_create_row_from_list(
		    st->widgets->utmp_tree, columns);
		gnt_tree_add_row_last(st->widgets->utmp_tree, index, r, NULL);
		keys=index;
		keys++;
	}

	/* Resize the columns according to their widest entry */
	unsigned int i;
	for(i=0; i<NUM_COLUMNS; i++) {
		gnt_tree_set_col_width(GNT_TREE(st->widgets->utmp_tree), i,
		    max_widths[i]+1);
	}
	gnt_widget_show(GNT_WIDGET(st->widgets->utmp_tree));
	free(max_widths);


	/* Prime the user info display */
	display_user_info(GNT_WIDGET(st->widgets->utmp_tree), NULL, NULL, st);
}

void sprint_interval(time_t elapsed, char** buf, interval_verbosity flag) {
	/* We only care about days, hours and minutes */
	unsigned int days, hours, minutes;

	days=elapsed/SEC_DAY;
	hours=(elapsed - days*SEC_DAY)/SEC_HOUR;
	minutes=(elapsed - days*SEC_DAY - hours*SEC_HOUR)/SEC_MINUTE;

	char* buffer;
	switch(flag) {
	case INTERVAL_COMPACT:
		buffer=calloc(12, sizeof(char));
		sprintf(buffer, "%dd:%dh:%dm", days, hours, minutes);
		break;
	case INTERVAL_FULL:
		buffer=calloc(32, sizeof(char));
		sprintf(buffer, "%d days %d hours %d minutes", days, hours,
		    minutes);
		break;
	}
	*buf=buffer;
}

void sprint_date(struct tm* date, char** buf, date_verbosity flag) {

	char* buffer;
	switch(flag) {
	case DATE_FULL:
		buffer=calloc(32, sizeof(char));
		strftime(buffer, 32, "%F %T", (const struct tm*) date);
		break;
	case DATE_COMPACT:
		buffer=calloc(12, sizeof(char));
		// is date less than a day old?
		time_t now;
		time(&now);
		time_t date_time=mktime(date);
		if( now - date_time <= SEC_DAY ) {
			strftime(buffer, 12, "%T", (const struct tm*) date);
		} else {
			strftime(buffer, 12, "%F", (const struct tm*) date);
		}
		break;
	}
	*buf=buffer;
}

/* vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list: */

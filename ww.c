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
#include "portable_info.h"

int main(int argc, char** argv) {

	gnt_init();

	/* Prep data structures */
	struct host_status host;
	struct main_window_widgets widgets;

	GHashTable* sessions=g_hash_table_new(NULL, NULL);
	GHashTable* users=g_hash_table_new(NULL, NULL);

	struct status st;
	st.users=users;
	st.sessions=sessions;
	st.widgets=&widgets;
	st.host=&host;


	/* Collect data */
	enumerate_sessions(sessions, &host, users);
	get_mips(sessions);
	get_host_info(&host);

	/* Build UI & display data */
	setup_ui(&st);
	display_host_info(&st);

	char** keys = calloc(g_hash_table_size(sessions), sizeof(char*) );
	populate_tree(&st, keys);

	/*
	 * TODO: Do something re: layout and the use of horizontal space in
	 * particular
	 */
	gnt_box_sync_children(GNT_BOX(widgets.window));
	gnt_main();


	gnt_quit();
	free(keys);
	return EXIT_SUCCESS;
}

/* vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list: */

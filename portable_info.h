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

#ifndef PORTABLE_INFO_H
#define PORTABLE_INFO_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <strings.h>
#include <utmpx.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __NetBSD__
#include <procfs.h>
#include <sys/proc.h>
#else
#include <kvm.h>
#include <sys/sysctl.h>
#endif
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/utsname.h>
#ifdef __sun
#include <netdb.h>
#include <dirent.h>
#else
#include <sys/param.h>
#endif

#include <glib-object.h>

#define MAX_PID_LENGTH 32

struct abstract_utmpx {
	char* user_name;
	pid_t pid;
	pid_t mip;
	char* tty_name;
	time_t login_time;
	char* login_host;
};

struct host_status {
	time_t boot_time;
	char* os_type;
	char* host_name;
	char* hardware;
};

struct abstract_user {
	uid_t uid;
	char* full_name;
	gid_t main_gid;
	GHashTable* group_membership;
};

struct status {
	GHashTable* users;
	GHashTable* sessions;
	struct main_window_widgets* widgets;
	struct host_status* host;
};

#ifdef sun
void sunos_get_process_info(pid_t, char**);
void sunos_get_mips(GHashTable*);
#define get_process_info(X, Y) sunos_get_process_info(X,Y)
#define get_mips(X) sunos_get_mips(X)
#endif

#ifdef __NetBSD__
void bsd_get_process_info(pid_t, char**);
int bsd_proc_compare(struct kinfo_proc2*, struct kinfo_proc2*);
void bsd_get_mips(GHashTable* sessions);
#define get_process_info(X, Y) bsd_get_process_info(X,Y)
#define get_mips(X) bsd_get_mips(X)
#endif

void get_host_info(struct host_status*);
void get_user_info(GHashTable*, char*);
int enumerate_sessions(GHashTable*, struct host_status*, GHashTable*);

#endif

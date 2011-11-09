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

#include "portable_info.h"


#ifdef __NetBSD__

#define ISRUN(p)        (((p)->p_stat == LSRUN) || ((p)->p_stat == SIDL) || \
                         ((p)->p_stat == LSONPROC))
#define TESTAB(a, b)    ((a)<<1 | (b))
#define ONLYA   2
#define ONLYB   1
#define BOTH    3


int bsd_proc_compare(struct kinfo_proc2* p1, struct kinfo_proc2* p2) {

        if (p1 == NULL)
                return (1);
        /*
         * see if at least one of them is runnable
         */
        switch (TESTAB(ISRUN(p1), ISRUN(p2))) {
        case ONLYA:
                return (0);
        case ONLYB:
                return (1);
        case BOTH:
                /*
                 * tie - favor one with highest recent CPU utilization
                 */
                if (p2->p_pctcpu > p1->p_pctcpu)
                        return (1);
                if (p1->p_pctcpu > p2->p_pctcpu)
                        return (0);
                return (p2->p_pid > p1->p_pid); /* tie - return highest pid */
        }
        /*
         * weed out zombies
         */
        switch (TESTAB(p1->p_stat == SZOMB, p2->p_stat == SZOMB)) {
        case ONLYA:
                return (1);
        case ONLYB:
                return (0);
        case BOTH:
                return (p2->p_pid > p1->p_pid); /* tie - return highest pid */
        }
        /*
         * pick the one with the smallest sleep time
         */
        if (p2->p_slptime > p1->p_slptime)
                return (0);
        if (p1->p_slptime > p2->p_slptime)
                return (1);
        /*
         * favor one sleeping in a non-interruptible sleep
         */
        if (p1->p_flag & L_SINTR && (p2->p_flag & L_SINTR) == 0)
                return (1);
        if (p2->p_flag & L_SINTR && (p1->p_flag & L_SINTR) == 0)
                return (0);
        return (p2->p_pid > p1->p_pid);         /* tie - return highest pid */

}

void bsd_get_mips(GHashTable* sessions) {

	/* Open kvm handle */
	char* error_buf=calloc(_POSIX2_LINE_MAX, sizeof(char));
	kvm_t* kerneld;
	kerneld=kvm_open((char*) NULL, (char*) NULL, (char*) NULL, KVM_NO_FILES, error_buf);
	if (NULL == kerneld) {
		return;
	}

	GHashTableIter session_iterator;
	gpointer index, value;
	g_hash_table_iter_init(&session_iterator, sessions);

	while(g_hash_table_iter_next(&session_iterator, &index, &value)) {
		struct abstract_utmpx* entry=(struct abstract_utmpx*) value;

		int processes=0;
		struct kinfo_proc2* session_process=kvm_getproc2(kerneld, KERN_PROC_PID, entry->pid, sizeof(struct kinfo_proc2), &processes);
		if (!session_process || 1!=processes)
			goto fini;
		/* make a backup of the struct */
		struct kinfo_proc2* session_info=calloc(1, sizeof(struct kinfo_proc2));
		memcpy(session_info, session_process, sizeof(struct kinfo_proc2));

		struct kinfo_proc2* mip_candidates=kvm_getproc2(kerneld, KERN_PROC_SESSION, entry->pid, sizeof(struct kinfo_proc2), &processes);
		if (!mip_candidates)
			goto fini;

		if (1==processes) {
			entry->mip=mip_candidates->p_pid;
		} else {
			/* Find a MIP with proc_compare*/
			unsigned int candidate_index;
			struct kinfo_proc2* mip=session_info;
			for(candidate_index=0; candidate_index<processes; candidate_index++, mip_candidates++) {
				if (bsd_proc_compare(mip, mip_candidates)) {
					mip=mip_candidates;
				}
			}
			entry->mip=mip->p_pid;
		}
	}

fini:
	kvm_close(kerneld);
	return;
}

void bsd_get_process_info(pid_t pid, char** process_args) {

	char* error_buf=calloc(_POSIX2_LINE_MAX, sizeof(char));
	kvm_t* kerneld;
	kerneld=kvm_open((char*) NULL, (char*) NULL, (char*) NULL, KVM_NO_FILES, error_buf);
	if (!kerneld)
		goto fini;

	int processes=0;
	struct kinfo_proc2* proc_info=kvm_getproc2(kerneld, KERN_PROC_PID, pid, sizeof(struct kinfo_proc2), &processes);
	if (1 != processes)
		goto fini;

	char ** process_argv=kvm_getargv2(kerneld, proc_info, 500);
	if (!process_argv )
		goto fini;

	if (!*process_argv) {
		sprintf(*process_args, "(%s)", proc_info->p_comm);
		goto fini;
	}

	while(*process_argv) {
		strcat(*process_args, *process_argv);
		strcat(*process_args, " ");
		process_argv++;
	}
fini:
	kvm_close(kerneld);
	free(error_buf);
	return;
}
#endif

#ifdef __sun

void sunos_get_mips(GHashTable* sessions) {

	GHashTableIter session_iterator;
	gpointer index, value;
	g_hash_table_iter_init(&session_iterator, sessions);

	while(g_hash_table_iter_next(&session_iterator, &index, &value)) {
		struct abstract_utmpx* entry=(struct abstract_utmpx*) value;
		entry->mip=entry->pid;
	}
}


void sunos_get_process_info(pid_t pid, char** process_args) {
	char* proc_file=calloc(500, sizeof(char));
	sprintf(proc_file, "/proc/%d/psinfo", pid);
	int psinfo_fd=open(proc_file, O_RDONLY);
	if ( 0 > psinfo_fd ) {
		sprintf(*process_args, "could not open %s\n", proc_file);
		return;
	}

	/* mmap() doesn't work on procfs on Solaris */
	struct psinfo* process_info=calloc(1, sizeof(struct psinfo));
	read(psinfo_fd, process_info, sizeof(struct psinfo));
	strncpy(*process_args, process_info->pr_psargs, PRARGSZ);

	free(process_info);
	free(proc_file);
	close(psinfo_fd);
}
#endif

void get_user_info(GHashTable* users, char* username) {
	struct passwd passwd_entry;
	size_t passwd_buflen=sysconf(_SC_GETPW_R_SIZE_MAX);
	void* passwd_buf=malloc(passwd_buflen);
#ifdef __sun
	getpwnam_r(username, &passwd_entry, passwd_buf, passwd_buflen);
#else
	struct passwd* result_struct;
	getpwnam_r(username, &passwd_entry, passwd_buf, passwd_buflen, &result_struct);
#endif

	struct abstract_user* user_entry=calloc(1,sizeof(struct abstract_user));
	user_entry->uid=passwd_entry.pw_uid;
	user_entry->full_name=passwd_entry.pw_gecos;

	user_entry->main_gid=passwd_entry.pw_gid;

	/* We store the main GID and the memberships in a separate hash table */
	GHashTable* membership=g_hash_table_new(g_int_hash, g_int_equal);

	/* temporarily use an array of gid_t */
	size_t max_groups=sysconf(_SC_NGROUPS_MAX);
	gid_t* gid_membership=calloc(max_groups, sizeof(gid_t));
	getgroups(max_groups, gid_membership);

	gid_t* index;
	size_t group_buflen=sysconf(_SC_GETGR_R_SIZE_MAX);
	for(index=&user_entry->main_gid; *index; index++) {

		struct group* group_info=calloc(1,sizeof(struct group));
		void* group_buf=malloc(group_buflen);

#ifdef __sun
		getgrgid_r(*index, group_info, group_buf, group_buflen);
#else
		struct group* result;
		getgrgid_r(*index, group_info, group_buf, group_buflen, &result);
#endif
		g_hash_table_insert(membership, index, group_info->gr_name);
		if (index == &user_entry->main_gid)
			index=gid_membership;
	}
	//free(gid_membership);
	user_entry->group_membership = membership;

	g_hash_table_insert(users, username, user_entry);
}

void get_host_info(struct host_status* status) {
	struct utsname uname_data;
	uname(&uname_data);
	status->os_type=strdup(uname_data.sysname);
	status->hardware=strdup(uname_data.machine);

	char* hostname = calloc(MAXHOSTNAMELEN, sizeof(char));
	gethostname(hostname, MAXHOSTNAMELEN);
	status->host_name=hostname;
}

int enumerate_sessions(GHashTable* abstract_sessions, struct host_status* status, GHashTable* users) {

	struct utmpx* entry;
	setutxent();

	while(entry = getutxent()) {
		if (entry->ut_type == BOOT_TIME) {
			status->boot_time = entry->ut_tv.tv_sec;
			continue;
		}
		if (entry->ut_type != USER_PROCESS)
			continue;

		struct abstract_utmpx* abstract=calloc(1,sizeof(struct abstract_utmpx));

		/* Get user name */
#if defined(__sun) || defined(__linux__)
		abstract->user_name=strdup(entry->ut_user);
#else
		abstract->user_name=strdup(entry->ut_name);
#endif

		/* Collect info on that user */
		if( !g_hash_table_lookup(users, abstract->user_name) )
			get_user_info(users, abstract->user_name);

		/* Get login PID*/
		abstract->pid=entry->ut_pid;

		/* Get TTY name*/
		char* line=strndup(entry->ut_line, MAX_PID_LENGTH);
		abstract->tty_name=line;

		/* Get login time */
		abstract->login_time=entry->ut_tv.tv_sec;


/*
#ifndef sun
		struct sockaddr_storage* remote_addr = (struct sockaddr_storage*) calloc( 1, sizeof( struct sockaddr_storage) );
		memcpy( (void*) remote_addr, (const void*) &entry->ut_ss, sizeof( struct sockaddr_storage) );
#endif
*/

#ifdef __sun
		char* host=strndup(entry->ut_host, entry->ut_syslen);
#endif
#ifdef __NetBSD__
		char* host=strndup(entry->ut_host, _UTX_HOSTSIZE);
#endif
#ifdef __linux__
               char* host=strndup(entry->ut_host, __UT_HOSTSIZE);
#endif
		abstract->login_host=host;

		char* session_key=calloc(MAX_PID_LENGTH, sizeof(char));
		sprintf(session_key, "%s@%d", abstract->user_name, abstract->pid);
		g_hash_table_insert(abstract_sessions, session_key, abstract);
	}
	return g_hash_table_size(abstract_sessions);
}

/*-------------------------------------------------------------------------*
 * GNU Prolog                                                              *
 *                                                                         *
 * Part  : Prolog buit-in predicates                                       *
 * File  : os_interf_c.c                                                   *
 * Descr.: operating system interface management - C part                  *
 * Author: Daniel Diaz                                                     *
 *                                                                         *
 * Copyright (C) 1999,2000 Daniel Diaz                                     *
 *                                                                         *
 * GNU Prolog is free software; you can redistribute it and/or modify it   *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2, or any later version.       *
 *                                                                         *
 * GNU Prolog is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc.  *
 * 59 Temple Place - Suite 330, Boston, MA 02111, USA.                     *
 *-------------------------------------------------------------------------*/

/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gp_config.h"

#ifdef M_ix86_win32
#include <process.h>
#include <direct.h>
#include <io.h>
#include <winsock.h>
#else
#define _XOPEN_SOURCE_EXTENDED	/* for alpha/OSF (usleep prototype) */
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif

#define OBJ_INIT Os_Interf_Initializer

#include "engine_pl.h"
#include "bips_pl.h"


#if 0
#define DEBUG
#endif




/*---------------------------------*
 * Constants                       *
 *---------------------------------*/

#define MAX_SIGNALS                255
#define MAX_SPAWN_ARGS             1024




/*---------------------------------*
 * Type Definitions                *
 *---------------------------------*/

typedef struct
{
  int atom;
  int sig;
}
InfSig;




/*---------------------------------*
 * Global Variables                *
 *---------------------------------*/

static int atom_dt;

static int atom_read;
static int atom_write;
static int atom_execute;
static int atom_search;

static int atom_regular;
static int atom_directory;
static int atom_fifo;
static int atom_socket;
static int atom_character_device;
static int atom_block_device;
static int atom_unknown;

static InfSig tsig[MAX_SIGNALS];
static int nb_sig;




/*---------------------------------*
 * Function Prototypes             *
 *---------------------------------*/

static int Flag_Of_Permission(WamWord perm_word, Bool is_a_directory);

static char *Get_Path_Name(WamWord path_name_word);

static Bool Date_Time_To_Prolog(time_t *t, WamWord date_time_word);

static int Select_Init_Set(WamWord list_word, fd_set *set, int check);

static Bool Select_Init_Ready_List(WamWord list_word, fd_set *set,
				   WamWord ready_list_word);




/*-------------------------------------------------------------------------*
 * OS_INTERF_INITIALIZER                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Os_Interf_Initializer(void)
{
  atom_dt = Create_Atom("dt");

  atom_read = Create_Atom("read");
  atom_write = Create_Atom("write");
  atom_execute = Create_Atom("execute");
  atom_search = Create_Atom("search");

  atom_regular = Create_Atom("regular");
  atom_directory = Create_Atom("directory");
  atom_fifo = Create_Atom("fifo");
  atom_socket = Create_Atom("socket");
  atom_character_device = Create_Atom("character_device");
  atom_block_device = Create_Atom("block_device");
  atom_unknown = Create_Atom("unknown");

  nb_sig = 0;
#if defined(__unix__) || defined(__CYGWIN__)
  tsig[nb_sig].atom = Create_Atom("SIGHUP");
  tsig[nb_sig++].sig = SIGHUP;
  tsig[nb_sig].atom = Create_Atom("SIGINT");
  tsig[nb_sig++].sig = SIGINT;
  tsig[nb_sig].atom = Create_Atom("SIGQUIT");
  tsig[nb_sig++].sig = SIGQUIT;
  tsig[nb_sig].atom = Create_Atom("SIGILL");
  tsig[nb_sig++].sig = SIGILL;
  tsig[nb_sig].atom = Create_Atom("SIGTRAP");
  tsig[nb_sig++].sig = SIGTRAP;
  tsig[nb_sig].atom = Create_Atom("SIGABRT");
  tsig[nb_sig++].sig = SIGABRT;
#ifndef M_ix86_cygwin
  tsig[nb_sig].atom = Create_Atom("SIGIOT");
  tsig[nb_sig++].sig = SIGIOT;
#endif
  tsig[nb_sig].atom = Create_Atom("SIGBUS");
  tsig[nb_sig++].sig = SIGBUS;
  tsig[nb_sig].atom = Create_Atom("SIGFPE");
  tsig[nb_sig++].sig = SIGFPE;
  tsig[nb_sig].atom = Create_Atom("SIGKILL");
  tsig[nb_sig++].sig = SIGKILL;
  tsig[nb_sig].atom = Create_Atom("SIGUSR1");
  tsig[nb_sig++].sig = SIGUSR1;
  tsig[nb_sig].atom = Create_Atom("SIGSEGV");
  tsig[nb_sig++].sig = SIGSEGV;
  tsig[nb_sig].atom = Create_Atom("SIGUSR2");
  tsig[nb_sig++].sig = SIGUSR2;
  tsig[nb_sig].atom = Create_Atom("SIGPIPE");
  tsig[nb_sig++].sig = SIGPIPE;
  tsig[nb_sig].atom = Create_Atom("SIGALRM");
  tsig[nb_sig++].sig = SIGALRM;
  tsig[nb_sig].atom = Create_Atom("SIGTERM");
  tsig[nb_sig++].sig = SIGTERM;
  tsig[nb_sig].atom = Create_Atom("SIGCHLD");
  tsig[nb_sig++].sig = SIGCHLD;
  tsig[nb_sig].atom = Create_Atom("SIGCONT");
  tsig[nb_sig++].sig = SIGCONT;
  tsig[nb_sig].atom = Create_Atom("SIGSTOP");
  tsig[nb_sig++].sig = SIGSTOP;
  tsig[nb_sig].atom = Create_Atom("SIGTSTP");
  tsig[nb_sig++].sig = SIGTSTP;
  tsig[nb_sig].atom = Create_Atom("SIGTTIN");
  tsig[nb_sig++].sig = SIGTTIN;
  tsig[nb_sig].atom = Create_Atom("SIGTTOU");
  tsig[nb_sig++].sig = SIGTTOU;
  tsig[nb_sig].atom = Create_Atom("SIGURG");
  tsig[nb_sig++].sig = SIGURG;
  tsig[nb_sig].atom = Create_Atom("SIGXCPU");
  tsig[nb_sig++].sig = SIGXCPU;
  tsig[nb_sig].atom = Create_Atom("SIGXFSZ");
  tsig[nb_sig++].sig = SIGXFSZ;
  tsig[nb_sig].atom = Create_Atom("SIGVTALRM");
  tsig[nb_sig++].sig = SIGVTALRM;
  tsig[nb_sig].atom = Create_Atom("SIGPROF");
  tsig[nb_sig++].sig = SIGPROF;
  tsig[nb_sig].atom = Create_Atom("SIGWINCH");
  tsig[nb_sig++].sig = SIGWINCH;
#ifndef M_ix86_sco
  tsig[nb_sig].atom = Create_Atom("SIGIO");
  tsig[nb_sig++].sig = SIGIO;
#endif
#ifndef M_ix86_bsd
  tsig[nb_sig].atom = Create_Atom("SIGPOLL");
  tsig[nb_sig++].sig = SIGPOLL;
#endif
#endif


#if defined(__unix__) || defined(__CYGWIN__)
  signal(SIGPIPE, SIG_IGN);
#endif
}




/*-------------------------------------------------------------------------*
 * MAKE_DIRECTORY_1                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Make_Directory_1(WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

#ifdef M_ix86_win32
  Os_Test_Error(_mkdir(path_name));
#else
  Os_Test_Error(mkdir(path_name, 0777));
#endif

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * DELETE_DIRECTORY_1                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Delete_Directory_1(WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  Os_Test_Error(rmdir(path_name));

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * WORKING_DIRECTORY_1                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Working_Directory_1(WamWord path_name_word)
{
  char *path_name;

  path_name = M_Get_Working_Dir();

  return Un_String_Check(path_name, path_name_word);
}




/*-------------------------------------------------------------------------*
 * CHANGE_DIRECTORY_1                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Change_Directory_1(WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  errno = -1;
  Os_Test_Error(!M_Set_Working_Dir(path_name));
  return TRUE;
}




/*-------------------------------------------------------------------------*
 * DIRECTORY_FILES_2                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Directory_Files_2(WamWord path_name_word, WamWord list_word)
{
  char *path_name;
  Bool res;
  char *name;

#ifdef M_ix86_win32
  long h;
  struct _finddata_t d;
  static char buff[MAXPATHLEN];
#else
  DIR *dir;
  struct dirent *cur_entry;
#endif


  Check_For_Un_List(list_word);

  path_name = Get_Path_Name(path_name_word);

#ifdef M_ix86_win32
  sprintf(buff, "%s\\*.*", path_name);
  h = _findfirst(buff, &d);	/* instead of Win32 FindFirstFile since uses errno */
  Os_Test_Error(h == -1);
#else
  dir = opendir(path_name);
  Os_Test_Error(dir == NULL);
#endif

#ifdef M_ix86_win32
  do
    {
      name = d.name;
#else
  while ((cur_entry = readdir(dir)) != NULL)
    {
      name = cur_entry->d_name;
#endif
      if (!Get_List(list_word) || !Unify_Atom(Create_Allocate_Atom(name)))
	{
	  res = FALSE;
	  goto finish;
	}

      list_word = Unify_Variable();
    }
#ifdef M_ix86_win32
  while (_findnext(h, &d) == 0);
#endif

  res = Get_Nil(list_word);

finish:
#ifdef M_ix86_win32
  _findclose(h);
#else
  closedir(dir);
#endif

  return res;
}




/*-------------------------------------------------------------------------*
 * RENAME_FILE_2                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Rename_File_2(WamWord path_name1_word, WamWord path_name2_word)
{
  char path_name1[MAXPATHLEN];
  char *path_name2;

  strcpy(path_name1, Get_Path_Name(path_name1_word));
  path_name2 = Get_Path_Name(path_name2_word);

  Os_Test_Error(rename(path_name1, path_name2));

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * UNLINK_1                                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Unlink_1(WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  unlink(path_name);
}




/*-------------------------------------------------------------------------*
 * DELETE_FILE_1                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Delete_File_1(WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  Os_Test_Error(unlink(path_name));

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * FILE_EXISTS_1                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Exists_1(WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  if (access(path_name, F_OK))
    {
      if (errno == ENOENT || errno == ENOTDIR)
	return FALSE;

      Os_Test_Error(1);
    }

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * FILE_PERMISSION_2                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Permission_2(WamWord path_name_word, WamWord perm_list_word)
{
  WamWord word, tag, *adr;
  WamWord save_perm_list_word;
  WamWord *lst_adr;
  char *path_name;
  int mode, perm = 0;
  struct stat file_info;
  int res;
  Bool is_a_directory;

  path_name = Get_Path_Name(path_name_word);

  res = stat(path_name, &file_info);
  if (res == -1 && errno != ENOENT && errno != ENOTDIR)
    Os_Test_Error(1);

  mode = file_info.st_mode;

  is_a_directory = (res == 0) && S_ISDIR(mode);

  Deref(perm_list_word, word, tag, adr);
  if (tag == ATM && word != NIL_WORD)
    perm |= Flag_Of_Permission(word, is_a_directory);
  else
    {
      save_perm_list_word = perm_list_word;

      for (;;)
	{
	  Deref(perm_list_word, word, tag, adr);

	  if (tag == REF)
	    Pl_Err_Instantiation();

	  if (word == NIL_WORD)
	    break;

	  if (tag != LST)
	    Pl_Err_Type(type_list, save_perm_list_word);

	  lst_adr = UnTag_LST(word);
	  perm |= Flag_Of_Permission(Car(lst_adr), is_a_directory);

	  perm_list_word = Cdr(lst_adr);
	}
    }

  return (res == 0) && perm > 0 && ((mode | perm) == mode);
}




/*-------------------------------------------------------------------------*
 * FLAG_OF_PERMISSION                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static int
Flag_Of_Permission(WamWord perm_word, Bool is_a_directory)
{
  int atom;

  atom = Rd_Atom_Check(perm_word);

  if (atom == atom_read)
    return S_IRUSR;

  if (atom == atom_write)
    return S_IWUSR;

  if (atom == atom_execute)
    return (is_a_directory) ? -1 : S_IXUSR;

  if (atom == atom_search)
    return (is_a_directory) ? S_IXUSR : -1;

  Pl_Err_Domain(domain_os_file_permission, perm_word);
  return 0;			/* anything for the compiler */
}




/*-------------------------------------------------------------------------*
 * FILE_PROP_ABSOLUTE_FILE_NAME_2                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Prop_Absolute_File_Name_2(WamWord absolute_path_name_word,
			       WamWord path_name_word)
{
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  Os_Test_Error(access(path_name, F_OK));	/* test if file exists */

  return Un_String_Check(path_name, absolute_path_name_word);
}




/*-------------------------------------------------------------------------*
 * FILE_PROP_REAL_FILE_NAME_2                                              *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Prop_Real_File_Name_2(WamWord real_path_name_word,
			   WamWord path_name_word)
{
  char *path_name = Get_Path_Name(path_name_word);

#ifndef M_ix86_win32
  char real_path_name[MAXPATHLEN];

  Os_Test_Error(realpath(path_name, real_path_name) == NULL);
#else
  char *real_path_name = path_name;
#endif

  return Un_String_Check(real_path_name, real_path_name_word);
}




/*-------------------------------------------------------------------------*
 * FILE_PROP_TYPE_2                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Prop_Type_2(WamWord type_word, WamWord path_name_word)
{
  char *path_name;
  struct stat file_info;
  int atom;

  path_name = Get_Path_Name(path_name_word);

  Os_Test_Error(stat(path_name, &file_info));

  if (S_ISREG(file_info.st_mode))
    atom = atom_regular;
  else if (S_ISDIR(file_info.st_mode))
    atom = atom_directory;
#ifdef S_ISFIFO
  else if (S_ISFIFO(file_info.st_mode))
    atom = atom_fifo;
#endif
#ifdef S_ISSOCK
  else if (S_ISSOCK(file_info.st_mode))
    atom = atom_socket;
#endif
#ifdef S_ISCHR
  else if (S_ISCHR(file_info.st_mode))
    atom = atom_character_device;
#endif
#ifdef S_ISBLK
  else if (S_ISBLK(file_info.st_mode))
    atom = atom_block_device;
#endif
  else
    atom = atom_unknown;

  return Un_Atom_Check(atom, type_word);
}




/*-------------------------------------------------------------------------*
 * FILE_PROP_SIZE_2                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Prop_Size_2(WamWord size_word, WamWord path_name_word)
{
  char *path_name;
  struct stat file_info;

  path_name = Get_Path_Name(path_name_word);

  Os_Test_Error(stat(path_name, &file_info));

  return Un_Positive_Check((int) file_info.st_size, size_word);
}




/*-------------------------------------------------------------------------*
 * CHECK_PROP_PERM_AND_FILE_2                                              *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Check_Prop_Perm_And_File_2(WamWord perm_word, WamWord path_name_word)
{
  WamWord word, tag, *adr;
  char *path_name;

  path_name = Get_Path_Name(path_name_word);

  Deref(perm_word, word, tag, adr);
  if (tag != REF)
    Flag_Of_Permission(perm_word, FALSE);	/* to check perm validity */

  Os_Test_Error(access(path_name, F_OK));	/* to check file existence */

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * FILE_PROP_DATE_2                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
File_Prop_Date_2(WamWord date_time_word, WamWord path_name_word)
{
  char *path_name;
  struct stat file_info;
  time_t *t;

  path_name = Get_Path_Name(path_name_word);

  Os_Test_Error(stat(path_name, &file_info));

  switch (sys_var[0])
    {
    case 0:
      t = &(file_info.st_ctime);
      break;

    case 1:
      t = &(file_info.st_atime);
      break;

    default:
      t = &(file_info.st_mtime);
      break;
    }

  return Date_Time_To_Prolog(t, date_time_word);
}




/*-------------------------------------------------------------------------*
 * TEMPORARY_NAME_2                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Temporary_Name_2(WamWord template_word, WamWord path_name_word)
{
  char *template;
  char *path_name;

  template = Get_Path_Name(template_word);

  errno = -1;
  path_name = (char *) mktemp(template);
  Os_Test_Error(path_name == NULL);

  return path_name && Un_String_Check(path_name, path_name_word);
}




/*-------------------------------------------------------------------------*
 * TEMPORARY_FILE_3                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Temporary_File_3(WamWord dir_word, WamWord prefix_word,
		 WamWord path_name_word)
{
  char *dir;
  char *prefix;
  char *path_name;

  dir = Rd_String_Check(dir_word);
  if (*dir == '\0')
    dir = NULL;
  else
    dir = Get_Path_Name(dir_word);

  prefix = Rd_String_Check(prefix_word);
  if (*prefix == '\0')
    prefix = NULL;

  errno = -1;
  path_name = (char *) tempnam(dir, prefix);
  Os_Test_Error(path_name == NULL);

  return path_name && Un_String_Check(path_name, path_name_word);
}




/*-------------------------------------------------------------------------*
 * DATE_TIME_1                                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Date_Time_1(WamWord date_time_word)
{
  time_t t;

  t = time(NULL);

  return Date_Time_To_Prolog(&t, date_time_word);
}




/*-------------------------------------------------------------------------*
 * HOST_NAME_1                                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Host_Name_1(WamWord host_name_word)
{
  WamWord word, tag, *adr;
  int atom;
  static int atom_host_name = -1;	/* not created in an init since implies to */

  /* establish a connection (if !NO_USE_SOCKETS) */

  if (atom_host_name < 0)
    atom_host_name = Create_Allocate_Atom(M_Host_Name_From_Name(NULL));

  Deref(host_name_word, word, tag, adr);
  if (tag == REF)
    return Get_Atom(atom_host_name, host_name_word);

  atom = Rd_Atom_Check(word);

  return atom == atom_host_name ||
    strcmp(M_Host_Name_From_Name(atom_tbl[atom].name),
	   atom_tbl[atom_host_name].name) == 0;
}




/*-------------------------------------------------------------------------*
 * OS_VERSION_1                                                            *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Os_Version_1(WamWord os_version_word)
{
  return Un_String_Check(m_os_version, os_version_word);
}




/*-------------------------------------------------------------------------*
 * ARCHITECTURE_1                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Architecture_1(WamWord architecture_word)
{
  return Un_String_Check(m_architecture, architecture_word);
}








/*-------------------------------------------------------------------------*
 * SLEEP_1                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Sleep_1(WamWord seconds_word)
{
#ifdef M_ix86_win32
  DWORD ms;

  ms = (DWORD) (Rd_Number_Check(seconds_word) * 1000);

  if (ms < 0)
    Pl_Err_Domain(domain_not_less_than_zero, seconds_word);

  Sleep(ms);
#else
  long us;

  us = (long) (Rd_Number_Check(seconds_word) * 1000000);

  if (us < 0)
    Pl_Err_Domain(domain_not_less_than_zero, seconds_word);

  usleep(us);
#endif
}




/*-------------------------------------------------------------------------*
 * SHELL_2                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Shell_2(WamWord cmd_word, WamWord status_word)
{
  char *cmd;
  int status;

  cmd = Rd_String_Check(cmd_word);
  if (*cmd == '\0')
    cmd = NULL;
  Check_For_Un_Integer(status_word);

  Flush_All_Streams();
  status = M_Shell(cmd);

  return Get_Integer(status, status_word);
}




/*-------------------------------------------------------------------------*
 * SYSTEM_2                                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
System_2(WamWord cmd_word, WamWord status_word)
{
  char *cmd;
  int status;

  cmd = Rd_String_Check(cmd_word);
  Check_For_Un_Integer(status_word);

#ifdef M_ix86_win32
  _flushall();
#endif

  Flush_All_Streams();
  status = system(cmd);

  return Get_Integer(status, status_word);
}




/*-------------------------------------------------------------------------*
 * SPAWN_3                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Spawn_3(WamWord cmd_word, WamWord list_word, WamWord status_word)
{
  WamWord word, tag, *adr;
  WamWord save_list_word;
  WamWord *lst_adr;
  char *arg[MAX_SPAWN_ARGS];
  char **p = arg;
  char err[64];
  int status;

  save_list_word = list_word;

  *p++ = Rd_String_Check(cmd_word);

  for (;;)
    {
      Deref(list_word, word, tag, adr);

      if (tag == REF)
	Pl_Err_Instantiation();

      if (word == NIL_WORD)
	break;

      if (tag != LST)
	Pl_Err_Type(type_list, save_list_word);

      lst_adr = UnTag_LST(word);

      *p++ = Rd_String_Check(Car(lst_adr));

      list_word = Cdr(lst_adr);
    }

  *p = NULL;
  Check_For_Un_Integer(status_word);

  Flush_All_Streams();
  status = M_Spawn(arg);

  Os_Test_Error(status == -1);
  if (status == -2)
    {
      sprintf(err, "error trying to execute %s", arg[0]);
      Pl_Err_System(Create_Allocate_Atom(err));
      return FALSE;
    }

  return Get_Integer(status, status_word);
}




/*-------------------------------------------------------------------------*
 * POPEN_3                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Popen_3(WamWord cmd_word, WamWord mode_word, WamWord stm_word)
{
  char *cmd;
  StmProp prop;
  int stm;
  FILE *f;
  char open_str[10];
  char stream_name[512];

  cmd = Rd_String_Check(cmd_word);
  prop = Get_Stream_Mode(mode_word, TRUE, open_str);

  prop.text = TRUE;
  prop.reposition = FALSE;
  prop.eof_action = STREAM_EOF_ACTION_RESET;
  prop.buffering = STREAM_BUFFERING_LINE;
  prop.tty = FALSE;
  prop.special_close = FALSE;
  prop.other = 0;

  Flush_All_Streams();
  f = popen(cmd, open_str);
  Os_Test_Error(f == NULL);

  sprintf(stream_name, "popen_stream('%.*s')",
	  (int) sizeof(stream_name) - 20, cmd);
  stm = Add_Stream(Create_Allocate_Atom(stream_name), (long) f, prop,
		   NULL, NULL, NULL, (StmFct) pclose, NULL, NULL, NULL);

  return Get_Integer(stm, stm_word);
}




/*-------------------------------------------------------------------------*
 * EXEC_5                                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Exec_5(WamWord cmd_word, WamWord stm_in_word, WamWord stm_out_word,
       WamWord stm_err_word, WamWord pid_word)
{
  char *cmd;
  char **arg;
  StmProp prop;
  int stm;
  FILE *f_in, *f_out, *f_err;
  int pid;
  int mask = SYS_VAR_OPTION_MASK;
  char stream_name[512];
  int atom;
  char err[64];

  cmd = Rd_String_Check(cmd_word);
  arg = M_Create_Shell_Command(cmd);

  Flush_All_Streams();
  pid = M_Spawn_Redirect(arg, (mask & 1) == 0, &f_in, &f_out, &f_err);
  Os_Test_Error(pid == -1);
  if (pid == -2)
    {
      sprintf(err, "error trying to execute %s", cmd);
      Pl_Err_System(Create_Allocate_Atom(err));
      return FALSE;
    }

  if (mask & 1)			/* pid needed ? */
    Get_Integer(pid, pid_word);

  sprintf(stream_name, "exec_stream('%.*s')",
	  (int) sizeof(stream_name) - 20, cmd);
  atom = Create_Allocate_Atom(stream_name);

  prop.mode = STREAM_MODE_WRITE;
  prop.input = FALSE;
  prop.output = TRUE;
  prop.text = TRUE;
  prop.reposition = FALSE;
  prop.eof_action = STREAM_EOF_ACTION_RESET;
  prop.buffering = STREAM_BUFFERING_LINE;
  prop.tty = FALSE;
  prop.special_close = FALSE;
  prop.other = 0;
  stm =
    Add_Stream(atom, (long) f_in, prop, NULL, NULL, NULL, NULL, NULL, NULL,
	       NULL);
  Get_Integer(stm, stm_in_word);
#ifdef DEBUG
  DBGPRINTF("Add_Stream(Input)=%d\n", stm);
#endif

  prop.mode = STREAM_MODE_READ;
  prop.input = TRUE;
  prop.output = FALSE;
  stm =
    Add_Stream(atom, (long) f_out, prop, NULL, NULL, NULL, NULL, NULL, NULL,
	       NULL);
  Get_Integer(stm, stm_out_word);
#ifdef DEBUG
  DBGPRINTF("Add_Stream(Output)=%d\n", stm);
#endif

  stm =
    Add_Stream(atom, (long) f_err, prop, NULL, NULL, NULL, NULL, NULL, NULL,
	       NULL);
  Get_Integer(stm, stm_err_word);
#ifdef DEBUG
  DBGPRINTF("Add_Stream(Error)=%d\n", stm);
#endif

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * SELECT_5                                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Select_5(WamWord reads_word, WamWord ready_reads_word,
	 WamWord writes_word, WamWord ready_writes_word,
	 WamWord time_out_word)
{
#if defined(M_ix86_win32) && defined(NO_USE_SOCKETS)

  Pl_Err_Resource(Create_Atom("not implemented"));
  return FALSE;

#else

  double time_out;
  struct timeval *p, t;
  fd_set read_set, write_set;
  int max, n;

  max = Select_Init_Set(reads_word, &read_set, STREAM_CHECK_INPUT);
  Check_For_Un_List(ready_reads_word);
  n = Select_Init_Set(writes_word, &write_set, STREAM_CHECK_OUTPUT);
  if (n > max)
    max = n;

  Check_For_Un_List(ready_writes_word);


  time_out = Rd_Number_Check(time_out_word);
  if (time_out <= 0)
    p = NULL;
  else
    {
      t.tv_sec = (long) (time_out / 1000);
      t.tv_usec = (long) (fmod(time_out, 1000) * 1000);
      p = &t;
    }


  Os_Test_Error(select(max + 1, &read_set, &write_set, NULL, p) < 0);


  return Select_Init_Ready_List(reads_word, &read_set, ready_reads_word) &&
    Select_Init_Ready_List(writes_word, &write_set, ready_writes_word);

#endif
}




/*-------------------------------------------------------------------------*
 * SELECT_INIT_SET                                                         *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static int
Select_Init_Set(WamWord list_word, fd_set *set, int check)
{
  WamWord word, tag, *adr;
  WamWord save_list_word;
  WamWord *lst_adr;
  int stm;
  int fd, max = 0;

  FD_ZERO(set);

  save_list_word = list_word;
  for (;;)
    {
      Deref(list_word, word, tag, adr);

      if (tag == REF)
	Pl_Err_Instantiation();

      if (word == NIL_WORD)
	break;

      if (tag != LST)
	Pl_Err_Type(type_list, save_list_word);

      lst_adr = UnTag_LST(word);
      Deref(Car(lst_adr), word, tag, adr);
      if (tag == INT)
	fd = Rd_Positive_Check(word);
      else
	{
	  stm = Get_Stream_Or_Alias(word, check);

	  fd = File_Number_Of_Stream(stm);
	  if (fd < 0)
	    Pl_Err_Domain(domain_selectable_item, word);
	}

#ifdef FD_SETSIZE
      if (fd >= FD_SETSIZE)
	{
	  errno = EBADF;
	  Os_Test_Error(1);
	}
#endif

      FD_SET(fd, set);
      if (fd > max)
	max = fd;
      list_word = Cdr(lst_adr);
    }

  return max;
}




/*-------------------------------------------------------------------------*
 * SELECT_INIT_READY_LIST                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static Bool
Select_Init_Ready_List(WamWord list_word, fd_set *set,
		       WamWord ready_list_word)
{
  WamWord word, tag, *adr;
  WamWord *lst_adr;
  int stm;
  int fd;

  for (;;)
    {
      Deref(list_word, word, tag, adr);
      if (word == NIL_WORD)
	break;

      lst_adr = UnTag_LST(word);
      Deref(Car(lst_adr), word, tag, adr);

      if (tag == INT)
	fd = UnTag_INT(word);
      else
	{
	  stm = Get_Stream_Or_Alias(word, STREAM_CHECK_VALID);
	  fd = File_Number_Of_Stream(stm);
	}

      if (FD_ISSET(fd, set))
	{
	  if (!Get_List(ready_list_word) || !Unify_Value(word))
	    return FALSE;

	  ready_list_word = Unify_Variable();
	}

      list_word = Cdr(lst_adr);
    }

  return Get_Nil(ready_list_word);
}




/*-------------------------------------------------------------------------*
 * PROLOG_PID_1                                                            *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Prolog_Pid_1(WamWord prolog_pid_word)
{
  int prolog_pid;

  prolog_pid = (int) getpid();

  return Un_Integer_Check(prolog_pid, prolog_pid_word);
}




/*-------------------------------------------------------------------------*
 * SEND_SIGNAL_2                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Send_Signal_2(WamWord pid_word, WamWord signal_word)
{
  WamWord word, tag, *adr;
  int pid;
  int sig;
  int atom;
  int i;

  pid = Rd_Integer_Check(pid_word);

  Deref(signal_word, word, tag, adr);
  if (tag == ATM)
    {
      atom = UnTag_ATM(word);
      sig = -1;
      for (i = 0; i < nb_sig; i++)
	if (tsig[i].atom == atom)
	  {
	    sig = tsig[i].sig;
	    break;
	  }
    }
  else
    sig = Rd_Integer_Check(word);

#ifdef M_ix86_win32
  {
    int ret;

    if (pid != _getpid())
      {
	errno = EINVAL;
	ret = 1;
      }
    else
      {
	errno = -1;
	ret = raise(sig);
      }

    Os_Test_Error(ret);
  }
#else
  Os_Test_Error(kill(pid, sig));
#endif

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * WAIT_2                                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Wait_2(WamWord pid_word, WamWord status_word)
{
  int pid;
  int status;

  pid = Rd_Integer_Check(pid_word);
  Check_For_Un_Integer(status_word);

#ifdef M_ix86_win32
  Os_Test_Error(_cwait(&status, pid, _WAIT_CHILD) == -1);
#else
  Os_Test_Error(waitpid(pid, &status, 0) == -1);

  if (WIFEXITED(status))
    status = WEXITSTATUS(status);
#endif

  return Get_Integer(status, status_word);
}




/*-------------------------------------------------------------------------*
 * GET_PATH_NAME                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static char *
Get_Path_Name(WamWord path_name_word)
{
  char *path_name;

  path_name = Rd_String_Check(path_name_word);
  if ((path_name = M_Absolute_Path_Name(path_name)) == NULL)
    Pl_Err_Domain(domain_os_path, path_name_word);

  return path_name;
}




/*-------------------------------------------------------------------------*
 * DATE_TIME_TO_PROLOG                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static Bool
Date_Time_To_Prolog(time_t *t, WamWord date_time_word)
{
  WamWord word, tag, *adr;
  WamWord year_word, month_word, day_word;
  WamWord hour_word, minute_word, second_word;
  struct tm *tm;
  int day, month, year;
  int hour, minute, second;

  tm = localtime(t);

  year = tm->tm_year + 1900;
  month = tm->tm_mon + 1;
  day = tm->tm_mday;
  hour = tm->tm_hour;
  minute = tm->tm_min;
  second = tm->tm_sec;


  Deref(date_time_word, word, tag, adr);
  if (tag != REF && tag != LST && tag != STC)
    Pl_Err_Type(type_compound, word);

  if (!Get_Structure(atom_dt, 6, word))
    Pl_Err_Domain(domain_date_time, word);

  year_word = Unify_Variable();
  month_word = Unify_Variable();
  day_word = Unify_Variable();
  hour_word = Unify_Variable();
  minute_word = Unify_Variable();
  second_word = Unify_Variable();

  Check_For_Un_Integer(year_word);
  Check_For_Un_Integer(month_word);
  Check_For_Un_Integer(day_word);
  Check_For_Un_Integer(hour_word);
  Check_For_Un_Integer(minute_word);
  Check_For_Un_Integer(second_word);

  return Get_Integer(year, year_word) &&
    Get_Integer(month, month_word) &&
    Get_Integer(day, day_word) &&
    Get_Integer(hour, hour_word) &&
    Get_Integer(minute, minute_word) && Get_Integer(second, second_word);
}
/*-------------------------------------------------------------------------*
 * GNU Prolog                                                              *
 *                                                                         *
 * Part  : Prolog buit-in predicates                                       *
 * File  : stream_c.c                                                      *
 * Descr.: stream selection and control management - C part                *
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
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "engine_pl.h"
#include "bips_pl.h"

#if defined(M_ix86_cygwin) || defined(M_ix86_win32)
#include <io.h>
#endif

#ifndef M_ix86_win32
#include <unistd.h>
#include <sys/fcntl.h>
#endif




/*---------------------------------*
 * Constants                       *
 *---------------------------------*/

#define TERM_STREAM_WRITE_BLOCK    1024

#define TERM_STREAM_ATOM           0
#define TERM_STREAM_CHARS          1
#define TERM_STREAM_CODES          2




	  /* Error Messages */

#define ERR_NEEDS_SPECIAL_CLOSE    "special stream: needs appropriate close predicate"
#define ERR_CANNOT_CLOSE_STREAM    "cannot close stream"




/*---------------------------------*
 * Type Definitions                *
 *---------------------------------*/

typedef struct
{
  int buff_size;
  Bool buff_is_alloc;
  char *buff;
  char *ptr;
}
TermSInf;




/*---------------------------------*
 * Global Variables                *
 *---------------------------------*/

/*---------------------------------*
 * Function Prototypes             *
 *---------------------------------*/

#define CURRENT_STREAM_ALT         X2463757272656E745F73747265616D5F616C74

#define CURRENT_ALIAS_ALT          X2463757272656E745F616C6961735F616C74

Prolog_Prototype(CURRENT_STREAM_ALT, 0)
Prolog_Prototype(CURRENT_ALIAS_ALT, 0)




/*-------------------------------------------------------------------------*
 * CURRENT_INPUT_1                                                         *
 *                                                                         *
 *-------------------------------------------------------------------------*/
     Bool Current_Input_1(WamWord stm_word)
{
  return Get_Integer(stm_input, stm_word);
}




/*-------------------------------------------------------------------------*
 * CURRENT_OUTPUT_1                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Current_Output_1(WamWord stm_word)
{
  return Get_Integer(stm_output, stm_word);
}




/*-------------------------------------------------------------------------*
 * SET_INPUT_1                                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Input_1(WamWord sora_word)
{
  stm_input = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_INPUT);
}




/*-------------------------------------------------------------------------*
 * SET_OUTPUT_1                                                            *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Output_1(WamWord sora_word)
{
  stm_output = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_OUTPUT);
}




/*-------------------------------------------------------------------------*
 * SET_TOP_LEVEL_STREAMS_2                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Top_Level_Streams_2(WamWord sora_in_word, WamWord sora_out_word)
{
  stm_top_level_input =
    Get_Stream_Or_Alias(sora_in_word, STREAM_CHECK_INPUT);
  stm_top_level_output =
    Get_Stream_Or_Alias(sora_out_word, STREAM_CHECK_OUTPUT);

  Reassign_Alias(atom_top_level_input, stm_top_level_input);
  Reassign_Alias(atom_top_level_output, stm_top_level_output);
}




/*-------------------------------------------------------------------------*
 * SET_DEBUGGER_STREAMS_2                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Debugger_Streams_2(WamWord sora_in_word, WamWord sora_out_word)
{
  stm_debugger_input =
    Get_Stream_Or_Alias(sora_in_word, STREAM_CHECK_INPUT);
  stm_debugger_output =
    Get_Stream_Or_Alias(sora_out_word, STREAM_CHECK_OUTPUT);

  Reassign_Alias(atom_debugger_input, stm_debugger_input);
  Reassign_Alias(atom_debugger_output, stm_debugger_output);
}




/*-------------------------------------------------------------------------*
 * OPEN_3                                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Open_3(WamWord source_sink_word, WamWord mode_word, WamWord stm_word)
{
  WamWord word, tag, *adr;
  StmProp prop;
  char *path;
  int atom_file_name;
  int stm;
  char open_str[10];
  FILE *f;
  long offset;
  int mask = SYS_VAR_OPTION_MASK;


  Deref(source_sink_word, word, tag, adr);
  if (tag == REF)
    Pl_Err_Instantiation();
  if (tag != ATM)
    Pl_Err_Domain(domain_source_sink, source_sink_word);

  atom_file_name = UnTag_ATM(word);
  path = atom_tbl[atom_file_name].name;
  if ((path = M_Absolute_Path_Name(path)) == NULL)
    Pl_Err_Existence(existence_source_sink, source_sink_word);

  prop = Get_Stream_Mode(mode_word, FALSE, open_str);

  prop.text = mask & 1;
  mask >>= 1;

  strcat(open_str, (prop.text) ? "t" : "b");

  if ((f = fopen(path, open_str)) == NULL)
    {
      if (errno == ENOENT || errno == ENOTDIR)
	Pl_Err_Existence(existence_source_sink, source_sink_word);
      else
	Pl_Err_Permission(permission_operation_open,
			  permission_type_source_sink, source_sink_word);
    }

  prop.tty = (isatty(fileno(f)) != 0);

  if ((mask & 2) == 0)		/* not specified - set to default */
    prop.reposition = !(prop.tty);
  else
    prop.reposition = mask & 1;
  mask >>= 2;


  if ((mask & 4) == 0)		/* not specified - set to default */
    prop.eof_action = (prop.tty) ? STREAM_EOF_ACTION_RESET
      : STREAM_EOF_ACTION_EOF_CODE;
  else
    prop.eof_action = mask & 3;
  mask >>= 2;


  if ((mask & 4) == 0)		/* not specified - set to default */
    prop.buffering = (prop.tty) ? STREAM_BUFFERING_LINE
      : STREAM_BUFFERING_BLOCK;
  else
    prop.buffering = mask & 3;
  mask >>= 2;

  if (prop.reposition)
    {
      offset = ftell(f);
      if (fseek(f, 0, SEEK_END) != 0 && errno != 0)
	{
	  word = Put_Structure(atom_reposition, 1);
	  Unify_Atom(atom_true);
	  Pl_Err_Permission(permission_operation_open,
			    permission_type_source_sink, word);
	}
      fseek(f, offset, SEEK_SET);
    }

  prop.special_close = FALSE;
  prop.other = 0;

  stm = Add_Stream(atom_file_name, (long) f, prop,
		   NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  Get_Integer(stm, stm_word);
}




/*-------------------------------------------------------------------------*
 * TEST_ALIAS_NOT_ASSIGNED_1                                               *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Test_Alias_Not_Assigned_1(WamWord alias_word)
{
  return Find_Stream_By_Alias(Rd_Atom_Check(alias_word)) < 0;
}




/*-------------------------------------------------------------------------*
 * FROM_ALIAS_TO_STREAM_2                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
From_Alias_To_Stream_2(WamWord alias_word, WamWord stm_word)
{
  int stm;

  stm = Find_Stream_By_Alias(Rd_Atom_Check(alias_word));

  return stm >= 0 && Get_Integer(stm, stm_word);
}




/*-------------------------------------------------------------------------*
 * ADD_STREAM_ALIAS_2                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Add_Stream_Alias_2(WamWord sora_word, WamWord alias_word)
{
  int stm;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);

  return Add_Alias_To_Stream(Rd_Atom_Check(alias_word), stm);
}




/*-------------------------------------------------------------------------*
 * SET_STREAM_TYPE_2                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Stream_Type_2(WamWord sora_word, WamWord is_text_word)
{
  int stm;
  StmInf *pstm;
  int text;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  text = Rd_Integer_Check(is_text_word);
  if ((unsigned) text == pstm->prop.text)
    return;

  if (pstm->char_count)
    Pl_Err_Permission(permission_operation_modify,
		      permission_type_stream, sora_word);

  pstm->prop.text = text;
#if defined(M_ix86_cygwin) || defined(M_ix86_win32)
  {
    FILE *f;

    f = File_Star_Of_Stream(stm);
    if (f == NULL)
      return;

    setmode(fileno(f), (text) ? O_TEXT : O_BINARY);
  }
#endif
}




/*-------------------------------------------------------------------------*
 * SET_STREAM_EOF_ACTION_2                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Stream_Eof_Action_2(WamWord sora_word, WamWord action_word)
{
  int stm;
  StmInf *pstm;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (pstm->prop.output)
    Pl_Err_Permission(permission_operation_modify,
		      permission_type_stream, sora_word);

  pstm->prop.eof_action = Rd_Integer_Check(action_word);
}




/*-------------------------------------------------------------------------*
 * SET_STREAM_BUFFERING_2                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Set_Stream_Buffering_2(WamWord sora_word, WamWord buff_mode_word)
{
  int stm;
  StmInf *pstm;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  pstm->prop.buffering = Rd_Integer_Check(buff_mode_word);
  Set_Stream_Buffering(stm);
}




/*-------------------------------------------------------------------------*
 * CLOSE_1                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Close_1(WamWord sora_word)
{
  int stm;
  StmInf *pstm;
  StmProp prop;
  int mask = SYS_VAR_OPTION_MASK;
  int fd = 0;


  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);

  pstm = stm_tbl + stm;
  prop = pstm->prop;

  Stream_Flush(pstm);

  if (stm == stm_stdin || stm == stm_stdout)
    return;

  if (stm == stm_top_level_input || stm == stm_top_level_output)
    return;

  if (stm == stm_debugger_input || stm == stm_debugger_output)
    return;

  if (stm == stm_input)
    stm_input = stm_stdin;
  else if (stm == stm_output)
    stm_output = stm_stdout;

  if (pstm->prop.special_close)
    Pl_Err_System(Create_Atom(ERR_NEEDS_SPECIAL_CLOSE));

  if (pstm->fct_close == fclose)
    fd = fileno((FILE *) (pstm->file));

  if (Stream_Close(pstm) != 0)
    {
      if ((mask & 1) == 0)
	Pl_Err_System(Create_Atom(ERR_CANNOT_CLOSE_STREAM));

      /* else force close */
      if (fd > 2)
	close(fd);
    }

  Delete_Stream(stm);
}




/*-------------------------------------------------------------------------*
 * FLUSH_OUTPUT_1                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Flush_Output_1(WamWord sora_word)
{
  int stm;


  stm = (sora_word == NOT_A_WAM_WORD)
    ? stm_output : Get_Stream_Or_Alias(sora_word, STREAM_CHECK_OUTPUT);

  last_output_sora = sora_word;

  Stream_Flush(stm_tbl + stm);
}




/*-------------------------------------------------------------------------*
 * FLUSH_OUTPUT_0                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Flush_Output_0(void)
{
  Flush_Output_1(NOT_A_WAM_WORD);
}




/*-------------------------------------------------------------------------*
 * CURRENT_STREAM_1                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Current_Stream_1(WamWord stm_word)
{
  WamWord word, tag, *adr;
  int i = 0;


  Deref(stm_word, word, tag, adr);	/* either an INT or a REF */
  if (tag == INT)
    return stm_tbl[UnTag_INT(word)].file != 0;


  for (; i <= stm_last_used; i++)
    if (stm_tbl[i].file != 0)
      break;

  if (i >= stm_last_used)
    {
      if (i > stm_last_used)
	return FALSE;
    }
  else				/* non deterministic case */
    {
      A(0) = stm_word;
      A(1) = i + 1;
      Create_Choice_Point((CodePtr) Prolog_Predicate(CURRENT_STREAM_ALT, 0),
			  2);
    }

  Get_Integer(i, stm_word);
  return TRUE;
}




/*-------------------------------------------------------------------------*
 * CURRENT_STREAM_ALT_0                                                    *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Current_Stream_Alt_0(void)
{
  WamWord stm_word;
  int i;

  Update_Choice_Point((CodePtr) Prolog_Predicate(CURRENT_STREAM_ALT, 0), 0);

  stm_word = AB(B, 0);
  i = AB(B, 1);



  for (; i <= stm_last_used; i++)
    if (stm_tbl[i].file != 0)
      break;

  if (i >= stm_last_used)
    {
      Delete_Last_Choice_Point();
      if (i > stm_last_used)
	return FALSE;
    }
  else				/* non deterministic case */
    {
/*   AB(B,0)=stm_word;                                         not changed */
      AB(B, 1) = i + 1;
    }

  Get_Integer(i, stm_word);
  return TRUE;
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_FILE_NAME_2                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_File_Name_2(WamWord file_name_word, WamWord stm_word)
{
  int stm;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  return Un_Atom_Check(stm_tbl[stm].atom_file_name, file_name_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_MODE_2                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Mode_2(WamWord mode_word, WamWord stm_word)
{
  int stm;
  int atom;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  switch (stm_tbl[stm].prop.mode)
    {
    case STREAM_MODE_READ:
      atom = atom_read;
      break;

    case STREAM_MODE_WRITE:
      atom = atom_write;
      break;

    case STREAM_MODE_APPEND:
      atom = atom_append;
      break;
    }

  return Un_Atom_Check(atom, mode_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_INPUT_1                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Input_1(WamWord stm_word)
{
  int stm;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  return stm_tbl[stm].prop.input;
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_OUTPUT_1                                                    *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Output_1(WamWord stm_word)
{
  int stm;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  return stm_tbl[stm].prop.output;
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_TYPE_2                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Type_2(WamWord type_word, WamWord stm_word)
{
  int stm;
  int atom;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  atom = (stm_tbl[stm].prop.text) ? atom_text : atom_binary;

  return Un_Atom_Check(atom, type_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_REPOSITION_2                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Reposition_2(WamWord reposition_word, WamWord stm_word)
{
  int stm;
  int atom;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  atom = (stm_tbl[stm].prop.reposition) ? atom_true : atom_false;

  return Un_Atom_Check(atom, reposition_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_EOF_ACTION_2                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Eof_Action_2(WamWord eof_action_word, WamWord stm_word)
{
  int stm;
  int atom;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  switch (stm_tbl[stm].prop.eof_action)
    {
    case STREAM_EOF_ACTION_ERROR:
      atom = atom_error;
      break;

    case STREAM_EOF_ACTION_EOF_CODE:
      atom = atom_eof_code;
      break;

    case STREAM_EOF_ACTION_RESET:
      atom = atom_reset;
      break;
    }

  return Un_Atom_Check(atom, eof_action_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_BUFFERING_2                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_Buffering_2(WamWord buffering_word, WamWord stm_word)
{
  int stm;
  int atom;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  switch (stm_tbl[stm].prop.buffering)
    {
    case STREAM_BUFFERING_NONE:
      atom = atom_none;
      break;

    case STREAM_BUFFERING_LINE:
      atom = atom_line;
      break;

    case STREAM_BUFFERING_BLOCK:
      atom = atom_block;
      break;
    }

  return Un_Atom_Check(atom, buffering_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_PROP_END_OF_STREAM_2                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Prop_End_Of_Stream_2(WamWord end_of_stream_word, WamWord stm_word)
{
  int stm;
  int atom;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  switch (Stream_End_Of_Stream(stm_tbl + stm))
    {
    case STREAM_EOF_NOT:
      atom = atom_not;
      break;

    case STREAM_EOF_AT:
      atom = atom_at;
      break;

    case STREAM_EOF_PAST:
      atom = atom_past;
      break;
    }

  return Un_Atom_Check(atom, end_of_stream_word);
}




/*-------------------------------------------------------------------------*
 * AT_END_OF_STREAM_1                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
At_End_Of_Stream_1(WamWord sora_word)
{
  int stm;

  stm = (sora_word == NOT_A_WAM_WORD)
    ? stm_input : Get_Stream_Or_Alias(sora_word, STREAM_CHECK_INPUT);

  return Stream_End_Of_Stream(stm_tbl + stm) != STREAM_EOF_NOT;
}




/*-------------------------------------------------------------------------*
 * AT_END_OF_STREAM_0                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
At_End_Of_Stream_0(void)
{
  return At_End_Of_Stream_1(NOT_A_WAM_WORD);
}




/*-------------------------------------------------------------------------*
 * CURRENT_ALIAS_2                                                         *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Current_Alias_2(WamWord stm_word, WamWord alias_word)
{
  WamWord word, tag, *adr;
  int stm;
  HashScan scan;
  AliasInf *alias;
  AliasInf *save_alias;

  stm = Rd_Integer_Check(stm_word);	/* stm is a valid stream entry */

  Deref(alias_word, word, tag, adr);
  if (tag != REF)
    return Find_Stream_By_Alias(Rd_Atom_Check(word)) == stm;

  for (alias = (AliasInf *) Hash_First(alias_tbl, &scan); alias;
       alias = (AliasInf *) Hash_Next(&scan))
    if (alias->stm == stm)
      break;

  if (alias == NULL)
    return FALSE;

  save_alias = alias;

  for (;;)
    {
      alias = (AliasInf *) Hash_Next(&scan);
      if (alias == NULL || alias->stm == stm)
	break;
    }


  if (alias)			/* non deterministic case */
    {
      A(0) = stm;
      A(1) = alias_word;
      A(2) = (WamWord) scan.endt;
      A(3) = (WamWord) scan.cur_t;
      A(4) = (WamWord) scan.cur_p;
      A(5) = (WamWord) alias;
      Create_Choice_Point((CodePtr) Prolog_Predicate(CURRENT_ALIAS_ALT, 0),
			  6);
    }

  Get_Atom(save_alias->atom, alias_word);
  return TRUE;
}




/*-------------------------------------------------------------------------*
 * CURRENT_ALIAS_ALT_0                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Current_Alias_Alt_0(void)
{
  int stm;
  WamWord alias_word;
  HashScan scan;
  AliasInf *alias;
  AliasInf *save_alias;


  Update_Choice_Point((CodePtr) Prolog_Predicate(CURRENT_ALIAS_ALT, 0), 0);

  stm = AB(B, 0);
  alias_word = AB(B, 1);
  scan.endt = (char *) AB(B, 2);
  scan.cur_t = (char *) AB(B, 3);
  scan.cur_p = (char *) AB(B, 4);
  alias = (AliasInf *) AB(B, 5);


  save_alias = alias;

  for (;;)
    {
      alias = (AliasInf *) Hash_Next(&scan);
      if (alias == NULL || alias->stm == stm)
	break;
    }


  if (alias)			/* non deterministic case */
    {
/*   AB(B,0)=          stm;                                    not changed */
/*   AB(B,1)=          alias_word;                             not changed */
/*   AB(B,2)=(WamWord) scan.endt;                              not changed */
      AB(B, 3) = (WamWord) scan.cur_t;
      AB(B, 4) = (WamWord) scan.cur_p;
      AB(B, 5) = (WamWord) alias;
    }
  else
    Delete_Last_Choice_Point();

  Get_Atom(save_alias->atom, alias_word);
  return TRUE;
}




/*-------------------------------------------------------------------------*
 * STREAM_POSITION_2                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Position_2(WamWord sora_word, WamWord position_word)
{
  WamWord word, tag, *adr;
  WamWord p_word[4];
  int p[4];
  int i;
  int stm;
  StmInf *pstm;


  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  Stream_Get_Position(pstm, p, p + 1, p + 2, p + 3);

  if (!Get_Structure(atom_stream_position, 4, position_word))
  dom_error:
    Pl_Err_Domain(domain_stream_position, position_word);

  for (i = 0; i < 4; i++)
    {
      p_word[i] = Unify_Variable();

      Deref(p_word[i], word, tag, adr);
      if (tag != REF && tag != INT)
	goto dom_error;
    }

  for (i = 0; i < 4; i++)
    if (!Get_Integer(p[i], p_word[i]))
      return FALSE;

  return TRUE;
}




/*-------------------------------------------------------------------------*
 * SET_STREAM_POSITION_2                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Set_Stream_Position_2(WamWord sora_word, WamWord position_word)
{
  WamWord word, tag, *adr;
  WamWord p_word[4];
  int p[4];
  int i;
  int stm;
  StmInf *pstm;


  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (!pstm->prop.reposition)
    Pl_Err_Permission(permission_operation_reposition,
		      permission_type_stream, sora_word);

  Deref(position_word, word, tag, adr);
  if (tag == REF)
    Pl_Err_Instantiation();

  if (!Get_Structure(atom_stream_position, 4, position_word))
  dom_error:
    Pl_Err_Domain(domain_stream_position, position_word);

  for (i = 0; i < 4; i++)
    {
      p_word[i] = Unify_Variable();

      Deref(p_word[i], word, tag, adr);
      if (tag == REF)
	Pl_Err_Instantiation();

      if (tag != INT)
	goto dom_error;

      p[i] = UnTag_INT(word);
    }

  return Stream_Set_Position(pstm, SEEK_SET, p[0], p[1], p[2], p[3]) == 0;
}




/*-------------------------------------------------------------------------*
 * SEEK_4                                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Seek_4(WamWord sora_word, WamWord whence_word, WamWord offset_word,
       WamWord new_loc_word)
{
  int stm;
  StmInf *pstm;
  int whence;
  int offset;
  int atom;
  int p[4];


  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (!pstm->prop.reposition)
    Pl_Err_Permission(permission_operation_reposition,
		      permission_type_stream, sora_word);

  if (pstm->prop.text)
    Pl_Err_Permission(permission_operation_reposition,
		      permission_type_text_stream, sora_word);

  atom = Rd_Atom_Check(whence_word);

  if (atom == atom_bof)
    whence = SEEK_SET;
  else if (atom == atom_current)
    whence = SEEK_CUR;
  else if (atom == atom_eof)
    whence = SEEK_END;
  else
    Pl_Err_Domain(domain_stream_seek_method, whence_word);

  offset = Rd_Integer_Check(offset_word);
  Check_For_Un_Integer(new_loc_word);

  if (Stream_Set_Position(pstm, whence, offset, offset, 0, 0) != 0)
    return FALSE;

  Stream_Get_Position(pstm, &offset, p + 1, p + 2, p + 3);

  return Get_Integer(offset, new_loc_word);
}




/*-------------------------------------------------------------------------*
 * CHARACTER_COUNT_2                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Character_Count_2(WamWord sora_word, WamWord count_word)
{
  int stm;
  StmInf *pstm;
  int offset, char_count, line_count, line_pos;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  Stream_Get_Position(pstm, &offset, &char_count, &line_count, &line_pos);

  return Un_Integer_Check(char_count, count_word);
}




/*-------------------------------------------------------------------------*
 * LINE_COUNT_2                                                            *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Line_Count_2(WamWord sora_word, WamWord count_word)
{
  int stm;
  StmInf *pstm;
  int offset, char_count, line_count, line_pos;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (!pstm->prop.text)
    Pl_Err_Permission(permission_operation_access,
		      permission_type_binary_stream, sora_word);

  Stream_Get_Position(pstm, &offset, &char_count, &line_count, &line_pos);

  return Un_Integer_Check(line_count, count_word);
}




/*-------------------------------------------------------------------------*
 * LINE_POSITION_2                                                         *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Line_Position_2(WamWord sora_word, WamWord count_word)
{
  int stm;
  StmInf *pstm;
  int offset, char_count, line_count, line_pos;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (!pstm->prop.text)
    Pl_Err_Permission(permission_operation_access,
		      permission_type_binary_stream, sora_word);

  Stream_Get_Position(pstm, &offset, &char_count, &line_count, &line_pos);

  return Un_Integer_Check(line_pos, count_word);
}




/*-------------------------------------------------------------------------*
 * STREAM_LINE_COLUMN_3                                                    *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Stream_Line_Column_3(WamWord sora_word, WamWord line_word, WamWord col_word)
{
  int stm;
  StmInf *pstm;
  int offset, char_count, line_count, line_pos;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (!pstm->prop.text)
    Pl_Err_Permission(permission_operation_access,
		      permission_type_binary_stream, sora_word);

  Stream_Get_Position(pstm, &offset, &char_count, &line_count, &line_pos);

  return Un_Integer_Check(line_count + 1, line_word) &&
    Un_Integer_Check(line_pos + 1, col_word);
}




/*-------------------------------------------------------------------------*
 * SET_STREAM_LINE_COLUMN_3                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Set_Stream_Line_Column_3(WamWord sora_word, WamWord line_word,
			 WamWord col_word)
{
  int stm;
  StmInf *pstm;
  int line_count, line_pos;


  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  if (!pstm->prop.reposition)
    Pl_Err_Permission(permission_operation_reposition,
		      permission_type_stream, sora_word);

  if (!pstm->prop.text)
    Pl_Err_Permission(permission_operation_reposition,
		      permission_type_binary_stream, sora_word);


  line_count = Rd_Integer_Check(line_word) - 1;
  line_pos = Rd_Integer_Check(col_word) - 1;

  return line_count >= 0 && line_pos >= 0 &&
    Stream_Set_Position_LC(pstm, line_count, line_pos) == 0;
}




/*-------------------------------------------------------------------------*
 * Operations on term_streams                                              *
 *                                                                         *
 *-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*
 * OPEN_INPUT_TERM_STREAM_2                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Open_Input_Term_Stream_2(WamWord sink_term_word, WamWord stm_word)
{
  char *str;
  int stm;
  int n;

  if (SYS_VAR_OPTION_MASK == TERM_STREAM_ATOM)
    str = atom_tbl[Rd_Atom_Check(sink_term_word)].name;
  else
    {
      n = List_Length(sink_term_word);	/* -1 if not a list */
      if (n >= 0)
	str = Malloc(n + 1);	/* +1 for \0 */
      else
	str = glob_buff;

      if (SYS_VAR_OPTION_MASK == TERM_STREAM_CHARS)
	Rd_Chars_Str_Check(sink_term_word, str);
      else
	Rd_Codes_Str_Check(sink_term_word, str);
    }

  stm = Add_Str_Stream(FALSE, str);
  stm_tbl[stm].prop.other = SYS_VAR_OPTION_MASK + 1;

  Get_Integer(stm, stm_word);
}




/*-------------------------------------------------------------------------*
 * CLOSE_INPUT_TERM_STREAM_1                                               *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Close_Input_Term_Stream_1(WamWord sora_word)
{
  int stm;
  StmInf *pstm;
  StrSInf *str_stream;
  int type;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  type = pstm->prop.other - 1;

  if (type < 0 || type > 2)
    Pl_Err_Domain(domain_term_stream_or_alias, sora_word);

  if (pstm->prop.output)
    Pl_Err_Permission(permission_operation_close,
		      permission_type_stream, sora_word);

  if (type != TERM_STREAM_ATOM)
    {
      str_stream = (StrSInf *) (pstm->file);
      Free(str_stream->buff);
    }

  Delete_Str_Stream(stm);
}




/*-------------------------------------------------------------------------*
 * OPEN_OUTPUT_TERM_STREAM_1                                               *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Open_Output_Term_Stream_1(WamWord stm_word)
{
  int stm;

  stm = Add_Str_Stream(FALSE, NULL);
  stm_tbl[stm].prop.other = SYS_VAR_OPTION_MASK + 1;

  Get_Integer(stm, stm_word);
}




/*-------------------------------------------------------------------------*
 * CLOSE_OUTPUT_TERM_STREAM_2                                              *
 *                                                                         *
 *-------------------------------------------------------------------------*/
Bool
Close_Output_Term_Stream_2(WamWord sora_word, WamWord sink_term_word)
{
  int stm;
  StmInf *pstm;
  int type;
  char *str;

  stm = Get_Stream_Or_Alias(sora_word, STREAM_CHECK_EXIST);
  pstm = stm_tbl + stm;

  type = pstm->prop.other - 1;

  if (type < 0 || type > 2)
    Pl_Err_Domain(domain_term_stream_or_alias, sora_word);

  if (pstm->prop.input)
    Pl_Err_Permission(permission_operation_close,
		      permission_type_stream, sora_word);

  str = Term_Write_Str_Stream(stm);

  switch (SYS_VAR_OPTION_MASK)
    {
    case TERM_STREAM_ATOM:
      if (!Un_String_Check(str, sink_term_word))
	return FALSE;
      break;

    case TERM_STREAM_CHARS:
      if (!Un_Chars_Check(str, sink_term_word))
	return FALSE;
      break;

    case TERM_STREAM_CODES:
      if (!Un_Codes_Check(str, sink_term_word))
	return FALSE;
      break;

    }

  Delete_Str_Stream(stm);
  return TRUE;
}
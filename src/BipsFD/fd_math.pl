/*-------------------------------------------------------------------------*/
/* GNU Prolog                                                              */
/*                                                                         */
/* Part  : FD constraint solver buit-in predicates                         */
/* File  : fd_math.pl                                                      */
/* Descr.: mathematical predicate management                               */
/* Author: Daniel Diaz                                                     */
/*                                                                         */
/* Copyright (C) 1999,2000 Daniel Diaz                                     */
/*                                                                         */
/* GNU Prolog is free software; you can redistribute it and/or modify it   */
/* under the terms of the GNU General Public License as published by the   */
/* Free Software Foundation; either version 2, or any later version.       */
/*                                                                         */
/* GNU Prolog is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY WARRANTY; without even the implied warranty of              */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        */
/* General Public License for more details.                                */
/*                                                                         */
/* You should have received a copy of the GNU General Public License along */
/* with this program; if not, write to the Free Software Foundation, Inc.  */
/* 59 Temple Place - Suite 330, Boston, MA 02111, USA.                     */
/*-------------------------------------------------------------------------*/

:- built_in_fd.

'$use_fd_math'.


LE #= RE:-
        set_bip_name(#=,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(0)),
	'$call_c_test'('Fd_Eq_2'(LE,RE)).

LE #\= RE:-
        set_bip_name(#\=,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(0)),
	'$call_c_test'('Fd_Neq_2'(LE,RE)).

LE #< RE:-
        set_bip_name(#<,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(0)),
	'$call_c_test'('Fd_Lt_2'(LE,RE)).

LE #=< RE:-
        set_bip_name(#=<,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(0)),
	'$call_c_test'('Fd_Lte_2'(LE,RE)).

LE #> RE:-
        set_bip_name(#>,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(0)),
	'$call_c_test'('Fd_Lt_2'(RE,LE)).

LE #>= RE:-
        set_bip_name(#>=,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(0)),
	'$call_c_test'('Fd_Lte_2'(RE,LE)).




LE #=# RE:-
        set_bip_name(#=,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(1)),
	'$call_c_test'('Fd_Eq_2'(LE,RE)).

LE #\=# RE:-
        set_bip_name(#\=,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(1)),
	'$call_c_test'('Fd_Neq_2'(LE,RE)).

LE #<# RE:-
        set_bip_name(#<,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(1)),
	'$call_c_test'('Fd_Lt_2'(LE,RE)).

LE #=<# RE:-
        set_bip_name(#=<,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(1)),
	'$call_c_test'('Fd_Lte_2'(LE,RE)).

LE #># RE:-
        set_bip_name(#>,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(1)),
	'$call_c_test'('Fd_Lt_2'(RE,LE)).

LE #>=# RE:-
        set_bip_name(#>=,2),
        '$call_c'('Fd_Set_Full_Ac_Flag_1'(1)),
	'$call_c_test'('Fd_Lte_2'(RE,LE)).


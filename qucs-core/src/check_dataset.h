/*
 * check_dataset.h - checker definitions for the Qucs dataset
 *
 * Copyright (C) 2003, 2005, 2006 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.  
 *
 * $Id: check_dataset.h,v 1.5 2006/02/06 09:50:15 raimi Exp $
 *
 */

#ifndef __CHECK_DATASET_H__
#define __CHECK_DATASET_H__

extern dataset * dataset_result;
extern vector  * dataset_vector;
extern strlist * dataset_idents;

/* Externalize variables used by the scanner and parser. */
extern int dataset_lineno;
extern FILE * dataset_in;
void dataset_restart (FILE *);

__BEGIN_DECLS

/* Available functions of the checker. */
int dataset_parse (void);
int dataset_error (char *);
int dataset_lex (void);
int dataset_lex_destroy (void);
int dataset_check (dataset *);

__END_DECLS

#endif /* __CHECK_DATASET_H__ */

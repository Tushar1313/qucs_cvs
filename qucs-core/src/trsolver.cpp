/*
 * trsolver.cpp - transient solver class implementation
 *
 * Copyright (C) 2004 Stefan Jahn <stefan@lkcc.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 *
 * $Id: trsolver.cpp,v 1.2 2004/09/11 20:39:29 ela Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "object.h"
#include "complex.h"
#include "circuit.h"
#include "sweep.h"
#include "net.h"
#include "analysis.h"
#include "nasolver.h"
#include "trsolver.h"
#include "transient.h"

// Constructor creates an unnamed instance of the trsolver class.
trsolver::trsolver () : nasolver<nr_double_t> () {
  swp = NULL;
  type = ANALYSIS_TRANSIENT;
  setDescription ("transient");
}

// Constructor creates a named instance of the trsolver class.
trsolver::trsolver (char * n) : nasolver<nr_double_t> (n) {
  swp = NULL;
  type = ANALYSIS_TRANSIENT;
  setDescription ("transient");
}

// Destructor deletes the trsolver class object.
trsolver::~trsolver () {
  if (swp) delete swp;
}

/* The copy constructor creates a new instance of the trsolver class
   based on the given trsolver object. */
trsolver::trsolver (trsolver & o) : nasolver<nr_double_t> (o) {
  swp = o.swp ? new sweep (*o.swp) : NULL;
}

/* This is the transient netlist solver.  It prepares the circuit list
   for each requested time and solves it then. */
void trsolver::solve (void) {
  nr_double_t time, current;
  runs++;

  // create time sweep if necessary
  if (swp == NULL) {
    char * type = getPropertyString ("Type");
    nr_double_t start = getPropertyDouble ("Start");
    nr_double_t stop = getPropertyDouble ("Stop");
    int points = getPropertyInteger ("Points");

    if (!strcmp (type, "lin")) {
      swp = new linsweep ("time");
      ((linsweep *) swp)->create (start, stop, points);
    }
    else if (!strcmp (type, "log")) {
      swp = new logsweep ("time");
      ((logsweep *) swp)->create (start, stop, points);
    }
  }

  // initialize node voltages, first guess for non-linear circuits and
  // generate extra circuits if necessary
  init ();
  solve_pre ();

  current = 0;
  swp->reset ();
  for (int i = 0; i < swp->getSize (); i++) {
    time = swp->next ();
    logprogressbar (i, swp->getSize (), 40);

#if DEBUG && 0
    logprint (LOG_STATUS, "NOTIFY: %s: solving netlist for t = %e\n",
	      getName (), (double) time);
#endif

    do {
      calc (current);
      current += delta;
      // start the linear solver
      solve_linear ();
    }
    while (current < time);
    
    // save results
    saveAllResults (time);
  }
  solve_post ();
  logprogressclear (40);
}

/* Goes through the list of circuit objects and runs its calcTR()
   function. */
void trsolver::calc (nr_double_t time) {
  circuit * root = subnet->getRoot ();
  for (circuit * c = root; c != NULL; c = (circuit *) c->getNext ()) {
    c->nextState ();
    c->calcTR (time);
  }
}

/* Goes through the list of circuit objects and runs its initTR()
   function. */
void trsolver::init (void) {
  char * IModel = getPropertyString ("IntegrationMethod");
  int order = getPropertyInteger ("Order");
  delta = getPropertyDouble ("InitialStep");
  calcCoefficients (IModel, order, coefficients, delta);
  circuit * root = subnet->getRoot ();
  for (circuit * c = root; c != NULL; c = (circuit *) c->getNext ()) {
    c->initTR (this);
    c->initStates ();
    c->setCoefficients (coefficients);
    c->setOrder (order);
    setIntegrationMethod (c, IModel);
  }
}

/* This function saves the results of a single solve() functionality
   (for the given timestamp) into the output dataset. */
void trsolver::saveAllResults (nr_double_t time) {
  vector * t;
  // add current frequency to the dependency of the output dataset
  if ((t = data->findDependency ("time")) == NULL) {
    t = new vector ("time");
    data->addDependency (t);
  }
  if (runs == 1) t->add (time);
  saveResults ("Vt", "It", 0, t);
}
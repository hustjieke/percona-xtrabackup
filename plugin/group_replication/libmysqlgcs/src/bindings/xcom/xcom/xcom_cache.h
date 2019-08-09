/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef XCOM_CACHE_H
#define XCOM_CACHE_H

#include <stddef.h>

#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/xcom/simset.h"
#include "plugin/group_replication/libmysqlgcs/xdr_gen/xcom_vp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
We require that the number of elements in the cache is big enough enough that
it is always possible to find instances that are not busy.
Under normal circumstances the number of busy instances will be
less than event_horizon, since the proposers only considers
instances which belong to the local node.
A node may start proposing no_ops for instances belonging
to other nodes, meaning that event_horizon * NSERVERS instances may be
involved. However, for the time being, proposing a no_op for an instance
will not mark it as busy. This may change in the future, so a safe upper
limit on the number of nodes marked as busy is event_horizon * NSERVERS.
*/
#define MIN_LENGTH 250000     // Also Default value
#define INCREMENT MIN_LENGTH  // Total number of slots to add/remove

#define is_cached(x) (hash_get(x) != NULL)

struct lru_machine;
typedef struct lru_machine lru_machine;

struct stack_machine;
typedef struct stack_machine stack_machine;

struct pax_machine;
typedef struct pax_machine pax_machine;

/* Definition of a Paxos instance */
struct pax_machine {
  linkage hash_link;
  stack_machine *stack_link;
  lru_machine *lru;
  synode_no synode;
  double last_modified; /* Start time */
  linkage rv; /* Tasks may sleep on this until something interesting happens */

  struct {
    ballot bal;            /* The current ballot we are working on */
    bit_set *prep_nodeset; /* Nodes which have answered my prepare */
    ballot sent_prop;
    bit_set *prop_nodeset; /* Nodes which have answered my propose */
    pax_msg *msg;          /* The value we are trying to push */
    ballot sent_learn;
  } proposer;

  struct {
    ballot promise; /* Promise to not accept any proposals less than this */
    pax_msg *msg;   /* The value we have accepted */
  } acceptor;

  struct {
    pax_msg *msg; /* The value we have learned */
  } learner;
  int lock; /* Busy ? */
  pax_op op;
  int force_delivery;
  int enforcer;

#ifndef XCOM_STANDALONE
  char is_instrumented;
#endif
};

pax_machine *init_pax_machine(pax_machine *p, lru_machine *lru,
                              synode_no synode);
int is_busy_machine(pax_machine *p);
int lock_pax_machine(pax_machine *p);
pax_machine *get_cache_no_touch(synode_no synode, bool_t force);
pax_machine *get_cache(synode_no synode);
pax_machine *force_get_cache(synode_no synode);
pax_machine *hash_get(synode_no synode);
char *dbg_machine_nodeset(pax_machine *p, u_int nodes);
char *dbg_pax_machine(pax_machine *p);
void init_cache();
void deinit_cache();
void unlock_pax_machine(pax_machine *p);
void xcom_cache_var_init();
size_t shrink_cache();
size_t pax_machine_size(pax_machine const *p);
synode_no cache_get_last_removed();

void init_cache_size();
size_t add_cache_size(pax_machine *p);
size_t sub_cache_size(pax_machine *p);
int above_cache_limit();
size_t set_max_cache_size(uint64_t x);
int was_removed_from_cache(synode_no x);
uint16_t check_decrease();
void do_cache_maintenance();

// Unit testing
#define DEC_THRESHOLD_LENGTH 500000  // MIN_LENGTH * 10
#define MIN_TARGET_OCCUPATION 0.7
#define DEC_THRESHOLD_SIZE 0.95
#define MIN_LENGTH_THRESHOLD 0.9

uint64_t get_xcom_cache_occupation();
uint64_t get_xcom_cache_length();
uint64_t get_xcom_cache_size();
void set_length_increment(size_t increment);
void set_size_decrement(size_t decrement);
void set_dec_threshold_length(uint64_t threshold);
void set_min_target_occupation(float threshold);
void set_dec_threshold_size(float threshold);
void set_min_length_threshold(float threshold);

#ifndef XCOM_STANDALONE
void psi_set_cache_resetting(int is_resetting);
void psi_report_cache_shutdown();
void psi_report_mem_free(size_t size, int is_instrumented);
int psi_report_mem_alloc(size_t size);
#else
#define psi_set_cache_resetting(x) \
  do {                             \
  } while (0)
#define psi_report_cache_shutdown(x) \
  do {                               \
  } while (0)
#define psi_report_mem_free(x) \
  do {                         \
  } while (0)
#define psi_report_mem_alloc(x) \
  do {                          \
  } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif

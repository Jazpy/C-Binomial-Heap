#ifndef BIN_H_H_INCLUDED
#define BIN_H_H_INCLUDED

#include <stddef.h>
#include "task_struct.h"

/**************
* STRUCT DEFS *
**************/

struct linked_node
{
    struct bin_node *curr;
    struct linked_node *next;
};

struct bin_node
{
    struct task_struct *process;
    struct bin_node *parent;
    struct linked_l *children;

    size_t rank;
};

struct linked_l
{
    struct linked_node *head;
    struct linked_node *tail;

    size_t size;
};

struct bin_h
{
    struct linked_l *roots;
    struct bin_node *min;
};

/******************
* END STRUCT DEFS *
******************/

/********************
* FUNC DECLARATIONS *
********************/

void init_bin_h(struct bin_h *bh);
void insert(struct bin_h *bh, struct task_struct *t);
size_t get_min(struct bin_h *bh);
void delete_min(struct bin_h *bh);
void free_bin_h(struct bin_h *bh);
void decrease_key(struct task_struct *t, size_t new_p);
void delete_key(struct bin_h *bh, struct task_struct *t);

/************************
* END FUNC DECLARATIONS *
************************/

#endif // BIN_H_H_INCLUDED

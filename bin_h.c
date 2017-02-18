#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include "bin_h.h"
#include "task_struct.h"

void free_linked_l(struct linked_l *l);
void free_bin_node(struct bin_node *n);
void free_linked_node(struct linked_node *n);

/********
* INITS *
********/

void init_linked_l(struct linked_l *l)
{
    l -> head = NULL;
    l -> tail = NULL;

    l -> size = 0;
}

void init_bin_node(struct bin_node *n, struct task_struct *t)
{
    n -> children = malloc(sizeof(struct linked_l));
    init_linked_l(n -> children);

    n -> parent = NULL;

    n -> process = t;

    n -> rank = 0;
}

void init_bin_h_l(struct bin_h *bh, struct linked_l *l)
{
    bh -> roots = l;

    if(l -> head == NULL)
        return;

    bh -> min = l -> head -> curr;

    struct linked_node *iter = l -> head -> next;
    while(iter != NULL)
    {
        if(iter -> curr -> process -> priority < bh -> min -> process -> priority)
            bh -> min = iter -> curr;

        iter = iter -> next;
    }
}

/************
* END INITS *
************/

/*********
* FREERS *
*********/

void free_bin_node(struct bin_node *n)
{
    free_linked_l(n -> children);
    free(n);
}

void free_linked_node(struct linked_node *n)
{
    free_bin_node(n -> curr);
    free(n);
}

void free_linked_l(struct linked_l *l)
{
    while(l -> head != NULL)
    {
        struct linked_node *temp = l -> head;
        l -> head = l -> head -> next;

        free_linked_node(temp);
    }

    free(l);
}

/*************
* END FREERS *
*************/

/**************
* AUXILIARIES *
**************/

void print_bh(struct bin_h *bh)
{
    printf("*************\n");

    printf("size: %d\n", bh -> roots -> size);

    if(bh -> min)
        printf("minimum: %d\n", bh -> min -> process -> priority);

    printf("roots: ");

    struct linked_node *iter = bh -> roots -> head;
    while(iter != NULL)
    {
        printf("%d, ", iter -> curr -> process -> priority);
        iter = iter -> next;
    }

    printf("\n--------ROOTS-------\n");

    iter = bh -> roots -> head;
    while(iter != NULL)
    {
        printf("rank of %d: %d\n", iter -> curr -> process -> priority, iter -> curr -> rank);
        printf("roots for %d: ", iter -> curr -> process -> priority);

        struct linked_node *inner_iter = iter -> curr -> children -> head;
        while(inner_iter != NULL)
        {
            printf("%d, ", inner_iter -> curr -> process -> priority);
            inner_iter = inner_iter -> next;
        }
        printf("\n");
        iter = iter -> next;
    }

    printf("\n*************\n");
}

void remove_linked_l(struct linked_l *l, struct bin_node *n)
{
    // Special case
    if(l -> head -> curr == n)
    {
        struct linked_node *temp = l -> head;

        l -> head = l -> head -> next;
        --l -> size;

        free(temp);

        return;
    }

    struct linked_node *prev = l -> head;
    struct linked_node *iter = l -> head -> next;

    while(iter != NULL)
    {
        if(iter -> curr == n)
        {
            prev -> next = iter -> next;
            --l -> size;
            free(iter);

            return;
        }

        prev = iter;
        iter = iter -> next;
    }
}

void insert_linked_l(struct linked_l *l, struct bin_node *n)
{
    struct linked_node *node = malloc(sizeof(struct linked_node));
    node -> curr = n;
    node -> next = NULL;

    // Check for empty list
    if(l -> size == 0)
    {
        l -> head = node;
        l -> tail = node;

        ++l -> size;

        return;
    }

    struct linked_node *prev = l -> head;
    struct linked_node *iter = l -> head;

    while(iter != NULL)
    {
        if(n -> rank < iter -> curr -> rank)
            break;

        prev = iter;
        iter = iter -> next;
    }

    if(iter == l -> head)
    {
        node -> next = l -> head;
        l -> head = node;
    } else {
        prev -> next = node;
        node -> next = iter;
    }

    ++l -> size;
}

int merge_tree(struct bin_node *p, struct bin_node *q)
{
    if(p -> process -> priority < q -> process -> priority)
    {
        insert_linked_l(p -> children, q);
        ++p -> rank;
        q -> parent = p;

        return 1;
    } else {
        insert_linked_l(q -> children, p);
        ++q -> rank;
        p -> parent = q;

        return 0;
    }
}

void merge(struct bin_h *p, struct bin_h *q)
{
    if(p -> roots -> size == 0 && q -> roots -> size == 0)
        return;

    // First, union of all roots (in order)
    struct linked_node *q_roots_iter = q -> roots -> head;

    while(q_roots_iter != NULL)
    {
        insert_linked_l(p -> roots, q_roots_iter -> curr);
        q_roots_iter = q_roots_iter -> next;
    }

    // Now merge trees
    struct linked_node *x = p -> roots -> head;
    struct linked_node *next = x -> next;

    while(x != NULL && next != NULL)
    {
        if(x -> curr -> rank != next -> curr-> rank)
        {
            x = x -> next;
            next = x -> next;
        } else if(next -> next && next -> next -> curr -> process -> priority == x -> curr -> process -> priority) {
            x = x -> next;
            next = x -> next;
        } else if(x -> curr -> process -> priority <= next -> curr -> process -> priority) {
            merge_tree(x -> curr, next -> curr);
            struct linked_node *temp = next;
            next = next -> next;
            remove_linked_l(p -> roots, temp -> curr);
        } else {
            merge_tree(x -> curr, next -> curr);
            struct linked_node *temp = x;
            x = next;
            next = x -> next;
            remove_linked_l(p -> roots, temp -> curr);
        }
    }
}

/******************
* END AUXILIARIES *
******************/

/***********************
* FUNC IMPLEMENTATIONS *
***********************/

void init_bin_h(struct bin_h *bh)
{
    bh -> roots = malloc(sizeof(struct linked_l));
    init_linked_l(bh -> roots);
    bh -> min = NULL;
}

void insert(struct bin_h *bh, struct task_struct *t)
{
    struct bin_node *node = malloc(sizeof(struct bin_node));
    init_bin_node(node, t);

    // Special case for empty bin_h
    if(bh -> roots -> size == 0)
    {
        insert_linked_l(bh -> roots, node);
        bh -> min = node;

        t -> ticket = node;

        return;
    }

    // Otherwise, first get a rank 0 tree
    struct bin_h *new_h = malloc(sizeof(struct bin_h));
    init_bin_h(new_h);
    insert(new_h, t);

    // And merge
    merge(bh, new_h);

    // Check for new min
    struct linked_node *iter = bh -> roots -> head;
    while(iter != NULL)
    {
        if(iter -> curr -> process -> priority < bh -> min -> process -> priority)
            bh -> min = iter -> curr;

        iter = iter -> next;
    }
}

size_t get_min(struct bin_h *bh)
{
    return bh -> min -> process -> priority;
}

void delete_min(struct bin_h *bh)
{
    struct linked_l *new_roots = bh -> min -> children;
    remove_linked_l(bh -> roots, bh -> min);

    struct bin_h *temp = malloc(sizeof(struct bin_h));
    init_bin_h_l(temp, new_roots);

    merge(bh, temp);

    // Check for new min
    free(bh -> min);
    bh -> min = NULL;

    struct linked_node *iter = bh -> roots -> head;
    while(iter != NULL)
    {
        if(bh -> min == NULL || iter -> curr -> process -> priority < bh -> min -> process -> priority)
            bh -> min = iter -> curr;

        iter = iter -> next;
    }
}

void decrease_key(struct task_struct *t, size_t new_p)
{
    t -> priority = new_p;

    while(t -> ticket -> parent != NULL && t -> ticket -> parent -> process -> priority >= t -> priority)
    {
        struct task_struct *temp_st = t;
        struct bin_node *temp_nt = t -> ticket;

        struct task_struct *temp_sp = t -> ticket -> parent -> process;
        struct bin_node *temp_np = t -> ticket -> parent;

        t -> ticket = temp_np;
        temp_np -> process = t;

        temp_sp -> ticket = temp_nt;
        temp_nt -> process = temp_sp;
    }
}

void delete_key(struct bin_h *bh, struct task_struct *t)
{
    decrease_key(t, 0);
    delete_min(bh);
}

void free_bin_h(struct bin_h *bh)
{
    free_linked_l(bh -> roots);
    free(bh);
}

/***************************
* END FUNC IMPLEMENTATIONS *
****************************/

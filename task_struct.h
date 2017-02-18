#ifndef TASK_STRUCT_H_INCLUDED
#define TASK_STRUCT_H_INCLUDED

#include <stddef.h>
#include "bin_h.h"

struct task_struct
{
    size_t priority;

    struct bin_node *ticket;
};

#endif // TASK_STRUCT_H_INCLUDED

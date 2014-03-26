/*
   Copyright (c) 2014 Red Hat, Inc. <http://www.redhat.com>
   This file is part of GlusterFS.

   This file is licensed to you under your choice of the GNU Lesser
   General Public License, version 3 or any later version (LGPLv3 or
   later), or the GNU General Public License, version 2 (GPLv2), in all
   cases as published by the Free Software Foundation.
*/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include "xlator.h"
#include "defaults.h"

#include "meta-mem-types.h"
#include "meta.h"
#include "strfd.h"
#include "globals.h"
#include "lkowner.h"


static int
frames_file_fill (xlator_t *this, inode_t *file, strfd_t *strfd)
{
	struct call_pool *pool = NULL;
	call_stack_t *stack = NULL;
	call_frame_t *frame = NULL;
	int i = 0;
	int j = 0;

	pool = this->ctx->pool;

	LOCK (&pool->lock);
	{
		strprintf (strfd, "Call_Count: %d\n", (int)pool->cnt);

		list_for_each_entry (stack, &pool->all_frames, all_frames) {
			strprintf (strfd, "== Stack %d ==\n", i++);
			strprintf (strfd, "Unique: %"PRId64"\n", stack->unique);
			strprintf (strfd, "Type: %s\n", gf_fop_list[stack->op]);
			strprintf (strfd, "UID: %d\n", stack->uid);
			strprintf (strfd, "GID: %d\n", stack->gid);
			strprintf (strfd, "LK_owner: %s\n",
				   lkowner_utoa (&stack->lk_owner));

			j = 0;
			for (frame = &stack->frames; frame; frame = frame->next) {
				strprintf (strfd, "\t-- Frame %d --\n", j++);
				strprintf (strfd, "\tXlator: %s\n", frame->this->name);
				if (frame->begin.tv_sec)
					strprintf (strfd, "\tCreation_time: %d.%d\n",
						   (int)frame->begin.tv_sec,
						   (int)frame->begin.tv_usec);
				strprintf (strfd, "\tRefcount: %d\n", frame->ref_count);
				strprintf (strfd, "\tComplete: %d\n", frame->complete);
				if (frame->parent)
					strprintf (strfd, "\tParent: %s\n",
						   frame->parent->this->name);
				if (frame->wind_from)
					strprintf (strfd, "\tWind_from: %s\n",
						   frame->wind_from);
				if (frame->wind_to)
					strprintf (strfd, "\tWind_to: %s\n",
						   frame->wind_to);
				if (frame->unwind_from)
					strprintf (strfd, "\tUnwind_from: %s\n",
						   frame->unwind_from);
				if (frame->unwind_to)
					strprintf (strfd, "\tUnwind_to: %s\n",
						   frame->unwind_to);
			}
		}
	}
	UNLOCK (&pool->lock);

	return strfd->size;
}


static struct meta_ops frames_file_ops = {
	.file_fill = frames_file_fill,
};


int
meta_frames_file_hook (call_frame_t *frame, xlator_t *this, loc_t *loc,
		       dict_t *xdata)
{
	meta_ops_set (loc->inode, this, &frames_file_ops);

	return 0;
}

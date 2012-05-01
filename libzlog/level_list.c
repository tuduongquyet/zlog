/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * The zlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The zlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the zlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "syslog.h"

#include "zc_defs.h"
#include "level.h"
#include "level_list.h"

/* zlog_level_list == zc_arraylist_t<zlog_level_t> */

void zlog_level_list_profile(zc_arraylist_t *levels, int flag)
{
	int i;
	zlog_level_t *a_level;

	zc_assert(levels,);
	zc_profile(flag, "--level_list[%p]--", levels);
	zc_arraylist_foreach(levels, i, a_level) {
		zc_level_profile(a_level, flag);
	}
	return;
}

/*******************************************************************************/
void zlog_level_list_del(zc_arraylist_t *levels)
{
	zc_assert(levels,);
	zc_arraylist_del(levels);
	zc_debug("zc_levels_del[%p]", levels);
	return;
}

static int zlog_level_list_set_default(zc_arraylist_t *levels)
{
	return zlog_level_list_set(levels, "* = 0, LOG_INFO")
	|| zlog_level_list_set(levels, "DEBUG = 20, LOG_DEBUG")
	|| zlog_level_list_set(levels, "INFO = 40, LOG_INFO")
	|| zlog_level_list_set(levels, "NOTICE = 60, LOG_NOTICE")
	|| zlog_level_list_set(levels, "WARN = 80, LOG_WARNING")
	|| zlog_level_list_set(levels, "ERROR = 100, LOG_ERR")
	|| zlog_level_list_set(levels, "FATAL = 120, LOG_ALERT")
	|| zlog_level_list_set(levels, "UNKNOWN = 254, LOG_ERR")
	|| zlog_level_list_set(levels, "! = 255, LOG_INFO");
}

zc_arraylist_t *zlog_levels_new(void)
{
	int rc;
	zc_arraylist_t *levels;

	levels = zc_arraylist_new((zc_arraylist_del_fn)zlog_level_del);
	if (!levels) {
		zc_error("zc_arraylist_new fail");
		return NULL;
	}

	rc = zlog_levels_set_default(levels);
	if (rc) {
		zc_error("zlog_level_set_default fail");
		rc = -1;
		goto zlog_level_init_exit;
	}

      zlog_level_init_exit:
	if (rc) {
		zc_arraylist_del(levels);
		return NULL;
	} else {
		zlog_levels_profile(levels, ZC_DEBUG);
		return levels;
	}
}

/*******************************************************************************/
int zlog_level_list_set(zc_arraylist_t *levels, char *line)
{
	int rc;
	zlog_level_t *a_level;

	a_level = zlog_level_new(line);
	if (!a_level) {
		zc_error("zlog_level_new fail");
		return -1;
	}

	rc = zc_arraylist_set(levels, a_level->int_level, a_level);
	if (rc) {
		zc_error("zc_arraylist_set fail");
		rc = -1;
		goto zlog_level_set_exit;
	}

      zlog_level_set_exit:
	if (rc) {
		zc_error("line[%s]", line);
		zlog_level_del(a_level);
		return -1;
	} else {
		return 0;
	}
}

zlog_level_t *zlog_level_list_get(zc_arraylist_t *levels, int l)
{
	zlog_level_t *a_level;

	if ((l <= 0) || (l > 254)) {
		/* illegal input from zlog() */
		zc_error("l[%d] not in (0,254), set to UNKOWN", l);
		l = 254;
	}

	a_level = zc_arraylist_get(levels, l);
	if (!a_level) {
		/* empty slot */
		zc_error("l[%d] in (0,254), but has no level defined,"
			"see configure file define, set ot UNKOWN", l);
		a_level = zc_arraylist_get(levels, 254);
	}

	return a_level;
}

/*******************************************************************************/

int zlog_level_list_atoi(zc_arraylist_t *levels, char *str)
{
	int i;
	zlog_level_t *a_level;

	if (str == NULL || *str == '\0') {
		zc_error("str is [%s], can't find level", str);
		return -1;
	}

	zc_arraylist_foreach(levels, i, a_level) {
		if (a_level && STRICMP(str, ==, a_level->str_uppercase)) {
			return i;
		}
	}

	zc_error("str[%s] can't found in level list", str);
	return -1;
}

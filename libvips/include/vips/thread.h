/* Private include file ... if we've been configured without gthread, we need
 * to point the g_thread_*() and g_mutex_*() functions at our own stubs.
 */

/*

	This file is part of VIPS.

	VIPS is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
	02110-1301  USA

 */

/*

	These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

#ifndef VIPS_THREAD_H
#define VIPS_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

VIPS_API
GThread *vips_g_thread_new(const char *domain, GThreadFunc func, gpointer data);

VIPS_API
gboolean vips_thread_isvips(void);

VIPS_API
int vips_thread_execute(const char *domain, GFunc func, gpointer data);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*VIPS_THREAD_H*/

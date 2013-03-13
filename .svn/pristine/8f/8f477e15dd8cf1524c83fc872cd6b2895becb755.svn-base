/*
 *  Copyright (C) 2004  Samuel Thibault <samuel.thibault@ens-lyon.org>
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the program ; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

void record_time( int ncpus, double mhz[] )
	{
	int			i;

	BEGIN_BLOCK(fd);
	if( write(fd, (void *)&ncpus, sizeof(ncpus)) < 0 )
		perror("write ncpus");
	for( i = 0;  i < ncpus;  i++ )
		{
		if( write(fd, (void *)&mhz[i], sizeof(mhz[i])) < 0 )
			perror("write mhz");
		}
	if( write(fd, (void *)&pid, sizeof(pid)) < 0 )
		perror("write pid");
	if( write(fd, (void *)&kpid, sizeof(kpid)) < 0 )
		perror("write kpid");
	if( write(fd, (void *)&start_time, sizeof(start_time)) < 0 )
		perror("write start_time");
	if( write(fd, (void *)&stop_time, sizeof(stop_time)) < 0 )
		perror("write stop_time");
	if( write(fd, (void *)&start_jiffies, sizeof(start_jiffies)) < 0 )
		perror("write start_jiffies");
	if( write(fd, (void *)&stop_jiffies, sizeof(stop_jiffies)) < 0 )
		perror("write stop_jiffies");
	if( write(fd, (void *)&page_size, sizeof(page_size)) < 0 )
		perror("write page size");
	END_BLOCK(fd);

	}


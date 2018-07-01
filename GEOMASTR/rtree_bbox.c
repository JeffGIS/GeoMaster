/*

 rtree_bbox.c -- querying the BBOX of an R*Tree

 Author: Sandro Furieri a.furieri@lqt.it
 
--------------------------------------------------------

The author has placed this work in the Public Domain, 
thereby relinquishing all copyrights. 
Everyone is free to use, modify, republish, sell or give 
away this work without prior consent from anybody.

*/

#include <stdio.h>
#include <sqlite3.h>

struct bbox_rtree
{
	int valid;
	double minx;
	double maxx;
	double miny;
	double maxy;
};
typedef struct
{
	double  xmn;
	double  ymn;
	double  xmx;
	double  ymx;
} MNMXCORD;
typedef MNMXCORD    *LPMNMXCORD;
static int
rtree_bbox_callback (sqlite3_rtree_query_info * info)
{
/*

 R*Tree Query Callback function 

----------------------------------------------------------

 this function will evaluate all first-level R*Tree Nodes
 (direct children of the Root Node) so to get the Full
 Extent of the R*Tree as a whole.

 further descending in the Tree's hierarchy will be
 carefully avoided, so to ensure that also in the case  
 of an R*Tree containing many million entries just few
 dozens of top level Nodes will require to be evaluated.
 
*/
    double minx;
    double maxx;
    double miny;
    double maxy;
    struct bbox_rtree *data = (struct bbox_rtree *) (info->pContext);
    if (info->nCoord != 4)
	{
	/* invalid RTree; not 2D */
		goto end;
	}

/* fetching the Node's BBOX */
    minx = info->aCoord[0];
    maxx = info->aCoord[1];
    miny = info->aCoord[2];
    maxy = info->aCoord[3];
	
/* updating the Full Extent BBOX */
	if (data->valid == 0)
	{
	/* first Node retrieved */
		data->valid = 1;
		data->minx = minx;
		data->maxx = maxx;
		data->miny = miny;
		data->maxy = maxy;
	}
	else
	{
	/* any other further Node */
		if (minx < data->minx)
			data->minx = minx;
		if (maxx > data->maxx)
			data->maxx = maxx;
		if (miny < data->miny)
			data->miny = miny;
		if (maxy > data->maxy)
			data->maxy = maxy;
	}
	
end:
/* setting NOT_WITHIN so to stop further descending into the tree */
	info->eWithin = NOT_WITHIN;
    return SQLITE_OK;
}

int query_rtree_bbox(sqlite3 *db_handle, const char *rtree_name, LPMNMXCORD pBounds)
{
/* attempting to query the BBOX of the R*Tree */
	int ret;
	struct bbox_rtree data;
	
	data.valid = 0;
	
/* registering the Geometry Query Callback SQL function */
	sqlite3_rtree_query_callback (db_handle, "rtree_bbox", 
		rtree_bbox_callback, &data, NULL);
		
/* executing the SQL Query statement */
	char sql[256];
	sprintf(sql,"SELECT id FROM %s_index WHERE id MATCH rtree_bbox(1)",
	    rtree_name);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
	{
		return 0;
	}
	
	if (data.valid == 0)
		return 0;
	
	pBounds->xmn = data.minx;
	pBounds->xmx = data.maxx;
	pBounds->ymn = data.miny;
	pBounds->ymx = data.maxy;
	return 1;
}

/*int main(int argc, char *argv[])
{
	const char *db_path;
	const char *rtree_name;
	int ret;
	sqlite3 *db_handle;
	

	if (argc != 3)
	{
		fprintf(stderr, "usage: ./rtree_bbox db_file_path rtree_name\n");
		return -1;
	}
	db_path = argv[1];
	rtree_name = argv[2];
	
	ret = sqlite3_open_v2(db_path, &db_handle, SQLITE_OPEN_READONLY, NULL);
	if (ret != SQLITE_OK)
	{
		fprintf (stderr, "cannot open '%s': %s\n", 
			db_path, sqlite3_errmsg (db_handle));
		sqlite3_close (db_handle);
		return -1;
	}
	
	query_rtree_bbox(db_handle, rtree_name);
	

	sqlite3_close(db_handle);
	return 0;
}
*/
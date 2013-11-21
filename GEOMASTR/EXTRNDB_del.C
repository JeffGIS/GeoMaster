typedef	struct
	{	char	Name[256];
		int		Length;
		int		Type;
		int		StartPos;
		int		Index;      
	} FIELDINFO;
typedef FIELDINFO FAR  *LPFIELDINFO;


HANDLE OpenExternalDatabase (LPSTR Name)
/*	Opens a FoxPro, MS Access or Pardox database	*/
/*	Returns handle if successful or NULL if not		*/

int	NumDatabaseTables (HANDLE DBhandle)
/*	Returns the number of tables in the database	*/

LPSTR GetTableName (HANDLE DBhandle, BOOL First)
/*	Returns the name of a table in the database		*/
/*	If First is TRUE returns the first table.		*/
/*	Otherwise returns the next or NULL if at end	*/

HANDLE OpenDatabaseTable (HANDLE DBHandle, LPSTR TableName)
/*	Opens the specified table						*/


LPFIELDDATA GetFieldInfo (HANDLE TBLHandle, BOOL First)
/*	Returns a pointer to a FIELDINFO struct.		*/
/*	If First is TRUE returns the first table.		*/
/*	Otherwise returns the next or NULL if at end	*/  

LPVOID GetExternalFieldData (HANDLE TBLHandle, int index,
							 LPVOID KeyData, LPFIELDINFO Field)
/*	Returns data for the specified field for the	*/
/*	row with the specified key.	Returns NULL if		*/
/*	the record does not exist.						*/


void CloseExternalTable (HANDLE TBLHandle)

void CloseExternalDatabase (HANDLE DBHandle)
//#include "ogr_spatialref.h"
#include "ogr_srs_api.h"

int ConvertPRJtoProj4(char *instr, char * outstr)
{
	//OGRSpatialReference oSRS;
	char in[1024];
	char *pin[2];
	char *pout = NULL;
	OGRSpatialReferenceH hSRS;
	OGRErr err;

	hSRS = OSRNewSpatialReference(NULL);
	//err = OSRSetWellKnownGeogCS(hSRS,"WGS84");
	pin[0] = instr;
	pin[1] = NULL;
	err = OSRImportFromESRI(hSRS,pin);


	err = OSRExportToProj4(hSRS, &pout);
	strcpy(outstr, pout);
	//free(pout);
	OSRRelease(hSRS);

	return err;
}


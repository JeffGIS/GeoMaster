
                                                                                                                                                                      
void     HASHS  (int *id, long ITLN,long  KEYLN, long  NITM, char *FNAME,
                          long NLEN,char *STATUS);
void     HASH_DESC (int ID, long *ITLN, long *KEYLN, long *NITM); 
void     HASHP  (int ID, char *ITEM,  long  *LOC);
void     HASHF  (int ID, char *ITEM,  long  *LOC);
void     HASHG  (int ID, char *ITEM,const long  LOC);
void     HASHI  (int ID, char *ITEM,const  long  LOC);
void     HASHN  (int ID, long *NUMBER);
void     HASHC  (int ID);
void     HASHWR (int ID, FILE *NIT);
void     HASHE  (int ID);
void     HASHA  (int ID, LPSTR *IPNTR);
void     HASHRA (int ID, char *DATA,const long IGET,const long  LEN);
void     HASHPA (int ID, char *DATA,const long IPUT,const long  LEN);
void     HASHCA (void);
long     IHRANS (int ID,long  ILEN,long  IMAX);
long     IHRAN  (int ID,char  item[]);
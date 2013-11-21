/* $Id$ */
/* //////////////////////////////////////////////////////////////////////////
//                                                                         //
// This code is Copyright (c) 2008 LizardTech, Inc, 1008 Western Avenue,   //
// Suite 200, Seattle, WA 98104.  Unauthorized use or distribution         //
// prohibited.  Access to and use of this code is permitted only under     //
// license from LizardTech, Inc.  Portions of the code are protected by    //
// US and foreign patents and other filings. All Rights Reserved.          //
//                                                                         //
////////////////////////////////////////////////////////////////////////// */
/* PUBLIC */

#include "lt_platform.h"
#include "support.h"

#ifndef _WIN32
#include <unistd.h>
#endif

void
Remove(const char* file)
{
   unlink(file);
}


void
Assert(const char* file, int line, const char* str, int cond)
{
   if (cond) return;
   printf("*** assertion failed: %s  (%s:%d)\n",
          str, file, line);
   exit(1);
}


static int
readFile(const char* file, char*& buf)
{
   FILE* fp = fopen(file,"rb");
   if (!fp)
   {
      printf("*** file does not exist: %s\n", file);
      exit(1);
   }
   fseek(fp,0,SEEK_END);
   int len = ftell(fp);
   fseek(fp,0,SEEK_SET);
   buf = new char[len];
   int cnt = (int)fread(buf, 1, len, fp);
   ASSERT(cnt==len);
   fclose(fp);
   return len;
}


int
Compare(const char* file1,
        const char* file2)
{
   return (GetNumDiffs(file1, file2) == 0);
}


int
GetNumDiffs(const char *file1,
            const char *file2)
{
   char* buf1 = NULL;
   int len1 = readFile(file1, buf1);
   char* buf2 = NULL;
   int len2 = readFile(file2, buf2);
   int diffs = 0;
   for (int i = 0; i < len1 && i < len2; i++)
      if (buf1[i] != buf2[i])
         diffs++;
   if (len1 > len2)
      diffs += (len1 - len2);
   else if (len2 > len1)
      diffs += (len2 - len1);
   delete[] buf1;
   delete[] buf2;
   return diffs;
}


void
checkCwd()
{
   FILE* fp = fopen("README-examples.txt","rb");
   if (!fp)
   {
      printf("*** example must be run from ./examples directory\n");
      exit(1);
   }
   fclose(fp);
}

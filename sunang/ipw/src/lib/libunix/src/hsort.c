
/* hsort -- Heapsort (Floyd) replacement for qsort

   from Unix Review, August 1992
*/

#define FALLBACK_SORT qsort

void swapfunc(char *i, char *j, int n)
{ do {
    char c = *i;
    *i++ = *j;
    *j++ = c;
  } while (--n > 0);
}

#define swap(i, j) swapfunc(a+(i)*es, a+(j)*es, es)

#define copy(i, j) { if (copytype == 0) \
                       *(long *)(i) = *(long *)(j); \
                     else \
                       memcpy((i), (j), es); }

#define siftdown(l, u) { \
  pu = (u) * es; \
  copy(buf, a+(l)*es); \
  for (pi = (l)*es; (pc = 2*pi) <= pu; pi = pc) { \
    if (pc < pu && cmp(a+pc+es, a+pc) > 0) \
      pc += es; \
    copy(a+pi, a+pc); \
  } \
  copy(a+pi, buf); \
  for (i=pi/es; (p = i/2) >= (l); i = p) { \
    if (cmp(a+p*es, a+i*es) >= 0) \
      break; \
    swap(p,i); \
  } \
}

#define MYBUFSIZE 1024
void hsort(char *a, int n, int es, int (*cmp)()) {
  int j, i, c, p, u, copytype, pi, pu, pc;
  char *buf, mybuf[MYBUFSIZE], *malloc();

  buf = mybuf;
  if (es > MYBUFSIZE && !(buf = malloc(es))) {
    FALLBACK_SORT(a, n, es, cmp);
    return;
  }
  copytype = (a - (char *) 0) % sizeof(long)
             || es != sizeof(long);
  a -= es;
  for (j = n/2; j >= 1; j--)
    siftdown(j, n);
  for (j = n; j >= 2; j--) {
    swap(1,j);
    siftdown(1, j-1);
  }
  if (es > MYBUFSIZE)
    free(buf);
}

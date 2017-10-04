
void *memset (void *p, int c, int n) {
  char *pb = (char *) p;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return p;
}

void *memcpy(void *d, const void *s, int n) {
	char * dt = d;
	const char* st = s;
    while (n--) {
        *dt++ = *st++;
	}
	return d;
}
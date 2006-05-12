
char *XCEFixPathA(const char *pathin, char *pathout);

char *
fixpath(const char *pathin, char *pathout)
{
  return XCEFixPathA(pathin, pathout);
}

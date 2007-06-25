BEGIN {
}

{
  winerr = $1;
  errno = $2;

  if (errno != "" && substr (errno, 1, 1) != "#" && prev_winerr != winerr)
    {
      print "case " winerr ": return " errno ";";
      prev_winerr = winerr;
    }
}

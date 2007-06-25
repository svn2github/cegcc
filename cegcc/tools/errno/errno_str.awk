BEGIN {
}

{
  errno = $1;
  winerr = $2;
  # get tail beginning at $3
  msg=$0; for (i=1;i<3;i++) sub($i,"",msg); sub(/ */,"",msg)

  if (errno != "" && substr ($1, 1, 1) != "#" && msg != "")
    print "case " winerr ": return " msg ";";
}

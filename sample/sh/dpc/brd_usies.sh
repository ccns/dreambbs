
cat run/brd_usies | 
awk ' BEGIN {
#print " �\\Ū����    �ݪO�W��     " 
#print "---------------------------"
}

{
  Number[$1]++
}

END {
  for(course in Number)
     printf("%10d %12s\n", Number[course], course)
}' | sort -n -r > run/brd_usies.log

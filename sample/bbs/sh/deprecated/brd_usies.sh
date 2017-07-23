
cat run/brd_usies | 
awk ' BEGIN {
#print " 閱\讀次數    看板名稱     " 
#print "---------------------------"
}

{
  Number[$1]++
}

END {
  for(course in Number)
     printf("%10d %12s\n", Number[course], course)
}' | sort -n -r > run/brd_usies.log

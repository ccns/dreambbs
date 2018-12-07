related commands:

```
find . -name "*.c" -execdir sh -c "indent -st -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 {} > {}.new " \;
find . -name "*.c.new" -exec prename -f -v 's/\.new//' {} \;
find . -name "*.h" -execdir sh -c "indent -st -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 {} > {}.new " \;
find . -name "*.h.new" -exec prename -f -v 's/\.new//' {} \;
find . -name "*.h" -exec sed -i 's/\t/    /g' {} \;
find . -name "*.c" -exec sed -i 's/\t/    /g' {} \;
```
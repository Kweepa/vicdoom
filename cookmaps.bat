rem call with, eg, e1m1 as the first parameter
copy /y c:\users\steve\documents\doomedit\compiled\%1.s .
cl65 -t vic20 -C doom_map_cc65.cfg %1.s
del %1
ren cookedmap %1

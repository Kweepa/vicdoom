cl65 -t vic20 -C doom_cc65.cfg -O -l vicdoom.c menu.c automap.c p_enemy.c summary.c victory.c util.c cheatsAsm.s utilAsm.s enemy.s m_random.s doomlogo.s drawLine.s updateInput.s doomfont.s textures.s dpsounds.s playSound.s drawColumnAsm.s logMathAsm.s mapAsm.s -vm -m map.txt -o doom_unc.prg
rem \app\pucrunch\pucrunch -c20 doom_unc.prg doom.prg
\app\exomizer\win32\exomizer sfx basic -t52 -odoom.prg doom_unc.prg
@echo off
cc65mapsort map.txt sortedmap.txt labels.txt
mid2vic\debug\prepend_load_address e1m1 ae00
mid2vic\debug\prepend_load_address e1m2 ae00
mid2vic\debug\prepend_load_address e1m3 ae00
mid2vic\debug\prepend_load_address e1m4 ae00
mid2vic\debug\prepend_load_address e1m5 ae00
mid2vic\debug\prepend_load_address e1m6 ae00
mid2vic\debug\prepend_load_address e1m7 ae00
mid2vic\debug\prepend_load_address e1m8 ae00
mid2vic\debug\prepend_load_address e1m9 ae00
mid2vic\debug\prepend_load_address e1m1mus b750
mid2vic\debug\prepend_load_address e1m2mus b750
mid2vic\debug\prepend_load_address e1m3mus b750
mid2vic\debug\prepend_load_address e1m4mus b750
mid2vic\debug\prepend_load_address e1m5mus b750
mid2vic\debug\prepend_load_address e1m6mus b750
mid2vic\debug\prepend_load_address e1m7mus b750
mid2vic\debug\prepend_load_address e1m8mus b750
mid2vic\debug\prepend_load_address e1m9mus b750
mid2vic\debug\prepend_load_address intermus b750
mid2vic\debug\prepend_load_address victormus b750
mid2vic\debug\prepend_load_address sluts 400
mid2vic\debug\prepend_load_address lowcode 640
mid2vic\debug\prepend_load_address textures a000
mid2vic\debug\prepend_load_address sounds ba70
mid2vic\debug\prepend_load_address hicode ad00
mid2vic\debug\prepend_load_address stackcode 0100
mid2vic\debug\prepend_load_address victory1.txt be00
mid2vic\debug\prepend_load_address victory2.txt be00
mid2vic\debug\prepend_load_address credits.txt be00
mid2vic\debug\prepend_load_address order.txt be00
mid2vic\debug\prepend_load_address help.txt be00
c:\app\vice\c1541 -format doom,id d64 doom.d64 -write doom.prg -write ptextures -write pe1m1 -write pe1m2 -write pe1m3 -write pe1m4 -write pe1m5 -write pe1m6 -write pe1m7 -write pe1m8 -write pe1m9 -write pe1m1mus -write pe1m2mus -write pe1m3mus -write pe1m4mus -write pe1m5mus -write pe1m6mus -write pe1m7mus -write pe1m8mus -write pe1m9mus -write pintermus -write pvictormus -write psluts -write psounds -write plowcode -write phicode -write pstackcode -write phelp.txt -write pvictory1.txt -write pvictory2.txt -write pcredits.txt -write porder.txt
c:\app\vice\xvic doom.d64

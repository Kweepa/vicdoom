cl65 -t vic20 -C doom_cc65.cfg -O -l vicdoom.c menu.c automap.c p_enemy.c summary.c victory.c util.c cheatsAsm.s utilAsm.s enemy.s m_random.s doomlogo.s drawLine.s updateInput.s doomfont.s textures.s dpsounds.s playSound.s drawColumnAsm.s logMathAsm.s mapAsm.s -vm -m map.txt -o doom.prg
@echo off
cc65mapsort map.txt sortedmap.txt
mid2vic\debug\prepend_load_address e1m1 ad00
mid2vic\debug\prepend_load_address e1m2 ad00
mid2vic\debug\prepend_load_address e1m3 ad00
mid2vic\debug\prepend_load_address e1m4 ad00
mid2vic\debug\prepend_load_address e1m5 ad00
mid2vic\debug\prepend_load_address e1m6 ad00
mid2vic\debug\prepend_load_address e1m7 ad00
mid2vic\debug\prepend_load_address e1m8 ad00
mid2vic\debug\prepend_load_address e1m9 ad00
mid2vic\debug\prepend_load_address e1m1mus b700
mid2vic\debug\prepend_load_address e1m2mus b700
mid2vic\debug\prepend_load_address e1m3mus b700
mid2vic\debug\prepend_load_address e1m4mus b700
mid2vic\debug\prepend_load_address e1m5mus b700
mid2vic\debug\prepend_load_address e1m6mus b700
mid2vic\debug\prepend_load_address e1m7mus b700
mid2vic\debug\prepend_load_address e1m8mus b700
mid2vic\debug\prepend_load_address e1m9mus b700
mid2vic\debug\prepend_load_address intermus b700
mid2vic\debug\prepend_load_address victormus b700
mid2vic\debug\prepend_load_address sluts 400
mid2vic\debug\prepend_load_address lowcode 640
mid2vic\debug\prepend_load_address textures a000
mid2vic\debug\prepend_load_address sounds ba20
mid2vic\debug\prepend_load_address midcode 1200
mid2vic\debug\prepend_load_address hicode bcc0
mid2vic\debug\prepend_load_address cred.pt 10f2
mid2vic\debug\prepend_load_address keys.pt 10f2
mid2vic\debug\prepend_load_address buy.pt 10f2
mid2vic\debug\prepend_load_address cred.co 94f2
mid2vic\debug\prepend_load_address keys.co 94f2
mid2vic\debug\prepend_load_address buy.co 94f2
mid2vic\debug\prepend_load_address victory1.txt 1600
mid2vic\debug\prepend_load_address victory2.txt 1600
mid2vic\debug\prepend_load_address credits.txt 1600
mid2vic\debug\prepend_load_address order.txt 1600
mid2vic\debug\prepend_load_address help.txt 1600
c:\app\vice\c1541 -format doom,id d64 doom.d64 -write doom.prg -write ptextures -write pe1m1 -write pe1m2 -write pe1m3 -write pe1m4 -write pe1m5 -write pe1m6 -write pe1m7 -write pe1m8 -write pe1m9 -write pe1m1mus -write pe1m2mus -write pe1m3mus -write pe1m4mus -write pe1m5mus -write pe1m6mus -write pe1m7mus -write pe1m8mus -write pe1m9mus -write pintermus -write pvictormus -write psluts -write psounds -write plowcode -write pmidcode -write phicode -write phelp.txt -write pvictory1.txt -write pvictory2.txt -write pcredits.txt -write porder.txt
c:\app\vice\xvic doom.d64

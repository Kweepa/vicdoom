cl65 -t vic20 -C doom_cc65.cfg -O vicdoom.c menu.c automap.c p_enemy.c summary.c util.c utilAsm.s enemy.s m_random.s doomlogo.s drawLine.s updateInput.s doomfont.s textures.s dpsounds.s playSound.s drawColumnAsm.s logMathAsm.s mapAsm.s -vm -m map.txt -o doom.prg
c:\app\vice\c1541 -format doom,id d64 doom.d64 -write doom.prg -write textures -write e1m1 -write e1m2 -write e1m3 -write e1m4 -write e1m1mus -write sluts -write sounds -write lowcode -write keys.pt -write keys.co -write cred.pt -write cred.co -write buy.pt -write buy.co
c:\app\vice\xvic doom.d64

cl65 -t vic20 -C doom_cc65.cfg -O vicdoom.c menu.c automap.c p_enemy.c enemy.s m_random.s doomlogo.s drawLine.s updateInput.s doomfont.s textures.s dpsounds.s playSound.s drawColumnAsm.s logMathAsm.s mapAsm.s -vm -m map.txt -o doom.prg
c:\app\WinVice-2.1\WinVice-2.1\c1541 -format doom,id d64 doom.d64 -write doom.prg -write textures -write e1m1 -write e1m1mus -write sluts -write sounds
c:\app\WinVice-2.1\WinVice-2.1\xvic-r23120-win32 doom.d64

mf2t d_%1sim_voice1.mid %1sim_v1.txt
mf2t d_%1sim_voice2.mid %1sim_v2.txt
mf2t d_%1sim_voice3.mid %1sim_v3.txt
debug\mid2vic %1 %2
cl65 -tvic20 %1mus.s musicplayer.s testmusic.c -o play%1.prg
copy %1mus ..
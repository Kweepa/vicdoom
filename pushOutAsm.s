.setcpu		"6502"
.autoimport	on
.case		on
.debuginfo	off
.importzp	sp
.export     _pushOut

.segment "CODE"

; pushOut
; takes a pointer to the object in AX
; returns 1 in A if it has to go again
; I'll need to have the data structures in an accessible place and format
; i.e. will need to put them in a separately defined segment

.proc _pushOut: near

rts

.endproc
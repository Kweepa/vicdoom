.if 0
.segment "MAPDATA"

; sector info and objects (one page)

; sector info (32 bytes)
secNumVerts:
.byte 6, 4
.res 30, 0

; total of 32 objects (224 bytes)
objXhi:
.byte 0, 10, -5, -10, -10
.res 27, 0
objXlo:
.res 32, 0
objYhi:
.byte -2, 2, 15, 0, 1
.res 27, 0
objYlo:
.res 32, 0
objAng:
.res 32, 0
objType:
.byte 0, 1, 2, 3, 4
.res 27, 0
objSec:
.byte 0, 0, 1, 0, 0
.res 27, 0

; total of 128 vertices (one page)
vertX:
.byte -20, -20, 20, 20, -10, -10, 0, 0
.res 120, 0
vertY:
.byte -10, 10, 10, -10, 10, 20, 20, 10
.res 120, 0

; total of 128 edges (two pages)
edgeTex:
.byte 0, 1, -1, 1, 2, 0, 1, 2, 0, -1
.res 118, 0
edgeSec:
.byte -1, -1, 1, -1, -1, -1, -1, -1, -1, 0
.res 118, 0
edgeIndex:
.byte -1, -1, 1, -1, -1, -1, -1, -1, -1, 1
.res 118, 0
edgeLen:
.byte 20, 10, 10, 20, 20, 40, 10, 10, 10, 10
.res 118, 0

; total of 32 sectors (two pages)
secVerts:
.byte 0, 1, 4, 7, 2, 3, 0, 0, 4, 5, 6, 7, 0, 0, 0, 0
.res 240, 0
secEdges:
.byte 0, 1, 2, 3, 4, 5, 0, 0, 6, 7, 8, 9, 0, 0, 0, 0
.res 240, 0

; summary data (4 bytes)
numVerts:
.byte 8
numEdges:
.byte 10
numSectors:
.byte 2
numObj:
.byte 3

.endif


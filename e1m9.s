.segment "MAPDATA"
; sector data
secVerts:
.byte 0, 1, 2, 3, 3, 2, 1, 0
.res 504, 0

secEdges:
.byte 1, 2, 3, 0, 3, 4, 3, 4
.res 504, 0

; summary data (8 bytes)
numVerts:
.byte 4
numEdges:
.byte 4
numSectors:
.byte 1
numObj:
.byte 1
playerSpawnX:
.byte 1
playerSpawnY:
.byte -10
playerSpawnAngle:
.byte 0
playerSpawnSector:
.byte 0
numEnemies:
.byte 1
numItems:
.byte 0
numSecrets:
.byte 0
parTime:
.byte 30
secretSectors:
.res 8, 0

; sector info
secNumVerts:
.byte 4
.res 63, 0

; object data
objXhi:
.byte -1
.res 47, 0

objYhi:
.byte 10
.res 47, 0

objType:
.byte 1
.res 47, 0

objSec:
.byte 0
.res 47, 0

; vertex data
vertX:
.byte -20, -20, 20, 20
.res 136, 0

vertY:
.byte -20, 20, 20, -20
.res 136, 0

; edge data
edgeTex:
.byte 0, 0, 0, 0
.res 196, 0

edgeSec1:
.byte -1, -1, -1, -1
.res 196, 0

edgeSec2:
.byte -1, -1, -1, -1
.res 196, 0

edgeLen:
.byte 40, 40, 40, 40
.res 196, 0


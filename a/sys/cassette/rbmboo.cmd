!
! BOOTSTRAP ON UP, GOING MULTI USER
!
SET DEF HEX
SET DEF LONG
SET REL:0
HALT
UNJAM
INIT
LOAD BOOT
D R10 3		! DEVICE CHOICE 3=RB
D R11 0		! 0= AUTOBOOT
START 2

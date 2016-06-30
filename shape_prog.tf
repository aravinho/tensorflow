declare input x
declare weight w
declare exp_output y
declare output o
declare intvar neg_y
declare intvar diff
declare loss lambda

define o  =  mul  x  w
define neg_y  =  mul  -1  y
define diff  =  add  o  neg_y
define lambda  =  mul  diff  diff


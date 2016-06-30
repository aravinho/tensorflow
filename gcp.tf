declare input x
declare input w
declare input y
declare intvar o
declare intvar neg_y
declare intvar diff
declare intvar lambda

define o  =  mul  x  w
define neg_y  =  mul  -1  y
define diff  =  add  o  neg_y
define lambda  =  mul  diff  diff


declare intvar d/lambda/d/lambda
define d/lambda/d/lambda = 1
declare intvar d/lambda/d/diff
define d/lambda/d/diff = mul 2 diff

declare intvar d/diff/d/o
declare intvar d/diff/d/neg_y
define d/diff/d/o = 1
define d/diff/d/neg_y = 1

declare intvar d/lambda/d/neg_y
define d/lambda/d/neg_y = mul d/lambda/d/diff d/diff/d/neg_y
declare intvar d/neg_y/d/y
define d/neg_y/d/y = -1

declare intvar d/lambda/d/y
define d/lambda/d/y = mul d/lambda/d/neg_y d/neg_y/d/y

declare intvar d/lambda/d/o
define d/lambda/d/o = mul d/lambda/d/diff d/diff/d/o
declare intvar d/o/d/x
declare intvar d/o/d/w
define d/o/d/x = w
define d/o/d/w = x

declare output d/lambda/d/w
define d/lambda/d/w = mul d/lambda/d/o d/o/d/w

declare intvar d/lambda/d/x
define d/lambda/d/x = mul d/lambda/d/o d/o/d/x


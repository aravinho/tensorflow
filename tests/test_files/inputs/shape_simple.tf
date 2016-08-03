#macro z = b_macro x y; declare intvar p; define p = mul x x; define z = mul y p;
#macro z = u_macro x; declare intvar p; declare intvar q; define p = add x -2; define q = add x 2; define z = mul p q;

declare_vector input a 3
declare_vector weight b 3
declare_vector input c 3
declare_vector weight d 3
declare_vector exp_output e 3
declare_vector exp_output f 3

declare output foo
define foo = dot a b

declare output bar
define bar = exp foo

declare output baz
define baz = ln bar

declare output baz_squared
define baz_squared = mul baz baz

declare_vector output A 3
define_vector A = add c d

declare_vector output B 3
define_vector B = b_macro e f

declare_vector output C 3
define_vector C = mul A foo

declare_vector output D 3
define_vector D = pow B 2

declare_vector output E 3
define_vector E = u_macro C

declare_vector output F 3
define_vector F = logistic D

declare output G
define G = dot E F

declare loss LAMBDA
define LAMBDA = add baz_squared G

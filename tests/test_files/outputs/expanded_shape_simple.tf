declare input a.0
declare input a.1
declare input a.2
declare weight b.0
declare weight b.1
declare weight b.2
declare input c.0
declare input c.1
declare input c.2
declare weight d.0
declare weight d.1
declare weight d.2
declare exp_output e.0
declare exp_output e.1
declare exp_output e.2
declare exp_output f.0
declare exp_output f.1
declare exp_output f.2
declare output foo
declare intvar foo.0
define foo.0 = mul a.0 b.0
declare intvar foo.1
define foo.1 = mul a.1 b.1
declare intvar foo.2
define foo.2 = mul a.2 b.2
declare intvar foo.3
define foo.3 = add foo.0 foo.1
declare intvar foo.4
define foo.4 = add foo.3 foo.2
define foo = foo.4
declare output bar
define bar = exp foo
declare output baz
define baz = ln bar
declare output baz_squared
define baz_squared = mul baz baz
declare output A.0
declare output A.1
declare output A.2
define A.0 = add c.0 d.0
define A.1 = add c.1 d.1
define A.2 = add c.2 d.2
declare output B.0
declare output B.1
declare output B.2
declare intvar B.0_p0
define B.0_p0 = mul e.0 e.0
define B.0 = mul f.0 B.0_p0
declare intvar B.1_p1
define B.1_p1 = mul e.1 e.1
define B.1 = mul f.1 B.1_p1
declare intvar B.2_p2
define B.2_p2 = mul e.2 e.2
define B.2 = mul f.2 B.2_p2
declare output C.0
declare output C.1
declare output C.2
define C.0 = mul A.0 foo
define C.1 = mul A.1 foo
define C.2 = mul A.2 foo
declare output D.0
declare output D.1
declare output D.2
define D.0 = pow B.0 2
define D.1 = pow B.1 2
define D.2 = pow B.2 2
declare output E.0
declare output E.1
declare output E.2
declare intvar E.0_p0
declare intvar E.0_q0
define E.0_p0 = add C.0 -2
define E.0_q0 = add C.0 2
define E.0 = mul E.0_p0 E.0_q0
declare intvar E.1_p1
declare intvar E.1_q1
define E.1_p1 = add C.1 -2
define E.1_q1 = add C.1 2
define E.1 = mul E.1_p1 E.1_q1
declare intvar E.2_p2
declare intvar E.2_q2
define E.2_p2 = add C.2 -2
define E.2_q2 = add C.2 2
define E.2 = mul E.2_p2 E.2_q2
declare output F.0
declare output F.1
declare output F.2
define F.0 = logistic D.0
define F.1 = logistic D.1
define F.2 = logistic D.2
declare output G
declare intvar G.0
define G.0 = mul E.0 F.0
declare intvar G.1
define G.1 = mul E.1 F.1
declare intvar G.2
define G.2 = mul E.2 F.2
declare intvar G.3
define G.3 = add G.0 G.1
declare intvar G.4
define G.4 = add G.3 G.2
define G = G.4
declare loss LAMBDA
define LAMBDA = add baz_squared G

declare input x.0
declare input x.1
declare input x.2
declare weight w.0
declare weight w.1
declare weight w.2
declare intvar A
declare intvar A.0
define A.0 = mul x.0 w.0
declare intvar A.1
define A.1 = mul x.1 w.1
declare intvar A.2
define A.2 = mul x.2 w.2
declare intvar A.3
define A.3 = add A.0 A.1
declare intvar A.4
define A.4 = add A.3 A.2
define A = A.4
declare intvar u.0
declare intvar u.1
declare intvar u.2
declare intvar v.0
declare intvar v.1
declare intvar v.2
define u.0 = add x.0 w.0
define u.1 = add x.1 w.1
define u.2 = add x.2 w.2
define v.0 = mul x.0 u.0
define v.1 = mul x.1 u.1
define v.2 = mul x.2 u.2
declare intvar p.0
declare intvar p.1
declare intvar p.2
declare intvar q.0
declare intvar q.1
declare intvar q.2
define p.0 = mul u.0 -2.3
define p.1 = mul u.1 -2.3
define p.2 = mul u.2 -2.3
define q.0 = add v.0 19.20
define q.1 = add v.1 19.20
define q.2 = add v.2 19.20
declare intvar B
declare intvar B.0
define B.0 = mul p.0 q.0
declare intvar B.1
define B.1 = mul p.1 q.1
declare intvar B.2
define B.2 = mul p.2 q.2
declare intvar B.3
define B.3 = add B.0 B.1
declare intvar B.4
define B.4 = add B.3 B.2
define B = B.4
declare intvar C
declare intvar C_p0
define C_p0 = mul A A
define C = mul B C_p0
declare intvar D
define D = add A C
declare intvar E
declare intvar E_p0
declare intvar E_q0
define E_p0 = add B -2
define E_q0 = add B 2
define E = mul E_p0 E_q0
declare intvar F
define F = logistic D
declare intvar G
define G = mul E C
declare intvar H
define H = ln E
declare intvar I
define I = pow G H
declare output J
define J = exp I
declare exp_output Y
declare loss LAMBDA
define LAMBDA = add J Y
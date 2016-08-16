declare input x.0
declare input x.1
declare input x.2
declare input w.0
declare input w.1
declare input w.2
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
declare intvar J
define J = exp I
declare input Y
declare intvar LAMBDA
define LAMBDA = add J Y
declare intvar d/LAMBDA/d/LAMBDA
define d/LAMBDA/d/LAMBDA = 1
declare intvar d/LAMBDA/d/J
declare intvar d/LAMBDA/d/Y
define d/LAMBDA/d/J = 1
define d/LAMBDA/d/Y = 1
declare intvar d/J/d/I
define d/J/d/I = exp I
declare intvar d/LAMBDA/d/I
define d/LAMBDA/d/I = mul d/LAMBDA/d/J d/J/d/I
declare intvar d/I/d/G
declare intvar d/I/d/H
declare intvar d/I/d/G:0
declare intvar d/I/d/G:1
define d/I/d/G:0 = sub H 1
define d/I/d/G:1 = pow G d/I/d/G:0
define d/I/d/G = mul H d/I/d/G:1
declare intvar d/I/d/H:0
declare intvar d/I/d/H:1
define d/I/d/H:0 = pow G H
define d/I/d/H:1 = ln G
define d/I/d/H = mul d/I/d/H:0 d/I/d/H:1
declare intvar d/LAMBDA/d/H
define d/LAMBDA/d/H = mul d/LAMBDA/d/I d/I/d/H
declare intvar d/H/d/E
define d/H/d/E = pow E -1
declare intvar d/LAMBDA/d/G
define d/LAMBDA/d/G = mul d/LAMBDA/d/I d/I/d/G
declare intvar d/G/d/E
declare intvar d/G/d/C
define d/G/d/E = C
define d/G/d/C = E
declare intvar d/LAMBDA/d/C
define d/LAMBDA/d/C = mul d/LAMBDA/d/G d/G/d/C
declare intvar d/C/d/B
declare intvar d/C/d/C_p0
define d/C/d/B = C_p0
define d/C/d/C_p0 = B
declare intvar d/LAMBDA/d/C_p0
define d/LAMBDA/d/C_p0 = mul d/LAMBDA/d/C d/C/d/C_p0
declare intvar d/C_p0/d/A
define d/C_p0/d/A = mul 2 A
declare intvar d/LAMBDA/d/A
define d/LAMBDA/d/A = mul d/LAMBDA/d/C_p0 d/C_p0/d/A
declare intvar d/A/d/A.4
define d/A/d/A.4 = 1
declare intvar d/LAMBDA/d/A.4
define d/LAMBDA/d/A.4 = mul d/LAMBDA/d/A d/A/d/A.4
declare intvar d/A.4/d/A.3
declare intvar d/A.4/d/A.2
define d/A.4/d/A.3 = 1
define d/A.4/d/A.2 = 1
declare intvar d/LAMBDA/d/A.2
define d/LAMBDA/d/A.2 = mul d/LAMBDA/d/A.4 d/A.4/d/A.2
declare intvar d/A.2/d/x.2
declare intvar d/A.2/d/w.2
define d/A.2/d/x.2 = w.2
define d/A.2/d/w.2 = x.2
declare intvar d/LAMBDA/d/A.3
define d/LAMBDA/d/A.3 = mul d/LAMBDA/d/A.4 d/A.4/d/A.3
declare intvar d/A.3/d/A.0
declare intvar d/A.3/d/A.1
define d/A.3/d/A.0 = 1
define d/A.3/d/A.1 = 1
declare intvar d/LAMBDA/d/A.1
define d/LAMBDA/d/A.1 = mul d/LAMBDA/d/A.3 d/A.3/d/A.1
declare intvar d/A.1/d/x.1
declare intvar d/A.1/d/w.1
define d/A.1/d/x.1 = w.1
define d/A.1/d/w.1 = x.1
declare intvar d/LAMBDA/d/A.0
define d/LAMBDA/d/A.0 = mul d/LAMBDA/d/A.3 d/A.3/d/A.0
declare intvar d/A.0/d/x.0
declare intvar d/A.0/d/w.0
define d/A.0/d/x.0 = w.0
define d/A.0/d/w.0 = x.0
declare intvar d/LAMBDA/d/E
define d/LAMBDA/d/E = mul d/LAMBDA/d/G d/G/d/E
declare intvar d/E/d/E_p0
declare intvar d/E/d/E_q0
define d/E/d/E_p0 = E_q0
define d/E/d/E_q0 = E_p0
declare intvar d/LAMBDA/d/E_q0
define d/LAMBDA/d/E_q0 = mul d/LAMBDA/d/E d/E/d/E_q0
declare intvar d/E_q0/d/B
define d/E_q0/d/B = 1
declare intvar d/LAMBDA/d/E_p0
define d/LAMBDA/d/E_p0 = mul d/LAMBDA/d/E d/E/d/E_p0
declare intvar d/E_p0/d/B
define d/E_p0/d/B = 1
declare intvar d/LAMBDA/d/B
define d/LAMBDA/d/B = mul d/LAMBDA/d/C d/C/d/B
declare intvar d/B/d/B.4
define d/B/d/B.4 = 1
declare intvar d/LAMBDA/d/B.4
define d/LAMBDA/d/B.4 = mul d/LAMBDA/d/B d/B/d/B.4
declare intvar d/B.4/d/B.3
declare intvar d/B.4/d/B.2
define d/B.4/d/B.3 = 1
define d/B.4/d/B.2 = 1
declare intvar d/LAMBDA/d/B.2
define d/LAMBDA/d/B.2 = mul d/LAMBDA/d/B.4 d/B.4/d/B.2
declare intvar d/B.2/d/p.2
declare intvar d/B.2/d/q.2
define d/B.2/d/p.2 = q.2
define d/B.2/d/q.2 = p.2
declare intvar d/LAMBDA/d/q.2
define d/LAMBDA/d/q.2 = mul d/LAMBDA/d/B.2 d/B.2/d/q.2
declare intvar d/q.2/d/v.2
define d/q.2/d/v.2 = 1
declare intvar d/LAMBDA/d/v.2
define d/LAMBDA/d/v.2 = mul d/LAMBDA/d/q.2 d/q.2/d/v.2
declare intvar d/v.2/d/x.2
declare intvar d/v.2/d/u.2
define d/v.2/d/x.2 = u.2
define d/v.2/d/u.2 = x.2
declare intvar d/LAMBDA/d/p.2
define d/LAMBDA/d/p.2 = mul d/LAMBDA/d/B.2 d/B.2/d/p.2
declare intvar d/p.2/d/u.2
define d/p.2/d/u.2 = -2.3
declare intvar d/LAMBDA/d/u.2
define d/LAMBDA/d/u.2 = mul d/LAMBDA/d/p.2 d/p.2/d/u.2
declare intvar d/u.2/d/x.2
declare intvar d/u.2/d/w.2
define d/u.2/d/x.2 = 1
define d/u.2/d/w.2 = 1
declare output d/LAMBDA/d/w.2
define d/LAMBDA/d/w.2 = mul d/LAMBDA/d/A.2 d/A.2/d/w.2
declare intvar d/LAMBDA/d/x.2
define d/LAMBDA/d/x.2 = mul d/LAMBDA/d/A.2 d/A.2/d/x.2
declare intvar d/LAMBDA/d/B.3
define d/LAMBDA/d/B.3 = mul d/LAMBDA/d/B.4 d/B.4/d/B.3
declare intvar d/B.3/d/B.0
declare intvar d/B.3/d/B.1
define d/B.3/d/B.0 = 1
define d/B.3/d/B.1 = 1
declare intvar d/LAMBDA/d/B.1
define d/LAMBDA/d/B.1 = mul d/LAMBDA/d/B.3 d/B.3/d/B.1
declare intvar d/B.1/d/p.1
declare intvar d/B.1/d/q.1
define d/B.1/d/p.1 = q.1
define d/B.1/d/q.1 = p.1
declare intvar d/LAMBDA/d/q.1
define d/LAMBDA/d/q.1 = mul d/LAMBDA/d/B.1 d/B.1/d/q.1
declare intvar d/q.1/d/v.1
define d/q.1/d/v.1 = 1
declare intvar d/LAMBDA/d/v.1
define d/LAMBDA/d/v.1 = mul d/LAMBDA/d/q.1 d/q.1/d/v.1
declare intvar d/v.1/d/x.1
declare intvar d/v.1/d/u.1
define d/v.1/d/x.1 = u.1
define d/v.1/d/u.1 = x.1
declare intvar d/LAMBDA/d/p.1
define d/LAMBDA/d/p.1 = mul d/LAMBDA/d/B.1 d/B.1/d/p.1
declare intvar d/p.1/d/u.1
define d/p.1/d/u.1 = -2.3
declare intvar d/LAMBDA/d/u.1
define d/LAMBDA/d/u.1 = mul d/LAMBDA/d/p.1 d/p.1/d/u.1
declare intvar d/u.1/d/x.1
declare intvar d/u.1/d/w.1
define d/u.1/d/x.1 = 1
define d/u.1/d/w.1 = 1
declare output d/LAMBDA/d/w.1
define d/LAMBDA/d/w.1 = mul d/LAMBDA/d/A.1 d/A.1/d/w.1
declare intvar d/LAMBDA/d/x.1
define d/LAMBDA/d/x.1 = mul d/LAMBDA/d/A.1 d/A.1/d/x.1
declare intvar d/LAMBDA/d/B.0
define d/LAMBDA/d/B.0 = mul d/LAMBDA/d/B.3 d/B.3/d/B.0
declare intvar d/B.0/d/p.0
declare intvar d/B.0/d/q.0
define d/B.0/d/p.0 = q.0
define d/B.0/d/q.0 = p.0
declare intvar d/LAMBDA/d/q.0
define d/LAMBDA/d/q.0 = mul d/LAMBDA/d/B.0 d/B.0/d/q.0
declare intvar d/q.0/d/v.0
define d/q.0/d/v.0 = 1
declare intvar d/LAMBDA/d/v.0
define d/LAMBDA/d/v.0 = mul d/LAMBDA/d/q.0 d/q.0/d/v.0
declare intvar d/v.0/d/x.0
declare intvar d/v.0/d/u.0
define d/v.0/d/x.0 = u.0
define d/v.0/d/u.0 = x.0
declare intvar d/LAMBDA/d/p.0
define d/LAMBDA/d/p.0 = mul d/LAMBDA/d/B.0 d/B.0/d/p.0
declare intvar d/p.0/d/u.0
define d/p.0/d/u.0 = -2.3
declare intvar d/LAMBDA/d/u.0
define d/LAMBDA/d/u.0 = mul d/LAMBDA/d/p.0 d/p.0/d/u.0
declare intvar d/u.0/d/x.0
declare intvar d/u.0/d/w.0
define d/u.0/d/x.0 = 1
define d/u.0/d/w.0 = 1
declare output d/LAMBDA/d/w.0
define d/LAMBDA/d/w.0 = mul d/LAMBDA/d/A.0 d/A.0/d/w.0
declare intvar d/LAMBDA/d/x.0
define d/LAMBDA/d/x.0 = mul d/LAMBDA/d/A.0 d/A.0/d/x.0

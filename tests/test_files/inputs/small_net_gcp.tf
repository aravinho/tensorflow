declare input a
declare input b
declare input c
declare input f
declare input g
declare input h
declare intvar i
declare intvar j
declare intvar k
declare input m
declare input n
declare input p
declare intvar a_times_f
define a_times_f = mul a f
declare intvar b_times_g
define b_times_g = mul b g
declare intvar c_times_h
define c_times_h = mul c h
define i = logistic a_times_f
define j = logistic b_times_g
define k = logistic c_times_h
declare intvar neg_m
declare intvar neg_n
declare intvar neg_p
define neg_m = mul m -1
define neg_n = mul n -1
define neg_p = mul p -1
declare intvar i_minus_m
declare intvar j_minus_n
declare intvar k_minus_p
define i_minus_m = add i neg_m
define j_minus_n = add j neg_n
define k_minus_p = add k neg_p
declare intvar i_minus_m_squared
declare intvar j_minus_n_squared
declare intvar k_minus_p_squared
define i_minus_m_squared = pow i_minus_m 2
define j_minus_n_squared = pow j_minus_n 2
define k_minus_p_squared = pow k_minus_p 2
declare intvar loss_one
define loss_one = add i_minus_m_squared j_minus_n_squared
declare intvar loss_two
define loss_two = add loss_one k_minus_p_squared
declare intvar one_third
define one_third = pow 3 -1
declare intvar LAMBDA
define LAMBDA = mul loss_two one_third
declare intvar d/LAMBDA/d/LAMBDA
define d/LAMBDA/d/LAMBDA = 1
declare intvar d/LAMBDA/d/loss_two
declare intvar d/LAMBDA/d/one_third
define d/LAMBDA/d/loss_two = one_third
define d/LAMBDA/d/one_third = loss_two
declare intvar d/loss_two/d/loss_one
declare intvar d/loss_two/d/k_minus_p_squared
define d/loss_two/d/loss_one = 1
define d/loss_two/d/k_minus_p_squared = 1
declare intvar d/LAMBDA/d/k_minus_p_squared
define d/LAMBDA/d/k_minus_p_squared = mul d/LAMBDA/d/loss_two d/loss_two/d/k_minus_p_squared
declare intvar d/k_minus_p_squared/d/k_minus_p
declare intvar d/k_minus_p_squared/d/k_minus_p:0
declare intvar d/k_minus_p_squared/d/k_minus_p:1
define d/k_minus_p_squared/d/k_minus_p:0 = add -1 2
define d/k_minus_p_squared/d/k_minus_p:1 = pow k_minus_p d/k_minus_p_squared/d/k_minus_p:0
define d/k_minus_p_squared/d/k_minus_p = mul 2 d/k_minus_p_squared/d/k_minus_p:1
declare intvar d/LAMBDA/d/k_minus_p
define d/LAMBDA/d/k_minus_p = mul d/LAMBDA/d/k_minus_p_squared d/k_minus_p_squared/d/k_minus_p
declare intvar d/k_minus_p/d/k
declare intvar d/k_minus_p/d/neg_p
define d/k_minus_p/d/k = 1
define d/k_minus_p/d/neg_p = 1
declare intvar d/LAMBDA/d/neg_p
define d/LAMBDA/d/neg_p = mul d/LAMBDA/d/k_minus_p d/k_minus_p/d/neg_p
declare intvar d/neg_p/d/p
define d/neg_p/d/p = -1
declare intvar d/LAMBDA/d/p
define d/LAMBDA/d/p = mul d/LAMBDA/d/neg_p d/neg_p/d/p
declare intvar d/LAMBDA/d/k
define d/LAMBDA/d/k = mul d/LAMBDA/d/k_minus_p d/k_minus_p/d/k
declare intvar d/k/d/c_times_h
declare intvar d/k/d/c_times_h:0
declare intvar d/k/d/c_times_h:1
declare intvar d/k/d/c_times_h:2
declare intvar d/k/d/c_times_h:3
define d/k/d/c_times_h:0 = exp c_times_h
define d/k/d/c_times_h:1 = add 1 d/k/d/c_times_h:0
define d/k/d/c_times_h:2 = pow d/k/d/c_times_h:1 2
define d/k/d/c_times_h:3 = pow d/k/d/c_times_h:2 -1
define d/k/d/c_times_h = mul d/k/d/c_times_h:0 d/k/d/c_times_h:3
declare intvar d/LAMBDA/d/c_times_h
define d/LAMBDA/d/c_times_h = mul d/LAMBDA/d/k d/k/d/c_times_h
declare intvar d/c_times_h/d/c
declare intvar d/c_times_h/d/h
define d/c_times_h/d/c = h
define d/c_times_h/d/h = c
declare output d/LAMBDA/d/h
define d/LAMBDA/d/h = mul d/LAMBDA/d/c_times_h d/c_times_h/d/h
declare intvar d/LAMBDA/d/c
define d/LAMBDA/d/c = mul d/LAMBDA/d/c_times_h d/c_times_h/d/c
declare intvar d/LAMBDA/d/loss_one
define d/LAMBDA/d/loss_one = mul d/LAMBDA/d/loss_two d/loss_two/d/loss_one
declare intvar d/loss_one/d/i_minus_m_squared
declare intvar d/loss_one/d/j_minus_n_squared
define d/loss_one/d/i_minus_m_squared = 1
define d/loss_one/d/j_minus_n_squared = 1
declare intvar d/LAMBDA/d/j_minus_n_squared
define d/LAMBDA/d/j_minus_n_squared = mul d/LAMBDA/d/loss_one d/loss_one/d/j_minus_n_squared
declare intvar d/j_minus_n_squared/d/j_minus_n
declare intvar d/j_minus_n_squared/d/j_minus_n:0
declare intvar d/j_minus_n_squared/d/j_minus_n:1
define d/j_minus_n_squared/d/j_minus_n:0 = add -1 2
define d/j_minus_n_squared/d/j_minus_n:1 = pow j_minus_n d/j_minus_n_squared/d/j_minus_n:0
define d/j_minus_n_squared/d/j_minus_n = mul 2 d/j_minus_n_squared/d/j_minus_n:1
declare intvar d/LAMBDA/d/j_minus_n
define d/LAMBDA/d/j_minus_n = mul d/LAMBDA/d/j_minus_n_squared d/j_minus_n_squared/d/j_minus_n
declare intvar d/j_minus_n/d/j
declare intvar d/j_minus_n/d/neg_n
define d/j_minus_n/d/j = 1
define d/j_minus_n/d/neg_n = 1
declare intvar d/LAMBDA/d/neg_n
define d/LAMBDA/d/neg_n = mul d/LAMBDA/d/j_minus_n d/j_minus_n/d/neg_n
declare intvar d/neg_n/d/n
define d/neg_n/d/n = -1
declare intvar d/LAMBDA/d/n
define d/LAMBDA/d/n = mul d/LAMBDA/d/neg_n d/neg_n/d/n
declare intvar d/LAMBDA/d/j
define d/LAMBDA/d/j = mul d/LAMBDA/d/j_minus_n d/j_minus_n/d/j
declare intvar d/j/d/b_times_g
declare intvar d/j/d/b_times_g:0
declare intvar d/j/d/b_times_g:1
declare intvar d/j/d/b_times_g:2
declare intvar d/j/d/b_times_g:3
define d/j/d/b_times_g:0 = exp b_times_g
define d/j/d/b_times_g:1 = add 1 d/j/d/b_times_g:0
define d/j/d/b_times_g:2 = pow d/j/d/b_times_g:1 2
define d/j/d/b_times_g:3 = pow d/j/d/b_times_g:2 -1
define d/j/d/b_times_g = mul d/j/d/b_times_g:0 d/j/d/b_times_g:3
declare intvar d/LAMBDA/d/b_times_g
define d/LAMBDA/d/b_times_g = mul d/LAMBDA/d/j d/j/d/b_times_g
declare intvar d/b_times_g/d/b
declare intvar d/b_times_g/d/g
define d/b_times_g/d/b = g
define d/b_times_g/d/g = b
declare output d/LAMBDA/d/g
define d/LAMBDA/d/g = mul d/LAMBDA/d/b_times_g d/b_times_g/d/g
declare intvar d/LAMBDA/d/b
define d/LAMBDA/d/b = mul d/LAMBDA/d/b_times_g d/b_times_g/d/b
declare intvar d/LAMBDA/d/i_minus_m_squared
define d/LAMBDA/d/i_minus_m_squared = mul d/LAMBDA/d/loss_one d/loss_one/d/i_minus_m_squared
declare intvar d/i_minus_m_squared/d/i_minus_m
declare intvar d/i_minus_m_squared/d/i_minus_m:0
declare intvar d/i_minus_m_squared/d/i_minus_m:1
define d/i_minus_m_squared/d/i_minus_m:0 = add -1 2
define d/i_minus_m_squared/d/i_minus_m:1 = pow i_minus_m d/i_minus_m_squared/d/i_minus_m:0
define d/i_minus_m_squared/d/i_minus_m = mul 2 d/i_minus_m_squared/d/i_minus_m:1
declare intvar d/LAMBDA/d/i_minus_m
define d/LAMBDA/d/i_minus_m = mul d/LAMBDA/d/i_minus_m_squared d/i_minus_m_squared/d/i_minus_m
declare intvar d/i_minus_m/d/i
declare intvar d/i_minus_m/d/neg_m
define d/i_minus_m/d/i = 1
define d/i_minus_m/d/neg_m = 1
declare intvar d/LAMBDA/d/neg_m
define d/LAMBDA/d/neg_m = mul d/LAMBDA/d/i_minus_m d/i_minus_m/d/neg_m
declare intvar d/neg_m/d/m
define d/neg_m/d/m = -1
declare intvar d/LAMBDA/d/m
define d/LAMBDA/d/m = mul d/LAMBDA/d/neg_m d/neg_m/d/m
declare intvar d/LAMBDA/d/i
define d/LAMBDA/d/i = mul d/LAMBDA/d/i_minus_m d/i_minus_m/d/i
declare intvar d/i/d/a_times_f
declare intvar d/i/d/a_times_f:0
declare intvar d/i/d/a_times_f:1
declare intvar d/i/d/a_times_f:2
declare intvar d/i/d/a_times_f:3
define d/i/d/a_times_f:0 = exp a_times_f
define d/i/d/a_times_f:1 = add 1 d/i/d/a_times_f:0
define d/i/d/a_times_f:2 = pow d/i/d/a_times_f:1 2
define d/i/d/a_times_f:3 = pow d/i/d/a_times_f:2 -1
define d/i/d/a_times_f = mul d/i/d/a_times_f:0 d/i/d/a_times_f:3
declare intvar d/LAMBDA/d/a_times_f
define d/LAMBDA/d/a_times_f = mul d/LAMBDA/d/i d/i/d/a_times_f
declare intvar d/a_times_f/d/a
declare intvar d/a_times_f/d/f
define d/a_times_f/d/a = f
define d/a_times_f/d/f = a
declare output d/LAMBDA/d/f
define d/LAMBDA/d/f = mul d/LAMBDA/d/a_times_f d/a_times_f/d/f
declare intvar d/LAMBDA/d/a
define d/LAMBDA/d/a = mul d/LAMBDA/d/a_times_f d/a_times_f/d/a

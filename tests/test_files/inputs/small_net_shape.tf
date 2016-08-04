declare input a
declare input b
declare input c

declare weight f
declare weight g
declare weight h

declare output i
declare output j
declare output k

declare exp_output m
declare exp_output n
declare exp_output p

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
declare loss LAMBDA
define LAMBDA = mul loss_two one_third

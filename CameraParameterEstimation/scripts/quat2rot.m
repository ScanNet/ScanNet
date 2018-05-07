function R = quat2rot(Q)
% QUAT2ROT
%   R = QUAT2ROT(Q) converts a quaternion (4x1 or 1x4) into a 3x3 rotation mattrix
%
%   reference: google!
%
%   Babak Taati, 2003
%   (revised 2009)

q0 = Q(1);
q1 = Q(2);
q2 = Q(3);
q3 = Q(4);

R(1,1)  = q0*q0  +  q1*q1  -  q2*q2  -  q3*q3;
R(1,2)  = 2 * (q1*q2  -  q0*q3);
R(1,3)  = 2 * (q1*q3  +  q0*q2);

R(2,1)  = 2 * (q1*q2  +  q0*q3);
R(2,2)  = q0*q0  -  q1*q1  +  q2*q2  -  q3*q3;
R(2,3)  = 2 * (q2*q3  -  q0*q1);

R(3,1)  = 2 * (q1*q3  -  q0*q2);
R(3,2)  = 2 * (q2*q3  +  q0*q1);
R(3,3)  = q0*q0  -  q1*q1  -  q2*q2  +  q3*q3;




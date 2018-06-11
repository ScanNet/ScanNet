function V_times = cross_times_matrix(V)
% CROSSTIMESMATRIX
%   V_TIMES = CROSSTIMESMATRIX(V) returns a 3x3 (or a series of 3x3) cross times matrices of input vector(s) V
% 
%   Input:
%       V a 3xN matrix, rpresenting a series of 3x1 vectors
% 
%   Output:   
%       V_TIMES (Vx) a series of 3x3 matrices where V_times(:,:,i) is the Vx matrix for the vector V(:,i)
% 
% 	Babak Taati, 2003
%   (revised 2009)

[a,b] = size(V);
V_times = zeros(a, 3, b);

% V_times(1,1,:) = 0;
V_times(1,2,:) = - V(3,:);
V_times(1,3,:) = V(2,:);

V_times(2,1,:) = V(3,:);
% V_times(2,2,:) = 0;
V_times(2,3,:) = - V(1,:);

V_times(3,1,:) = - V(2,:);
V_times(3,2,:) = V(1,:);
% V_times(3,3,:) = 0;

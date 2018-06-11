function [T, Eps] = estimate_rigid_transform(x, y)
% ESTIMATERIGIDTRANSFORM
%   [T, EPS] = ESTIMATERIGIDTRANSFORM(X, Y) estimates the rigid transformation
%   that best aligns x with y (in the least-squares sense).
%  
%   Reference: "Estimating Rigid Transformations" in 
%   "Computer Vision, a modern approach" by Forsyth and Ponce (1993), page 480
%   (page 717(?) of the newer edition)
%
%   Input:
%       X: 3xN, N 3-D points (N>=3)
%       Y: 3xN, N 3-D points (N>=3)
%
%   Output
%       T: the rigid transformation that aligns x and y as:  xh = T * yh
%          (h denotes homogenous coordinates)  
%          (corrspondence between points x(:,i) and y(:,i) is assumed)
%       
%       EPS: the smallest singular value. The closer this value it is 
%          to 0, the better the estimate is. (large values mean that the 
%          transform between the two data sets cannot be approximated
%          well with a rigid transform.
%
%   Babak Taati, 2003
%   (revised 2009)

if nargin ~= 2
    error('Requires two input arguments.')
end

if size(x,1)~=3 || size(y,1)~=3
    error('Input point clouds must be a 3xN matrix.');
end

if size(x, 2) ~= size(y,2)
    error('Input point clouds must be of the same size');
end                            

if size(x,2)<3 || size(y,2)<3
    error('At least 3 point matches are needed');
end                            
    
pointCount = length(x); % since x has N=3+ points, length shows the number of points
                    
x_centroid = sum(x,2) / pointCount;
y_centroid = sum(y,2) / pointCount; 

x_centrized = [x(1,:)-x_centroid(1) ; x(2,:)-x_centroid(2); x(3,:)-x_centroid(3)];
y_centrized = [y(1,:)-y_centroid(1) ; y(2,:)-y_centroid(2); y(3,:)-y_centroid(3)];

R12 = y_centrized' - x_centrized';
R21 = x_centrized - y_centrized;
R22_1 = y_centrized  + x_centrized;
R22 = cross_times_matrix(R22_1(1:3,:));

B = zeros(4, 4);
A = zeros(4, 4, pointCount);
for ii=1:pointCount
    A(1:4,1:4,ii) = [0, R12(ii,1:3); R21(1:3,ii), R22(1:3,1:3,ii)];
    B = B + A(:,:,ii)' * A(:,:,ii);
end

[dummy, S, V] = svd(B);
quat = V(:,4);
rot = quat2rot(quat);

T1 = [eye(3,3), -y_centroid ; 0 0 0 1];
T2 = [rot, [0; 0; 0]; 0 0 0 1];
T3 = [eye(3,3), x_centroid ;  0 0 0 1];

T = T3 * T2 * T1;
Eps = S(4,4);

                    
                    

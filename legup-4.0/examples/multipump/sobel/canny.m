function [sFinal,thresh] = canny(img, mLow, mHigh, sig)
% Canny edge detector 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% function  [sFinal, thresh] =canny(img, mLow, mHigh, sig)
% Applies the canny edge detection algo to the given image
%  img  : the given image (matrix) in color or B/W   
%  mLow : the threshold is set adaptively: low_threshold = mLow* mean_intensity(im_gradient)
%  mHigh: the threshol is set adaptively: high_threshold= mHigh*low_threshold
%  sig  : the value of sigma for the derivative of gaussian operator
%
% The default values for (sig, mLow, mHigh) are (1, 0.5, 2.5)
% The function displays the image and also returns:
%  sFinal       : the final (B/W) image with edges
%  thresh       :  =[lowT, highT] the actual low and high thresholds used
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CS223b HW# 1a: Canny Edge Detector
% Rohit Singh & Mitul Saha:  {rohitsi, mitul}@stanford.edu
%
% Good parameters: 
%     elephant.jpg :  canny(eim,  1.5, 2.6, 1)
%     macbeth.jpg  :  canny(mim, .1, 9, 2); recognises 21 of 24 squares
%     hecface.jpg  :  canny(him, .6, 3, 1); or canny(him, .4,3.5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if (nargin < 1)
  error(' Need a NxNx3 or NxN image matrix');
elseif (nargin ==1)
  mLow = 0.5; mHigh = 2.5; sig = 1;
elseif (nargin == 2)
  mHigh = 2.5; sig = 1;
elseif (nargin == 3)
  sig = 1;
end


%sig = 1;
%mLow = 0.5; 
%mHigh = 2.5;

%img  = imread('macbeth.jpg');
%img  = imread('elephant.jpg');
%img  =  imread('hecface.jpg');

origImage = img;

if (ndims(img)==3)
  img =double(rgb2gray(img));
end

%smoothen ??
%G = gauss(sig);
%img = conv2(img, G'*G,'same');

%CONVOLUTION WITH DERIVATIVE OF GAUSSIAN

dG=dgauss(sig);

[dummy, filterLen] = size(dG);
offset = (filterLen-1)/2;

sy = conv2(img, dG ,'same');
sx = conv2(img, dG','same');

[m, n]=size(img);

% crop off the boundary parts...the places where the convolution was partial
sx = sx(offset+1:m-offset, offset+1:n-offset); 
sy = sy(offset+1:m-offset, offset+1:n-offset); 

% norm of gradient
sNorm = sqrt( sx.^2 + sy.^2 );

% direction of gradient
sAngle = atan2( sy, sx) * (180.0/pi);

% handle divide by zero....
sx(sx==0) = 1e-10;
sSlope = abs(sy ./ sx);

sAorig = sAngle;

%for us, x and x-pi are the same....
y = sAngle < 0;
sAngle = sAngle + 180*y;

% bin the angles into 4 principal directions
% 0-45 45-90 90-135 135-180

binDist =    [-inf 45 90 135 inf];

[dummy, b] = histc(sAngle,binDist);

sDiscreteAngles = b;
[m,n] = size(sDiscreteAngles);

% each pixel is set to either 1,2,3 or 4
% set the boundary pixels to 0, so we don't count them in analysis...
sDiscreteAngles(1,:) = 0;
sDiscreteAngles(end,:)=0;
sDiscreteAngles(:,1) = 0;
sDiscreteAngles(:,end) = 0;

sEdgepoints = zeros(m,n);

sFinal = sEdgepoints;

lowT  = mLow * mean(sNorm(:));
highT = mHigh * lowT;

thresh = [ lowT highT];

% NON-MAXIMAL SUPPRESSION

% for each kind of direction, interpolate.... and see if the current point is
% is the local maximum in that direction

%in the following comments, assume that we are currently at (0,0) and
%we have to interpolate from the 8 surrounding pixels, also, we are
%using MATLAB's single index feature...

%gradient direction: 0-45 i.e. gradDir =1
gradDir = 1;
indxs = find(sDiscreteAngles == gradDir);
slp = sSlope(indxs);

% interpolate between (1,1) and (1,0)
% gDiff1 = Gy/Gx*(magtd(0,0) - magtd(1,1)) + (1- Gy/Gx)*(magtd(0,0)-magtd(1,0))
gDiff1 = slp.*(sNorm(indxs)-sNorm(indxs+m+1)) + (1-slp).*(sNorm(indxs)-sNorm(indxs+1));

% interpolate between (-1,-1) and (-1,0)
% gDiff2 = Gy/Gx*(magtd(0,0) - magtd(-1,-1)) +  (1- Gy/Gx)*(magtd(0,0)-magtd(-1,0))
gDiff2 = slp.*(sNorm(indxs)-sNorm(indxs-m-1)) + (1-slp).*(sNorm(indxs)-sNorm(indxs-1));

okIndxs = indxs( gDiff1 >=0 & gDiff2 >= 0);
sEdgepoints(okIndxs) = 1;


%gradient direction: 45-90 i.e. gradDir =2
gradDir = 2;
indxs = find(sDiscreteAngles == gradDir);
invSlp = 1 ./ sSlope(indxs);

% interpolate between (1,1) and (0,1)
% gDiff1 = (Gx/Gy)*(magtd(0,0) - magtd(1,1)) + (1- Gx/Gy)*(magtd(0,0)-magtd(0,1))
gDiff1 =   invSlp.*(sNorm(indxs)-sNorm(indxs+m+1)) + (1-invSlp).*(sNorm(indxs)-sNorm(indxs+m));

% interpolate between (-1,-1) and (0,-1)
% gDiff2 = (Gx/Gy)*(magtd(0,0) - magtd(-1,-1)) + (1- Gx/Gy)*(magtd(0,0)-magtd(0,-1))
gDiff2 =   invSlp.*(sNorm(indxs)-sNorm(indxs-m-1)) + (1-invSlp).*(sNorm(indxs)-sNorm(indxs-m));

okIndxs = indxs( gDiff1 >=0 & gDiff2 >= 0);
sEdgepoints(okIndxs) = 1;



%gradient direction: 90-135 i.e. gradDir =3
gradDir = 3;
indxs = find(sDiscreteAngles == gradDir);
invSlp = 1 ./ sSlope(indxs);

% interpolate between (-1,1) and (0,1)
% gDiff1 = (Gx/Gy)*(magtd(0,0) - magtd(-1,1)) + (1- Gx/Gy)*(magtd(0,0)-magtd(0,1))
gDiff1 =   invSlp.*(sNorm(indxs)-sNorm(indxs+m-1)) + (1-invSlp).*(sNorm(indxs)-sNorm(indxs+m));

% interpolate between (1,-1) and (0,-1)
% gDiff2 = (Gx/Gy)*(magtd(0,0) - magtd(1,-1)) + (1- Gx/Gy)*(magtd(0,0)-magtd(0,-1))
gDiff2 =   invSlp.*(sNorm(indxs)-sNorm(indxs-m+1)) + (1-invSlp).*(sNorm(indxs)-sNorm(indxs-m));

okIndxs = indxs( gDiff1 >=0 & gDiff2 >= 0);
sEdgepoints(okIndxs) = 1;



%gradient direction: 135-180 i.e. gradDir =4
gradDir = 4;
indxs = find(sDiscreteAngles == gradDir);
slp = sSlope(indxs);

% interpolate between (-1,1) and (-1,0)
% gDiff1 = Gy/Gx*(magtd(0,0) - magtd(-1,1)) + (1- Gy/Gx)*(magtd(0,0)-magtd(-1,0))
gDiff1 = slp.*(sNorm(indxs)-sNorm(indxs+m-1)) + (1-slp).*(sNorm(indxs)-sNorm(indxs-1));

% interpolate between (1,-1) and (1,0)
% gDiff2 = Gy/Gx*(magtd(0,0)-magtd(1,-1)) +  (1- Gy/Gx)*(magtd(0,0)-magtd(1,0))
gDiff2 = slp.*(sNorm(indxs)-sNorm(indxs-m+1)) + (1-slp).*(sNorm(indxs)-sNorm(indxs+1));

okIndxs = indxs( gDiff1 >=0 & gDiff2 >= 0);
sEdgepoints(okIndxs) = 1;


%HYSTERESIS PART...

sEdgepoints = sEdgepoints*0.6;
x = find(sEdgepoints > 0 & sNorm < lowT);
sEdgepoints(x)=0;
x = find(sEdgepoints > 0 & sNorm  >= highT);
sEdgepoints(x)=1;

%sFinal(sEdgepoints>0)=1;

%at this point, if 
%    sNorm(pixel) > lowT then sEdgepoints(pixel)=0
%    highT > sNorm(pixel) > lowT then sEdgepoints(pixel)=0.6
%    sNorm(pixel) > highT then sEdgepoints(pixel)=1

% the idea is this: along the neighbouring pixels in that direction
% add 0.4...so all points that were 0.6 will become 1.0
% see if the number of 1's has changed...
% keep doing while number of 1's doesn't change

oldx = [];
x = find(sEdgepoints==1);
while (size(oldx,1) ~= size(x,1))
  oldx = x;
  v = [x+m+1, x+m, x+m-1, x-1, x-m-1, x-m, x-m+1, x+1];
  sEdgepoints(v) = 0.4 + sEdgepoints(v);
  y = find(sEdgepoints==0.4);
  sEdgepoints(y) = 0;
  y = find(sEdgepoints>=1);
  sEdgepoints(y)=1;
  x = find(sEdgepoints==1);
end
		   
x = find(sEdgepoints==1);

sFinal(x)=1;

figure(1);
imagesc(sFinal); colormap(gray); axis image;

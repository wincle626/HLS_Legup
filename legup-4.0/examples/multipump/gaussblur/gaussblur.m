img = imread('small.pgm');
% create a gaussian low pass filter
gauss = fspecial('gaussian', [3 3], 1)
blur = imfilter(img, gauss, 'same');
figure
subplot(2,1,1)
imshow(img)
subplot(2,1,2)
imshow(blur)

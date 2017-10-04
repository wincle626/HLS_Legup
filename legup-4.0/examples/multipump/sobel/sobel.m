% from http://angeljohnsy.blogspot.ca/2011/12/sobel-edge-detection.html

function sobel(image, Thresh)

if (nargin < 2) Thresh = 100; end

A=imread(image);
B=rgb2gray(A);

C=double(B);


for i=1:size(C,1)-2
    for j=1:size(C,2)-2
        %Sobel mask for x-direction:
        Gx=((2*C(i+2,j+1)+C(i+2,j)+C(i+2,j+2))-(2*C(i,j+1)+C(i,j)+C(i,j+2)));
        %Sobel mask for y-direction:
        Gy=((2*C(i+1,j+2)+C(i,j+2)+C(i+2,j+2))-(2*C(i+1,j)+C(i,j)+C(i+2,j)));
       
        %The gradient of the image
        %keyboard;
        B(i,j)=sqrt(Gx.^2+Gy.^2);
        C(i,j)=abs(Gx)+abs(Gy);
       
    end
end
%keyboard
%figure,imshow(A); title('Original');
%figure,imshow(B); title('Sobel gradient');

B=max(B,Thresh);
B(B==round(Thresh))=0;

B=uint8(B);
%keyboard
figure,imshow(B~=0);title('Edge detected Image');

C=max(C,Thresh);
C(C==round(Thresh))=0;

C=uint8(C);
figure,imshow(C~=0);title('Approx Edge detected Image');

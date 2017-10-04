% from http://blog.weisu.org/2007/10/matlab-codeuse-dct-for-image_29.html

dvalue=imread('group.jpg');
dvalue = double(dvalue)/255;
dvalue = rgb2gray(dvalue);
img_dct=dct2(dvalue);
img_pow=(img_dct).^2;
img_pow=img_pow(:);
[B,index]=sort(img_pow);%no zig-zag
B=flipud(B);
index=flipud(index);
compressed_dct=zeros(size(dvalue));
coeff = 20000;% maybe change the value
for k=1:coeff
compressed_dct(index(k))=img_dct(index(k));
end
im=idct2(compressed_dct);

imwrite(im, 'maptemp2.bmp')

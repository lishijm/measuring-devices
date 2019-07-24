RGB=imread('C:\Users\Administrator\Desktop\1.jpg');%读取图像
I1=rgb2gray(RGB);%转化为灰度图像
K=medfilt2(I1,[3 3]);%增强图像，进行3*3中值滤波
L=histeq(K);%灰度图像直方图均衡化
BW=edge(L,'prewitt');%用prewitt算子进行边缘检测
CW=bwmorph(BW,'dilate');
imshow(BW);
figure,imshow(CW);
[H,T,R]=hough(CW,'RhoResolution',0.75,'ThetaResolution',0.75);
figure,imshow(imadjust(mat2gray(H)),'XData',T,'YData',R,'InitialMagnification','fit');
title('霍夫空间');
xlabel('\theta'),ylabel('\rho');
axis on,axis normal,hold on;
colormap(hot);
P =houghpeaks(H,4);%显示峰值累加器的位置
x=T(P(:,2));
y=R(P(:,1));
plot(x,y,'s','color','white');%将峰值点在霍夫空间标出
lines=houghlines(CW,T,R,P,'FillGap',5,'MinLength',10);
%根据峰值累加器的位置确定直线段，最短距离为5，小于5的视为同一条直线；直线段的
%最短距离为10，否则将不会被检测到
imshow(RGB),hold on;
b=floor([lines.rho]);%向下取整
point1=cat(1,lines.point1);
point2=cat(1,lines.point2);
k=1;
while k<length(b)
if abs(lines(k).theta)<92&abs(lines(k).theta)>88%限定范围
xy(1,:)=point1(k,:);
xy(2,:)=point2(k,:);
for a=k+1:length(b)
if b(k)==b(a),
xy(2,:)=point2(a,:);
k=a;
end
End%将属于同一直线的直线段重组
if abs(xy(1,1)-xy(2,1))>100%重组后的直线长度应大于100
plot(xy(:,1),xy(:,2),'LineWidth',2,'Color','green');%画出重组后的直线
plot(xy(1,1),xy(1,2),'x','LineWidth',2,'Color','yellow');%标出端点
plot(xy(2,1),xy(2,2),'x','LineWidth',2,'Color','red');
temp(i)=b(k);&保存RHO的值
i=i+1;
end
end
k=k+1;
end
hold off;
y1=max(abs(b));
y2=min(abs(b));
h=y1-y2%计算障碍物的成像高度
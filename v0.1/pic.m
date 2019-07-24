RGB=imread('C:\Users\Administrator\Desktop\5.jpg');%��ȡͼ��
I1=rgb2gray(RGB);%ת��Ϊ�Ҷ�ͼ��
K=medfilt2(I1,[3 3]);%��ǿͼ�񣬽���3*3��ֵ�˲�
L=histeq(K);%�Ҷ�ͼ��ֱ��ͼ���⻯
BW=edge(L,'prewitt');%��prewitt���ӽ��б�Ե���
CW=bwmorph(BW,'dilate');
imshow(BW);
figure,imshow(CW);
[H,T,R]=hough(CW,'RhoResolution',0.75,'ThetaResolution',0.75);
figure,imshow(imadjust(mat2gray(H)),'XData',T,'YData',R,'InitialMagnification','fit');
title('����ռ�');
xlabel('\theta'),ylabel('\rho');
axis on,axis normal,hold on;
colormap(hot);
P =houghpeaks(H,4);%��ʾ��ֵ�ۼ�����λ��
x=T(P(:,2));
y=R(P(:,1));
plot(x,y,'s','color','white');%����ֵ���ڻ���ռ���
lines=houghlines(CW,T,R,P,'FillGap',5,'MinLength',10);
%���ݷ�ֵ�ۼ�����λ��ȷ��ֱ�߶Σ���̾���Ϊ5��С��5����Ϊͬһ��ֱ�ߣ�ֱ�߶ε�
%��̾���Ϊ10�����򽫲��ᱻ��⵽
imshow(RGB),hold on;
b=floor([lines.rho]);%����ȡ��
point1=cat(1,lines.point1);
point2=cat(1,lines.point2);
k=1;
while k<length(b)
if abs(lines(k).theta)<92&abs(lines(k).theta)>88%�޶���Χ
xy(1,:)=point1(k,:);
xy(2,:)=point2(k,:);
for a=k+1:length(b)
if b(k)==b(a),
xy(2,:)=point2(a,:);
k=a;
end
End%������ͬһֱ�ߵ�ֱ�߶�����
if abs(xy(1,1)-xy(2,1))>100%������ֱ�߳���Ӧ����100
plot(xy(:,1),xy(:,2),'LineWidth',2,'Color','green');%����������ֱ��
plot(xy(1,1),xy(1,2),'x','LineWidth',2,'Color','yellow');%����˵�
plot(xy(2,1),xy(2,2),'x','LineWidth',2,'Color','red');
temp(i)=b(k);&����RHO��ֵ
i=i+1;
end
end
k=k+1;
end
hold off;
y1=max(abs(b));
y2=min(abs(b));
h=y1-y2%�����ϰ���ĳ���߶�
clear all
close all
clc

% Import raw data from Xsens cpp acquisition code 
T1=readtable('Group1_task1.0.csv');
T2=readtable('Group1_task4.0.csv');

% Task 1
var1=9; %data "Axel_Z_AC" (hand sensors) used for labeling
% we delete packet counter data
T1.packetCounter_C9 = [];
T1.packetCounter_AC = [];
T1.packetCounter_B5 = [];
T1.packetCounter_B6 = [];
T1.globalPacketCounter = [];
T1 = table2array(T1);
T1 = [T1 , ones(1,length(T1))'];
for i=1:length(T1(:,var1))
    if(9.2<T1(i,var1) && T1(i,var1)<9.5)
    T1(i,25)=0; %we label T1 data as "rest" when thresholding condition is satisfied
    end
end

% TASK 2
var2=21; %Axel_Y_B6 (forearm)
T2.packetCounter_C9 = [];
T2.packetCounter_AC = [];
T2.packetCounter_B5 = [];
T2.packetCounter_B6 = [];
T2.globalPacketCounter = [];
T2 = table2array(T2);
T2 = [T2 , 2*ones(1,length(T2))'];
for i=1:length(T2(:,var2))
    if(7.4<T2(i,var2) && T2(i,var2)<8.8)
        T2(i,25)=0; %rest
    end
end

% Concatenate task in a matrix and save it for further processing 
A = T1(1:round(length(T1)*0.8), :); % we select 80% of data for training
B = T2(1:round(length(T2)*0.8), :);
Data_train = [A; B];

C = T1(round(length(T1)*0.8:end), :); % and 20% for testing
D = T2(round(length(T2)*0.8:end), :);
Data_test = [C; D];

csvwrite('Data_train.csv',Data_train); %we save the obtained labelled data in csv files
csvwrite('Data_test.csv',Data_test);
%% Plots comparing chosen variable, monitored variable and label
close all
figure;plot(A(:,6),"Color","r");title('task 1');hold on %Monitored variable to identify movement
plot(A(:,var1)-6,"Color","#0072BD");hold on % Variable used for thresholding
plot(A(:,25),"LineWidth",1.5,"Color","g"); %Label
legend('Gyro Z bicep','Axel Z hand','Label')

figure;plot(B(:,6),"Color","r"); title ('task 2');hold on %Monitored variable to identify movement
plot(B(:,var2)-7,"Color","#0072BD");hold on % Variable used for thresholding
plot(B(:,25),"LineWidth",1.5,"Color","g"); %Label
legend('Gyro Z bicep','Axel Y forearm','Label')

% Import raw data from Xsens cpp acquisition code 

% For each task aquire a different file and read each file separatelty
T1=readtable('File1.csv');
T2=readtable('File2.csv');
T3=readtable('File3.csv');

% In data preparation each task should be assigned with a number 
%%
% For Each IMU code Acceleration and Gyroscope signals along x, y, z are
% stored in columns 
% Each IMU is identifies with a 2 digit code 

%%%%%%%%%%%%%%%
%%%%%Task1%%%%%
%%%%%%%%%%%%%%%

% For each task remove the packetCounter columns (i.e. keep only columns with data)
T1.packetCounter_92 = [];
T1.packetCounter_9C = [];
T1.packetCounter_95 = [];
T1.packetCounter_9F = [];
T1.globalPacketCounter = [];

T1 = table2array(T1);
% Add one column with class = 1 
T1 = [T1 , ones(1,length(T1))'];

% Identify pauses (where Acceleration is lower than a certain threshold) and assign class 0 to them 
lower_threshold = 2.2;
upper_threshold = 2.6; 

for i=1:length(T1(:,13))
    if(lower_threshold<T1(i,13) && T1(i,13)<upper_threshold)
        T1(i,25)=0;
    end
end

% Repeat for all the tasks acquired

%%%%%%%%%%%%%%%
%%%%%Task2%%%%%
%%%%%%%%%%%%%%%

T2.packetCounter_92 = [];
T2.packetCounter_9C = [];
T2.packetCounter_95 = [];
T2.packetCounter_9F = [];
T2.globalPacketCounter = [];

T2 = table2array(T2);
T2 = [T2 , 2*ones(1,length(T2))'];

for i=1:length(T2(:,13))
    if(2.2<T2(i,13) && T2(i,13)<2.6)
        T2(i,25)=0;
    end
end

%%%%%%%%%%%%%%%
%%%%%Task3%%%%%
%%%%%%%%%%%%%%%

T3.packetCounter_92 = [];
T3.packetCounter_9C = [];
T3.packetCounter_95 = [];
T3.packetCounter_9F = [];
T3.globalPacketCounter = [];

T3 = table2array(T3);
T3 = [T3 , 3*ones(1,length(T3))'];

for i=1:length(T3(:,13))
    if(2.2<T3(i,13) && T3(i,13)<2.6)
        T3(i,25)=0;
    end
end

% Concatenate task in a matrix and save it for further processing 
Data = [T1; T2; T3];
Data_train = [T1(1:round(length(T1)*0.8)); T2(1:round(length(T2)*0.8)); T3(1:round(length(T3)*0.8))];
Data_test = [T1(1:round(length(T1)*0.2)); T2(1:round(length(T2)*0.2)); T3(1:round(length(T3)*0.2))];
save ('Data_train.mat', Data_train);
save ('Data_test.mat', Data_test);

save('Data.mat','Data') ;


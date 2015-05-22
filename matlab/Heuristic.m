
% Sheet1 contains detailed information of 10 vehicles, 
% Sheet2 gives the speed profile you need (marked yellow).

clear all;
close all;
clc;

vehID = xlsread('10_veh_Trajectory.xlsx', 2, 'A:A');
time = xlsread('10_veh_Trajectory.xlsx', 2, 'E:E');
speed = xlsread('10_veh_Trajectory.xlsx', 2, 'G:G');
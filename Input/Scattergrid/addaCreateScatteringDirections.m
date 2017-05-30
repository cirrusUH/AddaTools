% This generates the different scattering directions 
% in form of a scat_params.dat, as given on page 47 (B.4) in the Adda
% Manual. 
% 
% use with in Adda: -scat_grid_inp <filename>
%                    adda  -scat_grid_inp 1deg_1deg.dat -store_scat_grid -scat_matr both
%
% G Ritter, London 2015

function ret = addaCreateScatteringDirections(filename) 

%filename = "1deg_1deg.dat";

%dPhi = 0.2;
%dThetaLowRes = 0.2;
%dThetaHiRes = 0.1; %degree
%phis=0:dPhi:360;
%thetas=0:dTheta:180;

% Highres_angle_low = 5;
% Highres_angle_high = 26;
% thetas=[ 0:dThetaLowRes:(Highres_angle_low-1) Highres_angle_low:dThetaHiRes:(Highres_angle_high-dThetaHiRes)  Highres_angle_high:dThetaLowRes:180 ];
% thetas = [ 0:0.1:5-0.1 5:0.2:26-0.2 26:1:175-1 175:0.5:179-0.5 179:0.1:180];

% 1 deg grid
phis  =  [ 0:1:360 ];
thetas = [ 0:1:180 ];

fid = fopen(filename, "w");

% print header
fprintf(fid, "global_type=pairs\n");

N = numel(phis)*numel(thetas);
fprintf(fid, "N=%d\n", N);

fprintf(fid, "pairs=\n");

for phi=phis
for theta=thetas
  fprintf(fid,"%0.2f %0.2f\n", theta, phi);
end
end

fclose(fid)


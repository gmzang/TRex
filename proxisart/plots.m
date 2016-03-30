%% Actual Shepp Logan phantom
SL = [  2   .69   .92    0     0     0   
        -.98 .6624 .8740   0  -.0184   0
        -.02 .1100 .3100  .22    0    -18
        -.02 .1600 .4100 -.22    0     18
         .01 .2100 .2500   0    .35    0
         .01 .0460 .0460   0    .1     0
         .01 .0460 .0460   0   -.1     0
         .01 .0460 .0230 -.08  -.605   0 
         .01 .0230 .0230   0   -.606   0
         .01 .0230 .0460  .06  -.605   0   ];
P = phantom(SL, 512);
save phantoms/sl-512.mat P
% imwrite(P, 'sl-256.png', 'BitDepth',16);

%% Modified Shepp-Logan
P = phantom(512);
save phantoms/mod-sl-512.mat P

%% Ncat
% How it was generated
% 	imgBig = read_ncat('nx', 1024, 'ny', 1024, 'marrow', true, ...
% 		'mu', [0 0.05 0.2 0.4 0.4 0.2]/0.2*1000); % True highres NCAT
% 	imgBig = single(imgBig) / single(max(imgBig(:))) * 0.4; % convert to 1/cm units

load phantoms/ncat-1024.mat
figure(1), imshow(P, [])

scale = 256;
P = imresize(P, scale / 1024, 'lanczos3');
figure(2), imshow(P, []), colorbar
save(sprintf('phantoms/ncat-%d.mat', scale), 'P')

%% Per iteration SNR for different # of projections
%

algs = {
  % ASTRA types
  %
  struct('name','SART', 'type','astra', ...
    'clr','k', 'lstyle','-', 'marker','d', ...
    'alg','SART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))
      
  struct('name','SART-2', 'type','astra', ...
    'clr','k', 'lstyle','--', 'marker','d', ...
    'alg','SART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1.99, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

  struct('name','SART-0.1', 'type','astra', ...
    'clr','k', 'lstyle','-.', 'marker','d', ...
    'alg','SART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',.1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

%   struct('name','SART-0.5', 'type','astra', 'clr','-rd', ...
%     'alg','SART', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',0.5, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
% 
%   struct('name','SART-0.1', 'type','astra', 'clr','-gd', ...
%     'alg','SART', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',0.1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
      
%   struct('name','SART-0.9', 'type','astra', 'clr','-gd', ...
%     'alg','SART', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',0.9, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
%       
%   struct('name','SART-1.9', 'type','astra', 'clr','-bd', ...
%     'alg','SART', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',1.9, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
      
%   struct('name','SART-PROX', 'type','astra', 'clr','-.k<', ...
%     'alg','SART-PROX', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',1, 'Lambda',1e1, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
      
  struct('name','BSSART', 'type','astra', ...
    'clr',[1 0.5 0], 'lstyle','-', 'marker','d', ...
    'alg','SART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',1)))

  struct('name','BSSART-2', 'type','astra', ...
    'clr',[1 0.5 0], 'lstyle','--', 'marker','d', ...
    'alg','SART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1.99, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',1)))
      
  struct('name','ART', 'type','astra', ...
    'clr','r', 'lstyle','-', 'marker','s', ...
    'alg','ART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

	struct('name','ART-2', 'type','astra', ...
    'clr','r', 'lstyle','--', 'marker','s', ...
    'alg','ART', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1.99, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

  struct('name','SIRT', 'type','astra', ...
    'clr','b', 'lstyle','-', 'marker','x', ...
    'alg','SIRT', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

  struct('name','SIRT-2', 'type','astra', ...
    'clr','b', 'lstyle','--', 'marker','x', ...
    'alg','SIRT', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1.99, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

	struct('name','BICAV', 'type','astra', ...
    'clr','g', 'lstyle','-', 'marker','*', ...
    'alg','BICAV', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

	struct('name','BICAV-2', 'type','astra', ...
    'clr','g', 'lstyle','--', 'marker','*', ...
    'alg','BICAV', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1.99, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))
      
  struct('name','OS-SQS', 'type','astra', ...
    'clr','m', 'lstyle','-', 'marker','^', ...
    'alg','OS-SQS', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))

  struct('name','OS-SQS-0.1', 'type','astra', ...
    'clr','m', 'lstyle','-.', 'marker','^', ...
    'alg','OS-SQS', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',.1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))
      
%   struct('name','OS-SQS-0.5', 'type','astra', 'clr','-r^', ...
%     'alg','OS-SQS', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',0.5, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
% 
%   struct('name','OS-SQS-0.1', 'type','astra', 'clr','-g^', ...
%     'alg','OS-SQS', ...
%     'alg_params', struct('iter',1, ...
%       'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
%       'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
%       'Alpha',.1, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
%       'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
%       'UseBSSART',0)))
      
  struct('name','CGLS', 'type','astra', ...
    'clr',[0.5 0.5 0.5], 'lstyle','-', 'marker','v', ...
    'alg','CGLS', ...
    'alg_params', struct('iter',1, ...
      'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',2, 'Lambda',1e3, 'ComputeIterationMetrics',1, ...
      'ClearReconstruction',1, 'UseJacobiPreconditioner',0, ...
      'UseBSSART',0)))      
  };

phantoms = {'mouse'}; {'ncat', 'mod-sl'};
num_projs = [15, 30, 60, 90, 180]; 
noise_levels = 0; %[0, .005, .01, .05, .1]; %.001
proj_types = {'mouse'}; %{'fan', 'parallel'};
iter = 30;
prefix = '';
plt = 'per_iter';

for phantom = phantoms
  for num_proj = num_projs
    for noise_level = noise_levels
      for proj_type = proj_types
        % put args
        arg = struct('phan',phantom, 'phan_size',512, 'path','./plots', ...
          'noise_type','gauss', 'noise_level',noise_level, ...
          'num_proj',num_proj, 'iter',iter, 'recompute',1, ...
          'proj_type',proj_type, 'prefix',prefix);
        % call
        ma_run_and_plot(plt, arg, algs);
      end
    end
  end
end


%%

addpath I:/temp/astra-1.7.1beta-src/matlab/tools/
addpath I:/temp/astra-1.7.1beta-src/bin/x64/Release/

arg = struct('phan','mod-sl', 'phan_size',512, 'path','./plots', ...
  'noise_type','gauss', 'noise_level',.001, ...
  'num_proj',15, 'iter',30, 'recompute',1, ...
  'proj_type','fan');
% call
ma_fig_periter(arg, algs(1:2));

%%
arg = struct('phan','mouse', 'phan_size',512, 'path','./plots', ...
  'noise_type','gauss', 'noise_level',.001, ...
  'num_proj',15, 'iter',30, 'recompute',1, ...
  'proj_type','mouse');
% call
ma_fig_periter(arg, algs(1));

%% Extracting the data

pat = 'C:/Program Files (x86)/Exxim_Mouse_Example/raw.%04d';

sino = zeros(195, 512);
row = 128;
for i=0:194
  % read
  f = fopen(sprintf(pat, i), 'rb');
  im = fread(f, 256*512, 'int16');
  % reshape: it's row major
  im = reshape(im, 512,256)';
  % take the middle row
  sino(i+1, :) = im(row,:);
  fclose(f);
end
%
% read offset
offset_file = 'C:/Program Files (x86)/Exxim_Mouse_Example/offset';
offset = fread(fopen(offset_file, 'rb'), 256*512, 'int16');
offset = reshape(offset, 512,256)';
offset = offset(row, :);

% read air raw
airraw_file = 'C:/Program Files (x86)/Exxim_Mouse_Example/airraw';
airraw = fread(fopen(airraw_file, 'rb'), 256*512, 'int16');
airraw = reshape(airraw, 512,256)';
airraw = airraw(row, :);

% Correction
%   S (u,v) = log ( S_air(u,v) - S_offset(u,v) ) - log (S_prj_in(u,v) - S_offset(u,v) )
save phantoms/mouse-raw.mat sino airraw offset

%% Computing the GT volume

phan_size = 512;
det_size = 4/3; 0.16176;
num_det = 512;
sdd = 529.59 * (4/3) / .16176;
sid = sdd * (phan_size/2) / (num_det/2 * det_size); %395.730011;
proj_geom = astra_create_proj_geom('fanflat', det_size, num_det, ...
  (0:194)*pi/180 + pi/2, sid, sdd - sid); %2*pi
vol_geom = astra_create_vol_geom(phan_size, phan_size);

% projector
proj_id = astra_create_projector('line_fanflat', proj_geom, vol_geom);
  
pp = load('phantoms/mouse-raw.mat');
% get rid of negative values
% correction
air = bsxfun(@minus, pp.airraw, pp.offset);
% air(air < 0) = min(air(air > 0));

sino = bsxfun(@minus, double(pp.sino), pp.offset);
% sino(sino < 0) = 1;

sino = bsxfun(@minus, log(air), log(sino));

% copy last column
sino(:, end) = sino(:, end-1);
% sino(rows, cols) = repmat(log(air(cols)), length(rows), 1);
% sino(isinf(sino)) = min(sino(:));
% sino = log(60e3) - log(sino);
%   sino = sino - min(sino(:));
% shift left by 4 (detector offset)
ss = sino;
sino(:,1:end-4) = sino(:,5:end);
sino(:,end-3:end) = ss(:,1:4);
% sino(:,end-3:end) = min(sino(:));
%   sino = fliplr(sino);
sino(sino < 0) = 0;
% multiply by 10 to make in cm^-1 
sino = sino * 10;
  
in_params = struct('vol_geom',vol_geom, 'proj_geom',proj_geom, ...
  'gt_vol',[], 'sino',sino, 'proj_id',proj_id, ...
  'wi',ones(size(sino)), 'fbp',[], 'prox_in',[]);

alg = 'SART';
alg_params = struct('iter',500, ...
    'option',struct('UseMinConstraint',1, 'MinConstraintValue',0, ...
      'UseMaxConstraint',0, 'MaxConstraintValue',1, ...
      'Alpha',.1, 'Lambda',1e3, 'ComputeIterationMetrics',0, ...
      'ClearReconstruction',1, 'UseBSSART',1));

P = ma_alg_astra(alg, in_params, alg_params);
astra_mex_projector('delete', proj_id);

phan_file = 'phantoms/mouse-sart-512.mat';
save(phan_file, 'P', 'sino');
%%  
figure(3), imshow(P, []), colorbar
figure(2), hist(P(:), (0:.01:.3))


function [] = export_calibration_info( device_folder )
%EXPORT_CALIBRATION_INFO Exports information from calibration files
%   This function expects that user has captured 2 calibration sequences
%    -> Color + IR
%    -> Color + Depth
%
%   Color + IR should be used to calibrate cameras using Calibrator App, 
%   and saved in device folder as 'color_session.mat' and 
%   'infrared_session.mat' respectively.
%
%   Color + Depth should be saved in depth_distortion folder in device 
%   folder.

% TODO: Check for folder existence!

% depth_distortion folder path
depth_distortion_folder = fullfile(device_folder, 'depth_distortion');

% get the prepared sessions. These contain required calibrated parameters.
color_info    = load(fullfile(device_folder,'color_session.mat'));
color_session = color_info.calibrationSession;
ir_info       = load(fullfile(device_folder,'infrared_session.mat'));
ir_session    = ir_info.calibrationSession;


% extract parameters to our format into the depth_distortion folder
parameters_filename = fullfile(depth_distortion_folder, 'parameters.txt');
fprintf('Exporting color and ir camera parameters to \"%s\" -> ',...
                                                 parameters_filename); tic;
export_color_and_ir_params(color_session, ir_session, parameters_filename);
fprintf('Done in %f\n', toc);


% estimate plane poses for depth distortion session
fprintf('Estimating plane poses for depth distortion -> '); tic;
[plane_poses, images_info] = estimate_depth_distortion_plane_poses(...
                             color_session, ...
                             fullfile( depth_distortion_folder, 'color'));
fprintf('Done in %f\n', toc);

% save plane poses
pose_filename = fullfile(depth_distortion_folder, 'plane_poses.txt');
fprintf('Saving estimated plane poses to \"%s\" -> ', pose_filename); tic;
pose_fid = fopen(pose_filename, 'w');
n_valid = 0;
for pose_idx = 1:size(plane_poses,3)
    T = plane_poses(:,:,pose_idx);
    if ~any(T)
      continue;
    end;
    
    fprintf(pose_fid, '%12.8f %12.8f %12.8f %12.8f  ', T(:,1));
    fprintf(pose_fid, '%12.8f %12.8f %12.8f %12.8f  ', T(:,2));
    fprintf(pose_fid, '%12.8f %12.8f %12.8f %12.8f  ', T(:,3));
    fprintf(pose_fid, '%12.8f %12.8f %12.8f %12.8f\n', T(:,4));
    n_valid = n_valid+1;
end;
fclose(pose_fid);
fprintf('Done in %f\n', toc);

% save configuration file
conf_filename = fullfile(depth_distortion_folder,...
                                      'depth_distortion_calibration.conf');
fprintf('Saving configuration file to \"%s\" -> ', conf_filename); tic;
conf_fid = fopen( conf_filename, 'w');
fprintf(conf_fid, 'n_images %d\n\n', n_valid);
fprintf(conf_fid, 'parameters parameters.txt\n');
fprintf(conf_fid, 'plane_poses plane_poses.txt\n');

for pose_idx = 1:size(plane_poses,3)
    T = plane_poses(:,:,pose_idx);
    if ~any(T)
      continue;
    end;
    image_info = images_info(pose_idx);
    image_name = fullfile( image_info.folder, image_info.name );
    [~, name, ext] = fileparts(image_name);
    color_image_name = strcat(name, ext);
    depth_image_name = strcat(name, '.png');
    fprintf(conf_fid,...
                'scan %30s %30s  1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 1\n',...
                depth_image_name,...
                color_image_name);
end;

fclose(conf_fid);
fprintf('Done in %f\n', toc);

end


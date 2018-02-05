function [] = export_color_and_ir_params( color_session,...
                                          ir_session,...
                                          parameter_filename )
%EXPORT_INFO Writes color depth/extrinsics and color/depth intrinsics.

%extract info from calibration session file
color_f = color_session.CameraParameters.FocalLength;
color_c = color_session.CameraParameters.PrincipalPoint;
color_rd = color_session.CameraParameters.RadialDistortion;
color_td = color_session.CameraParameters.TangentialDistortion;

ir_f = ir_session.CameraParameters.FocalLength;
ir_c = ir_session.CameraParameters.PrincipalPoint;
ir_rd = ir_session.CameraParameters.RadialDistortion;
ir_td = ir_session.CameraParameters.TangentialDistortion;

% get the pariwise transformation
depthToColor = estimate_pairwise_transform( color_session,...
                                            ir_session);

% get image size
color_im = imread( color_session.BoardSet.FullPathNames{1} );
depth_im = imread( ir_session.BoardSet.FullPathNames{1} );

% output scannet format parameter file
params_fid = fopen( parameter_filename, 'w');
fprintf(params_fid, 'colorWidth = %d\n', size(color_im, 2) );
fprintf(params_fid, 'colorHeight = %d\n', size(color_im, 1) );
fprintf(params_fid, 'depthWidth = %d\n', size(depth_im,2) );
fprintf(params_fid, 'depthHeight = %d\n', size(depth_im,1) );
fprintf(params_fid, 'fx_color = %f\n', color_f(1) );
fprintf(params_fid, 'fy_color = %f\n', color_f(2) );
fprintf(params_fid, 'mx_color = %f\n', color_c(1) );
fprintf(params_fid, 'my_color = %f\n', color_c(2) );
fprintf(params_fid, 'fx_depth = %f\n', ir_f(1) );
fprintf(params_fid, 'fy_depth = %f\n', ir_f(2) );
fprintf(params_fid, 'mx_depth = %f\n', ir_c(1) );
fprintf(params_fid, 'my_depth = %f\n', ir_c(2) );
fprintf(params_fid, 'k1_color = %f\n', color_rd(1) );
fprintf(params_fid, 'k2_color = %f\n', color_rd(2) );
fprintf(params_fid, 'k3_color = %f\n', color_td(1) );
fprintf(params_fid, 'k4_color = %f\n', color_td(2) );
fprintf(params_fid, 'k5_color = %f\n', 0.0 );
fprintf(params_fid, 'k1_depth = %f\n', ir_rd(1) );
fprintf(params_fid, 'k2_depth = %f\n', ir_rd(2) );
fprintf(params_fid, 'k3_depth = %f\n', ir_td(1) );
fprintf(params_fid, 'k4_depth = %f\n', ir_td(2) );
fprintf(params_fid, 'k5_depth = %f\n', 0.0 );
fprintf(params_fid, 'depthToColorExtrinsics = %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n', depthToColor(1,:), depthToColor(2,:), depthToColor(3,:), depthToColor(4,:) );
fclose(params_fid);

% same information, for compatibility with .conf file.
%TODO: Remove!
% di_fid = fopen(strcat(dst_dir, 'DEPTH_INTRINSICS'), 'w');
% fprintf( di_fid, '%12.8f %12.8f %12.8f\n', ir_f(1), 0.0, ir_c(1) );
% fprintf( di_fid, '%12.8f %12.8f %12.8f\n', 0.0, ir_f(2), ir_c(2) );
% fprintf( di_fid, '%12.8f %12.8f %12.8f\n', 0.0, 0.0, 1.0 );
% fclose(di_fid);
% 
% ci_fid = fopen(strcat(dst_dir, 'COLOR_INTRINSICS'), 'w');
% fprintf( ci_fid, '%12.8f %12.8f %12.8f\n', color_f(1), 0.0, color_c(1) );
% fprintf( ci_fid, '%12.8f %12.8f %12.8f\n', 0.0, color_f(2), color_c(2) );
% fprintf( ci_fid, '%12.8f %12.8f %12.8f\n', 0.0, 0.0, 1.0 );
% fclose( ci_fid );
% 
% de_fid = fopen(strcat(dst_dir, 'DEPTH_EXTRINSICS'), 'w');
% fprintf( de_fid, '%12.8f %12.8f %12.8f %12.8f\n', depthToColor(1,:) );
% fprintf( de_fid, '%12.8f %12.8f %12.8f %12.8f\n', depthToColor(2,:) );
% fprintf( de_fid, '%12.8f %12.8f %12.8f %12.8f\n', depthToColor(3,:) );
% fprintf( de_fid, '%12.8f %12.8f %12.8f %12.8f\n', depthToColor(4,:) );
% fclose( de_fid );
% 
% ce_fid = fopen(strcat(dst_dir, 'COLOR_EXTRINSICS'), 'w');
% fprintf( ce_fid, '%12.8f %12.8f %12.8f %12.8f\n', 1.0, 0.0, 0.0, 0.0 );
% fprintf( ce_fid, '%12.8f %12.8f %12.8f %12.8f\n', 0.0, 1.0, 0.0, 0.0 );
% fprintf( ce_fid, '%12.8f %12.8f %12.8f %12.8f\n', 0.0, 0.0, 1.0, 0.0 );
% fprintf( ce_fid, '%12.8f %12.8f %12.8f %12.8f\n', 0.0, 0.0, 0.0, 1.0 );
% fclose( ce_fid );
end


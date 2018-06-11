function [plane_poses, depth_distortion_images_info] = estimate_depth_distortion_plane_poses( color_session, depth_distortion_folder )

% Get depth distortion session color images
depth_distortion_images_info = dir(fullfile(depth_distortion_folder,'*.jpg'));
n_images = length(depth_distortion_images_info);

% Get camera parameters && board set
camera_params = color_session.CameraParameters;
board_set = color_session.BoardSet;

% Get the ideal world points ( assumes same calibration grid has been used!)
board_size   = board_set.BoardSize;
square_size  = board_set.SquareSize; % in millimeters
world_points = generateCheckerboardPoints(board_size, square_size);

% Prealocate storage for plane poses
plane_poses = zeros(4, 4, n_images);

% Matrix to convert matlab extrinsics to our format 
conversion_matrix = [1 0 0 0 ; 0 -1 0 0 ; 0 0 -1 0 ; 0 0 0 1];

% Extract extrinsics
progress = 0.025;
fprintf('Progress: [');
for image_idx=1:n_images;
  if image_idx / n_images > progress
    progress = progress + 0.025;
    fprintf('.');
  end
  image_info = depth_distortion_images_info(image_idx);
  image_name = fullfile( image_info.folder, image_info.name );
  image = imread(image_name);
  undistorted_image = undistortImage(image, camera_params);
  [image_points, board_size] = detectCheckerboardPoints(undistorted_image);
  if numel(image_points) == numel(world_points)
    [rotation, translation] = extrinsics(image_points, world_points, camera_params);
    translation = 0.001 * translation;
    pose = [[rotation ; translation], [0 0 0 1]'];
    plane_poses(:,:,image_idx) = conversion_matrix * pose * conversion_matrix;
  end
end
fprintf('] ', toc );


end

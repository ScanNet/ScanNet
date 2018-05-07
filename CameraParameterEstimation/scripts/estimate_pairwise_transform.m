function transformation = estimate_pairwise_transform( color_session, ir_session )

    color_camera_params = color_session.CameraParameters;
    ir_camera_params    = ir_session.CameraParameters;
    
    % extract data
    col_world_points = color_camera_params.WorldPoints;
    col_rotations    = color_camera_params.RotationMatrices;
    col_translations = color_camera_params.TranslationVectors;

    ir_world_points  = ir_camera_params.WorldPoints;
    ir_rotations     = ir_camera_params.RotationMatrices;
    ir_translations  = ir_camera_params.TranslationVectors;

    % useful info
    n_images = size( col_rotations,3 );
    n_points_per_image = size( col_world_points, 1 );


    % points assume z = 0, let's add this dimension
    col_world_points = [ col_world_points, zeros( size(col_world_points, 1), 1 ) ];
    ir_world_points  = [ ir_world_points, zeros(size(ir_world_points, 1), 1) ];

    % lets get the points represented with camera centered at origin

    % first color ...
    col_camera_points = zeros( n_images * n_points_per_image, 3 );
    for j = 1:size( col_rotations,3 )
        for i = 1:size(col_world_points, 1 )
            col_camera_points( (j-1) * n_points_per_image + i, :) = col_world_points(i,:) *  col_rotations(:,:,j) + col_translations(j,:);
        end
    end

    % ... and ir
    ir_camera_points = zeros( n_images * n_points_per_image, 3 );
    for j = 1:size( ir_rotations,3 )
        for i = 1:size(ir_world_points, 1 )
            ir_camera_points( (j-1) * n_points_per_image + i, :) = ir_world_points(i,:) * ir_rotations(:,:,j) + ir_translations(j,:);
        end
    end

    % convert these points to our coordinate system
    conversion = [1 0 0; 0 -1 0 ; 0 0 -1];
    for i = 1:size(ir_camera_points,1)
        ir_point = ir_camera_points(i, :);
        col_point = col_camera_points(i, :);
        ir_camera_points(i, :) = ir_point * conversion;
        col_camera_points(i, :) = col_point * conversion;
    end
    
    % now we can estimate the transformation between the two
    [transformation, eps] = estimate_rigid_transform(col_camera_points',...
                                                       ir_camera_points' );
    transformation(1:3,4) = 0.001 * transformation(1:3,4);
        
    % adjust the transformation
    conversion = [ 1 0 0 0 ; 0 -1 0 0 ; 0 0 -1 0 ; 0 0 0 1 ];
    transformation = conversion * transformation * conversion;
    
end
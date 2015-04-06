program TestSolver2D
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, i, j
	integer:: allocstat

	integer:: i_flow
	real:: h_slope
	character(10)::outfile

	integer:: discharge_size
	double precision,dimension(:),allocatable:: discharge_time, discharge_value 

	integer:: isize, jsize
	double precision,dimension(:,:),allocatable:: grid_x, grid_y, elevation
	integer,dimension(:,:),allocatable:: obstacle, vegetation

	double precision::time, dtime, etime, convergence
	integer::iteration
	double precision,dimension(:,:),allocatable:: velocity_x, velocity_y, depth
	integer,dimension(:,:),allocatable:: wetflag

	! *****************************************************************************
	! ****** �O�����P�iCGNS�t�@�C������̌v�Z�����A�i�q�A�����l���̓ǂݍ��݁j *****
	! *****************************************************************************

	! CGNS �t�@�C���̃I�[�v��
	call cg_open_f('test.cgn', CG_MODE_MODIFY, fin, ier)
	if (ier /=0) STOP "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	print *, ier
!	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

	! ��͏������p�����[�^�̓ǂݍ���
	call cg_iric_read_integer_f('i_flow', i_flow, ier)
	call cg_iric_read_real_f('h_slope', h_slope, ier)
	call cg_iric_read_string_f('outfile', outfile, ier)
	! �i�o�́j
	print *, 'i_flow:  ', i_flow
	print *, 'h_slope: ', h_slope
	print *, 'outfile: ', outfile

	! discharge �f�[�^�̓ǂݍ���
	call cg_iric_read_functionalsize_f('discharge', discharge_size, ier)
	allocate (discharge_time(discharge_size), discharge_value(discharge_size), stat = allocstat)
	if (allocstat /=0) STOP "*** Not enough memory ***"
	call cg_iric_read_functional_f('discharge', discharge_time, discharge_value, ier)
	! �i�o�́j
	if (ier ==0) then
		print *, 'discharge: discharge_size=', discharge_size
		do i = 1, min(discharge_size, 5)
			print *, ' i,time,value:', i, discharge_time(i), discharge_value(i)
		end do
	end if

	! �Q�����\���i�q�f�[�^�̓ǂݍ���
	call cg_iric_gotogridcoord2d_f(isize, jsize, ier)
	allocate (grid_x(isize,jsize), grid_y(isize,jsize), stat = allocstat)
	if (allocstat /=0) STOP "*** Not enough memory ***"
	call cg_iric_getgridcoord2d_f(grid_x, grid_y, ier)
	if (ier /=0) STOP "*** No grid data ***"
	! �i�o�́j
	print *, 'grid x,y: isize, jsize=', isize, jsize
	do i = 1, min(isize,5)
		do j = 1, min(jsize,5)
			print *, ' (',i,',',j,')=(',grid_x(i,j),',',grid_y(i,j),')'
		end do
	end do

	! �i�q�̑����f�[�^�̓ǂݍ��݁i�m�[�h�^�C�v�j
	allocate (elevation(isize,jsize), stat = allocstat)
	if (allocstat /=0) STOP "*** Not enough memory ***"
	call cg_iric_read_grid_real_node_f('Elevation', elevation, ier)
	! �i�o�́j
	if (ier ==0) then
		print *, 'Elevation: isize, jsize=', isize, jsize
		do i = 1, min(isize,5)
			do j = 1, min(jsize,5)
				print *, ' (',i,',',j,')=(',elevation(i,j),')'
			end do
		end do
	else
		print *, 'Elevation: Not found. ier=', ier
	end if

	! �i�q�̑����f�[�^�̓ǂݍ��݁i�Z���^�C�v�j
	allocate (obstacle(isize-1,jsize-1), vegetation(isize-1,jsize-1), stat = allocstat)
	if (allocstat /=0) STOP "*** Not enough memory ***"
	call cg_iric_read_grid_integer_cell_f('Obstacle', obstacle, ier)
	! �i�o�́j
	if (ier ==0) then
		print *, 'Obstacle: isize-1, jsize-1=', isize-1, jsize-1
		do i = 1, min(isize-1,5)
			do j = 1, min(jsize-1,5)
				print *, ' (',i,',',j,')=(',obstacle(i,j),')'
			end do
		end do
	else
		print *, 'Obstacle: Not found. ier=', ier
	end if

	call cg_iric_read_grid_integer_cell_f('Vegetation', vegetation, ier)
	! �i�o�́j
	if (ier ==0) then
		print *, 'Vegetation: isize-1, jsize-1=', isize-1, jsize-1
		do i = 1, isize-1
			do j = 1, jsize-1
				print *, 'Vegetation:(',i,',',j,')=(',vegetation(i,j),')'
			end do
		end do
	else
		print *, 'Vegetation: Not found. ier=', ier
	end if

	! *****************************************************
	! ****** �O�����Q�i�v�Z�p�G���A�̊m�ہA���������j *****
	! ******************************************************

	! �v�Z�p�G���A�̊m��
	allocate(velocity_x(isize,jsize), velocity_y(isize,jsize), depth(isize, jsize), wetflag(isize,jsize))
	if (allocstat /=0) STOP "*** Not enough memory ***"

	! ������
	time = 0.0
	dtime = 10.0
	etime = 100.0
	iteration = 0
	velocity_x = 1.0
	velocity_y = 2.0
	depth = 3.0
	wetflag = -1

	! ***********************************************************************
	! ****** ��͌v�Z���[�v�iCGNS�t�@�C���ւ̌v�Z���ʂ̏����o�����܂ށj *****
	! ***********************************************************************

	do
		time = time +dtime
		iteration = iteration+1
		convergence =0.0
		print *, 'time,iteration=',time,iteration

		! ***** �����Ōv�Z�����{ *****

		! ��͎����i�܂��̓��[�v�񐔂̂����ꂩ�j�̏o��
		call cg_iric_write_sol_time_f(time, ier)
		! �i�q�̏o�́i�ړ��i�q�̏ꍇ�j
		call cg_iric_write_sol_gridcoord2d_f(grid_x, grid_y, ier)
		! �v�Z���ʂ̏o�́i�����ł͗����A���[�A���t���O�A�����l�A�����񐔂̏����o����Ꭶ�j
		call cg_iric_write_sol_real_f('VelocityX', velocity_x, ier)
		call cg_iric_write_sol_real_f('VelocityY', velocity_y, ier)
		call cg_iric_write_sol_real_f('Depth', depth, ier)
		call cg_iric_write_sol_integer_f('Wet', wetflag, ier)
		call cg_iric_write_sol_baseiterative_real_f('Convergence', convergence, ier)
		call cg_iric_write_sol_baseiterative_integer_f('Iteration', iteration, ier)

		if (time>=etime) exit
	end do

	! *********************************
	! ****** ��͌v�Z���[�v�̏I�� *****
	! *********************************

	! allocate�����������̊J��
	deallocate(discharge_time, discharge_value)
	deallocate(grid_x, grid_y)
	deallocate(elevation)
	deallocate(obstacle, vegetation)
	deallocate(velocity_x, velocity_y, depth, wetflag)

	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program TestSolver2D
